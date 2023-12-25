#include "gallery.h"

void anim_fold_start(lv_anim_t *a)
{
    printf("%s\n", __func__);
    common_anim_start();
    lv_gl_obj_ready(obj_fold, 1);
}

void anim_fold(void *var, int32_t v)
{
    int pos;
    int angle;

    lv_slider_set_value(slider, v, LV_ANIM_ON);

    pos = v / 180;
    angle = v % 180;
    if (angle == 0) angle = 1;

    obj_fold->steps = 8;
    obj_fold->pos = pos * 2 + 1;
    lv_gl_obj_set_angle(obj_fold, 0, angle, 0);
    lv_obj_invalidate(lv_layer_top());
}

void anim_fold_end(lv_anim_t *a)
{
    animing = 0;
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 0, LV_ANIM_ON);
}

