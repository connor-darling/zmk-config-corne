#include <lvgl.h>
#include <zmk/display.h>

lv_obj_t *zmk_display_status_screen_custom(lv_disp_t *disp) {
    lv_obj_t *screen = lv_disp_get_scr_act(disp);

    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "HELLO");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    return screen;
}
