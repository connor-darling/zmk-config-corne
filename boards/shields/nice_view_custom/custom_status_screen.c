#include <zephyr/kernel.h>
#include <lvgl.h>
#include <zmk/display.h>

#include "widgets/battery_status.h"
#include "widgets/art.h"

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include "widgets/ble_status.h"
#endif

static struct zmk_widget_battery_status battery_status_widget;
static struct zmk_widget_art art_widget;

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static struct zmk_widget_ble_status ble_status_widget;
#endif

lv_obj_t *zmk_display_status_screen() {
    lv_disp_t *disp = lv_disp_get_default();
    disp->driver->sw_rotate = 1;
    lv_disp_set_rotation(disp, LV_DISP_ROT_90);

    lv_obj_t *screen = lv_obj_create(NULL);

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    zmk_widget_ble_status_init(&ble_status_widget, screen);
    lv_obj_align(zmk_widget_ble_status_obj(&ble_status_widget),
                 LV_ALIGN_TOP_LEFT, 0, 0);
#endif

    zmk_widget_battery_status_init(&battery_status_widget, screen);
    lv_obj_align(zmk_widget_battery_status_obj(&battery_status_widget),
                 LV_ALIGN_TOP_RIGHT, 0, 0);

    zmk_widget_art_init(&art_widget, screen);
    lv_obj_align(zmk_widget_art_obj(&art_widget),
                 LV_ALIGN_CENTER, 0, 5);

    return screen;
}
