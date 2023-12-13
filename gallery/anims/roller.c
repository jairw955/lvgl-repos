#include "gallery.h"

void anim_roller_start(lv_anim_t *a);
void anim_roller(void *var, int32_t v);
void anim_roller_end(lv_anim_t *a);

static lv_anim_t sub_anims[] =
{
    {
        .time = 10000,
        .start_value = 200,
        .current_value = 200,
        .end_value = -200,
        .repeat_cnt = 1,
        .path_cb = lv_anim_path_linear,
        .exec_cb = anim_roller,
        .deleted_cb = anim_roller_end,
        .var = (void *)1,
    },
    {
        .time = 1000,
        .start_value = -200,
        .current_value = -200,
        .end_value = -400,
        .repeat_cnt = 1,
        .path_cb = lv_anim_path_linear,
        .exec_cb = anim_roller,
        .deleted_cb = anim_roller_end,
        .var = (void *)2,
    }
};

void anim_roller_start(lv_anim_t *a)
{
    SDL_Rect c;

    printf("%s\n", __func__);
    c.w = lv_obj_get_width(scr);
    c.h = lv_obj_get_height(scr) * 12;
    c.x = 0;
    c.y = -(c.h - lv_obj_get_height(scr)) / 2;
    lv_gl_set_viewport(NULL, &c);

    obj_roller->offset_z = 0.1;
    obj_roller->base.r.y = (crop.h - obj_roller->base.r.h) / 2;

    lv_obj_add_flag(img1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img2, LV_OBJ_FLAG_HIDDEN);
    lv_gl_obj_idle(obj_cube);
    lv_gl_obj_idle(obj_fold);
    lv_gl_obj_ready(obj_roller, 1);
}

void anim_roller(void *var, int32_t v)
{
    int index = (intptr_t)var;
    lv_slider_set_value(slider, v, LV_ANIM_ON);

    if (index == 0)
        obj_roller->alpha = (float)(400 - v) / 200 * 1.0;
    if (index == 1)
        obj_roller->alpha = 1.0;
    if (index == 2)
        obj_roller->alpha = (float)(v + 400) / 200 * 1.0;
    lv_gl_obj_set_angle(obj_roller, v / 10.0, 0, 0);
    lv_obj_invalidate(lv_layer_top());
}

void anim_roller_end(lv_anim_t *a)
{
    int index = (intptr_t)a->var;
    if (index == 0)
        lv_anim_start(&sub_anims[0]);
    if (index == 1)
        lv_anim_start(&sub_anims[1]);
    if (index == 2)
    {
        animing = 0;
        lv_slider_set_range(slider, 0, 100);
        lv_slider_set_value(slider, 0, LV_ANIM_ON);
    }
}

