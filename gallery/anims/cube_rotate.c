#include "gallery.h"

void anim_cube_rotate_start(lv_anim_t *a)
{
    printf("%s\n", __func__);
    common_anim_start();
    lv_gl_obj_set_view_angle(obj_cube, 0, 0, 0);
    lv_gl_obj_set_angle(obj_cube, 0, 0, 0);
    lv_gl_obj_ready(obj_cube, 1);
    lv_gl_set_viewport(NULL, NULL);
}

void anim_cube_rotate(void *var, int32_t v)
{
    lv_slider_set_value(slider, v, LV_ANIM_ON);

    obj_cube->base.r.w = obj_cube->base.w * 0.57;
    obj_cube->scale = (float)obj_cube->base.r.w / crop.w;
    lv_gl_obj_set_angle(obj_cube, v, v, 0);
    lv_obj_invalidate(lv_layer_top());
}

void anim_cube_rotate_end(lv_anim_t *a)
{
    animing = 0;
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 0, LV_ANIM_ON);
}

