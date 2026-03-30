#include <lvgl.h>
#include <zmk/display.h>

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen = lv_obj_create(NULL);

    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "HELLO");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    return screen;
}
