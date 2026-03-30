#pragma once

#include <lvgl.h>

struct zmk_widget_art {
    lv_obj_t *obj;
};

int zmk_widget_art_init(struct zmk_widget_art *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_art_obj(struct zmk_widget_art *widget);
