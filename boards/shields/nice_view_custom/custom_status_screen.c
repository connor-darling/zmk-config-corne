#include <zephyr/kernel.h>
#include <lvgl.h>
#include <zmk/display.h>

#include "widgets/status.h"

static struct zmk_widget_status status_widget;

extern const lv_img_dsc_t art_image;

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen = lv_obj_create(NULL);

    /* Art image — pre-rotated, positioned at left in landscape = bottom in portrait */
    lv_obj_t *art = lv_img_create(screen);
    lv_img_set_src(art, &art_image);
    lv_obj_align(art, LV_ALIGN_TOP_LEFT, 0, 0);

    /* Status widget — canvas at right side = portrait top */
    zmk_widget_status_init(&status_widget, screen);

    return screen;
}
