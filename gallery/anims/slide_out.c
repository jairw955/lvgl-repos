#include "gallery.h"

void anim_slide_out_start(lv_anim_t *a)
{
    printf("%s\n", __func__);
    lv_obj_clear_flag(img1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(img2, LV_OBJ_FLAG_HIDDEN);
    lv_gl_obj_idle(obj_cube);
    lv_gl_obj_idle(obj_fold);
    lv_gl_obj_idle(obj_roller);
}

void anim_slide_out(void *var, int32_t v)
{
    lv_obj_set_x(img2, v - 480);
    lv_obj_set_x(img1, v);
    lv_obj_set_style_img_opa(img2, LV_OPA_COVER, 0);
    lv_obj_set_style_img_opa(img1, LV_OPA_COVER, 0);
    lv_slider_set_value(slider, 480 - v, LV_ANIM_ON);
}

void anim_slide_out_end(lv_anim_t *a)
{
    lv_obj_t *t;

    animing = 0;
    t = img1;
    img1 = img2;
    img2 = t;

    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 0, LV_ANIM_ON);
}

