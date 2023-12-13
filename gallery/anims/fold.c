#include "gallery.h"

void anim_fold_start(lv_anim_t *a)
{
    printf("%s\n", __func__);
    lv_obj_add_flag(img1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img2, LV_OBJ_FLAG_HIDDEN);
    lv_gl_obj_idle(obj_cube);
    lv_gl_obj_ready(obj_fold, 1);
    lv_gl_obj_idle(obj_roller);
    lv_gl_set_viewport(NULL, NULL);
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

