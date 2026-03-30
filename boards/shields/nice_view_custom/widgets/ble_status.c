#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/ble.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/event_manager.h>

#include "ble_status.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct ble_status_state {
    bool connected;
    uint8_t profile_index;
};

static void set_ble_status(struct zmk_widget_ble_status *widget,
                           struct ble_status_state state) {
    char text[12];
    if (state.connected) {
        snprintf(text, sizeof(text), LV_SYMBOL_BLUETOOTH " %d", state.profile_index + 1);
    } else {
        snprintf(text, sizeof(text), LV_SYMBOL_BLUETOOTH " X");
    }
    lv_label_set_text(widget->obj, text);
}

static void ble_status_update_cb(struct ble_status_state state) {
    struct zmk_widget_ble_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        set_ble_status(widget, state);
    }
}

static struct ble_status_state ble_status_get_state(const zmk_event_t *eh) {
    return (struct ble_status_state){
        .connected = zmk_ble_active_profile_is_connected(),
        .profile_index = zmk_ble_active_profile_index(),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_ble_status, struct ble_status_state,
                            ble_status_update_cb, ble_status_get_state)

ZMK_SUBSCRIPTION(widget_ble_status, zmk_ble_active_profile_changed);

int zmk_widget_ble_status_init(struct zmk_widget_ble_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);
    lv_label_set_text(widget->obj, "");

    sys_slist_append(&widgets, &widget->node);
    widget_ble_status_init();

    return 0;
}

lv_obj_t *zmk_widget_ble_status_obj(struct zmk_widget_ble_status *widget) {
    return widget->obj;
}
