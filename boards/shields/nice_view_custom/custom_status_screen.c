#include <zephyr/kernel.h>
#include <lvgl.h>
#include <zmk/display.h>

#include "widgets/battery_status.h"
#include "widgets/ble_status.h"
#include "widgets/art.h"

static struct zmk_widget_battery_status battery_status_widget;
static struct zmk_widget_ble_status ble_status_widget;
static struct zmk_widget_art art_widget;

lv_obj_t *zmk_display_status_screen_custom(lv_disp_t *disp) {
    lv_obj_t *screen = lv_disp_get_scr_act(disp);

    zmk_widget_ble_status_init(&ble_status_widget, screen);
    lv_obj_align(zmk_widget_ble_status_obj(&ble_status_widget),
                 LV_ALIGN_TOP_LEFT, 0, 0);

    zmk_widget_battery_status_init(&battery_status_widget, screen);
    lv_obj_align(zmk_widget_battery_status_obj(&battery_status_widget),
                 LV_ALIGN_TOP_RIGHT, 0, 0);

    zmk_widget_art_init(&art_widget, screen);
    lv_obj_align(zmk_widget_art_obj(&art_widget),
                 LV_ALIGN_CENTER, 0, 5);

    return screen;
}
