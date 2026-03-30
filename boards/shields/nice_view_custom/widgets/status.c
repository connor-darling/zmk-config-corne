#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/event_manager.h>

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/ble.h>
#include <zmk/events/ble_active_profile_changed.h>
#endif

#include "status.h"
#include "util.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct status_state {
    uint8_t battery;
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    bool connected;
    uint8_t profile_index;
#endif
};

static void draw_status(struct zmk_widget_status *widget, struct status_state state) {
    lv_obj_t *canvas = lv_obj_get_child(widget->obj, 0);

    lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_COVER);

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = lv_color_black();
    label_dsc.font = &lv_font_montserrat_16;

    /* Battery — draw at top of canvas (becomes right side after rotation,
       which is the right side of the portrait status bar) */
    char bat_text[8];
    snprintf(bat_text, sizeof(bat_text), "%d%%", state.battery);
    lv_canvas_draw_text(canvas, 2, 2, 64, &label_dsc, bat_text);

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    /* BLE status — draw below battery */
    char ble_text[16];
    if (state.connected) {
        snprintf(ble_text, sizeof(ble_text), "BT%d", state.profile_index + 1);
    } else {
        snprintf(ble_text, sizeof(ble_text), "BT X");
    }
    lv_canvas_draw_text(canvas, 2, 22, 64, &label_dsc, ble_text);
#endif

    /* Separator line */
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.color = lv_color_black();
    line_dsc.width = 1;
    lv_point_t line_points[] = {{0, 64}, {68, 64}};
    lv_canvas_draw_line(canvas, line_points, 2, &line_dsc);

    rotate_canvas(canvas, widget->cbuf);
}

static void update_status(struct status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        draw_status(widget, state);
    }
}

static struct status_state get_state(const zmk_event_t *eh) {
    return (struct status_state){
        .battery = 0,
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
        .connected = zmk_ble_active_profile_is_connected(),
        .profile_index = zmk_ble_active_profile_index(),
#endif
    };
}

static struct status_state battery_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);
    struct status_state state = get_state(eh);
    state.battery = (ev != NULL) ? ev->state_of_charge : 0;
    return state;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_status_battery, struct status_state,
                            update_status, battery_get_state)
ZMK_SUBSCRIPTION(widget_status_battery, zmk_battery_state_changed);

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static struct status_state ble_get_state(const zmk_event_t *eh) {
    struct status_state state = get_state(eh);
    return state;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_status_ble, struct status_state,
                            update_status, ble_get_state)
ZMK_SUBSCRIPTION(widget_status_ble, zmk_ble_active_profile_changed);
#endif

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 160, 68);
    lv_obj_align(widget->obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);

    /* Status canvas — positioned at right edge (= portrait top) */
    lv_obj_t *canvas = lv_canvas_create(widget->obj);
    lv_canvas_set_buffer(canvas, widget->cbuf, CANVAS_SIZE, CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(canvas, LV_ALIGN_TOP_RIGHT, 0, 0);

    sys_slist_append(&widgets, &widget->node);
    widget_status_battery_init();
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    widget_status_ble_init();
#endif

    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) {
    return widget->obj;
}
