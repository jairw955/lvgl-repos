#include "gallery.h"

void anim_cube_lyric_start(lv_anim_t *a);
void anim_cube_lyric(void *var, int32_t v);
void anim_cube_lyric_end(lv_anim_t *a);

static lv_anim_t sub_anims[] =
{
    {
        .time = 300,
        .start_value = 0,
        .current_value = 0,
        .end_value = 90,
        .repeat_cnt = 1,
        .start_cb = anim_cube_lyric_start,
        .path_cb = lv_anim_path_linear,
        .exec_cb = anim_cube_lyric,
        .deleted_cb = anim_cube_lyric_end,
        .var = (void *)1,
    },
    {
        .time = 3000,
        .start_value = 0,
        .current_value = 0,
        .end_value = 14,
        .repeat_cnt = 1,
        .start_cb = anim_cube_lyric_start,
        .path_cb = lv_anim_path_linear,
        .exec_cb = anim_cube_lyric,
        .deleted_cb = anim_cube_lyric_end,
        .var = (void *)2,
    },
};

void anim_cube_lyric_start(lv_anim_t *a)
{
    printf("%s\n", __func__);
    lv_obj_add_flag(img1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img2, LV_OBJ_FLAG_HIDDEN);
    obj_cube->base.r.w = obj_cube->base.w * 0.57;
    obj_cube->scale = (float)obj_cube->base.r.w / crop.w;
    lv_gl_obj_set_view_angle(obj_cube, -10, 15, 0);
    lv_gl_obj_ready(obj_cube, 1);
    lv_gl_obj_idle(obj_fold);
    lv_gl_obj_idle(obj_roller);
    lv_gl_set_viewport(NULL, NULL);
}

void anim_cube_lyric(void *var, int32_t v)
{
    static int32_t last_v = -1;
    int index = (intptr_t)var;
    int pos = 0;
    lv_slider_set_value(slider, v, LV_ANIM_ON);

    if (last_v == v)
        return;
    last_v = v;

    if (index == 0)
    {
        for (int i = 0; i < lines / 2; i++)
        {
            if (obj_lyrics[i].len < (v - pos))
            {
                pos += obj_lyrics[i].len;
                continue;
            }
            lv_gl_obj_ready(obj_lyrics[i].objs[v - pos - 1], 1);
            break;
        }
    }
    if (index == 1)
    {
        lv_gl_obj_set_angle(obj_cube, 0, -v, 0);
    }
    if (index == 2)
    {
        for (int i = lines / 2; i < lines; i++)
        {
            if (obj_lyrics[i].len < (v - pos))
            {
                pos += obj_lyrics[i].len;
                continue;
            }
            lv_gl_obj_ready(obj_lyrics[i].objs[v - pos - 1], 1);
            break;
        }
    }

    lv_obj_invalidate(lv_layer_top());
}

void anim_cube_lyric_end(lv_anim_t *a)
{
    int index = (intptr_t)a->var;
    lv_obj_t *t;

    if (index == 0)
    {
        lv_anim_set_delay(&sub_anims[0], 1000);
        lv_anim_start(&sub_anims[0]);
    }
    if (index == 1)
    {
        lv_anim_start(&sub_anims[1]);
    }
    if (index == 2)
    {
        animing = 0;
        lv_slider_set_range(slider, 0, 100);
        lv_slider_set_value(slider, 0, LV_ANIM_ON);
    }
}


