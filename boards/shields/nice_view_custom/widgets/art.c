#include <lvgl.h>
#include "art.h"

extern const lv_img_dsc_t art_image;

int zmk_widget_art_init(struct zmk_widget_art *widget, lv_obj_t *parent) {
    widget->obj = lv_img_create(parent);
    lv_img_set_src(widget->obj, &art_image);
    return 0;
}

lv_obj_t *zmk_widget_art_obj(struct zmk_widget_art *widget) {
    return widget->obj;
}
