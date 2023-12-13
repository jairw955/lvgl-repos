#include "gallery.h"

void anim_cube_flip_start(lv_anim_t *a);
void anim_cube_flip(void *var, int32_t v);
void anim_cube_flip_end(lv_anim_t *a);

static lv_anim_t sub_anims[] =
{
    {
        .time = 300,
        .act_time = -1000,
        .start_value = 90,
        .current_value = 90,
        .end_value = 180,
        .repeat_cnt = 1,
        .path_cb = lv_anim_path_ease_in_out,
        .exec_cb = anim_cube_flip,
        .deleted_cb = anim_cube_flip_end,
        .var = (void *)90,
    },
    {
        .time = 300,
        .act_time = -1000,
        .start_value = 180,
        .current_value = 180,
        .end_value = 270,
        .repeat_cnt = 1,
        .path_cb = lv_anim_path_ease_in_out,
        .exec_cb = anim_cube_flip,
        .deleted_cb = anim_cube_flip_end,
        .var = (void *)180,
    },
    {
        .time = 300,
        .act_time = -1000,
        .start_value = 270,
        .current_value = 270,
        .end_value = 360,
        .repeat_cnt = 1,
        .path_cb = lv_anim_path_ease_in_out,
        .exec_cb = anim_cube_flip,
        .deleted_cb = anim_cube_flip_end,
        .var = (void *)270,
    }
};

#define ZOOM_IN_SIZE        40
#define ZOOM_IN     do { \
    if (pos < ZOOM_IN_SIZE) \
    { \
        obj_cube->base.r.w = crop.w - pos; \
    } \
    else if (pos > (90 - ZOOM_IN_SIZE)) \
    { \
        obj_cube->base.r.w = crop.w - (90 - pos); \
    } \
    else \
    { \
        obj_cube->base.r.w = crop.w - ZOOM_IN_SIZE; \
    } \
    obj_cube->scale = (float)obj_cube->base.r.w / view.w; \
} while (0);

void anim_cube_flip_start(lv_anim_t *a)
{
    printf("%s\n", __func__);
    lv_obj_add_flag(img1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img2, LV_OBJ_FLAG_HIDDEN);
    lv_gl_obj_set_view_angle(obj_cube, 0, 0, 0);
    lv_gl_obj_set_angle(obj_cube, 0, 0, 0);
    lv_gl_obj_ready(obj_cube, 1);
    lv_gl_obj_idle(obj_fold);
    lv_gl_obj_idle(obj_roller);
    lv_gl_set_viewport(NULL, NULL);
}

void anim_cube_flip(void *var, int32_t v)
{
    int pos;
    lv_slider_set_value(slider, v, LV_ANIM_ON);

    pos = v - (intptr_t)var;
    ZOOM_IN;
    lv_gl_obj_set_angle(obj_cube, 0, v, 0);
    lv_obj_invalidate(lv_layer_top());
}

void anim_cube_flip_end(lv_anim_t *a)
{
    int var = (intptr_t)a->var;

    if (var == 0)
        lv_anim_start(&sub_anims[0]);
    if (var == 90)
        lv_anim_start(&sub_anims[1]);
    if (var == 180)
        lv_anim_start(&sub_anims[2]);
    if (var == 270)
    {
        animing = 0;
        lv_slider_set_range(slider, 0, 100);
        lv_slider_set_value(slider, 0, LV_ANIM_ON);
    }
}

