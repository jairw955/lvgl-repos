#include <lvgl/lvgl.h>
#include <lvgl/lv_drivers/sdl/gl/gl.h>

#include "gallery.h"

#include "anims/cube_flip.h"
#include "anims/cube_lyric.h"
#include "anims/cube_rotate.h"
#include "anims/fade_out.h"
#include "anims/fade_slide_out.h"
#include "anims/fold.h"
#include "anims/roller.h"
#include "anims/photo_stream.h"
#include "anims/slide_out.h"

static char *lyric[] =
{
    "上次",
    "预订的花",
    "摆了很久",
    "已经枯萎",
    "也忘了",
    "去打卡",
    "地图上",
    "标记的经纬"
};
int lines = sizeof(lyric) / sizeof(lyric[0]);
int animing = 0;
lv_ft_info_t ttf_main;

lv_gl_obj_t *obj_cube;
lv_gl_obj_t *obj_fold;
lv_gl_obj_t *obj_roller;
lyric_row *obj_lyrics;

lv_obj_t *scr;
lv_obj_t *img1;
lv_obj_t *img2;
lv_obj_t *btn_mat;
lv_obj_t *slider;
SDL_Rect crop;
SDL_Rect view;

lv_obj_t *photo_box;
lv_obj_t *photos[4];

static lv_anim_t anims[] =
{
    ANIM_FADE_OUT,
    ANIM_SLIDE_OUT,
    ANIM_FADE_SLIDE_OUT,
    ANIM_CUBE_ROTATE,
    ANIM_CUBE_FLIP,
    ANIM_FOLD,
    ANIM_ROLLER,
    ANIM_CUBE_LYRIC,
    ANIM_PHOTO_STREAM,
};
static int anim_cnt = sizeof(anims) / sizeof(anims[0]);

static const char *btnm_map[] = {
    "1", "2", "3", "4", "5", "6", "\n",
    "7", "8", "9", "10", "11", "12", ""
};

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        if (id >= anim_cnt)
            return;
        if (!animing)
        {
            lv_anim_start(&anims[id]);
            if (anims[id].start_value > anims[id].end_value)
            {
                lv_slider_set_range(slider,
                    anims[id].end_value,
                    anims[id].start_value);
            }
            else
            {
                lv_slider_set_range(slider,
                    anims[id].start_value,
                    anims[id].end_value);
            }
            lv_slider_set_value(slider, 0, LV_ANIM_OFF);
            animing = 1;
        }
    }
}

static void font_init(void)
{
    printf("%s\n", __func__);
    lv_freetype_init(64, 1, 0);

    ttf_main.weight = 60;
    ttf_main.name = "/data/SmileySans-Oblique.ttf";
    ttf_main.style = FT_FONT_STYLE_NORMAL;
    lv_ft_font_init(&ttf_main);
}

static label_canvas *create_canvas(lv_color_t color,
    lv_font_t *font)
{
    label_canvas *lc;

    lc = lv_mem_alloc(sizeof(label_canvas));
    if (!lc)
        return NULL;
    lv_memset_00(lc, sizeof(label_canvas));

    lv_draw_label_dsc_init(&lc->label_dsc);
    lc->label_dsc.color = color;
    lc->label_dsc.font = font;
    lc->canvas = lv_canvas_create(NULL);

    return lc;
}

static lv_gl_obj_t *utf8_to_obj(lv_gl_obj_t *parent,
    label_canvas *lc, char *text)
{
    lv_img_dsc_t *img_dsc = lc->img_dsc;
    lv_gl_obj_t *obj;
    lv_gl_img_t img;
    lv_coord_t data_size;
    lv_point_t size;

    lv_txt_get_size(&size, text, lc->label_dsc.font,
        0, 0, LV_COORD_MAX, 0);

    data_size = lv_img_buf_get_img_size(ALIGN(size.x, 16),
            ALIGN(size.y, 16), LV_IMG_CF_TRUE_COLOR_ALPHA);
    if (!img_dsc ||
        data_size > img_dsc->data_size)
    {
        if (img_dsc)
            lv_img_buf_free(img_dsc);
        img_dsc = lv_img_buf_alloc(ALIGN(size.x, 16),
            ALIGN(size.y, 16), LV_IMG_CF_TRUE_COLOR_ALPHA);
        lc->img_dsc = img_dsc;
        printf("new buf %p\n", img_dsc);
    }
    else
    {
        lv_memset_00((uint8_t *)img_dsc->data,
            img_dsc->data_size);
    }
    lv_canvas_set_buffer(lc->canvas, (void *)img_dsc->data,
        ALIGN(size.x, 16), ALIGN(size.y, 16),
        img_dsc->header.cf);
    lv_canvas_draw_text(lc->canvas,
        ((ALIGN(size.x, 16) - size.x) / 2),
        ((ALIGN(size.y, 16) - size.y) / 2),
        ALIGN(size.x, 16), &lc->label_dsc, text);

    img.pixels = img_dsc->data;
    img.w = img_dsc->header.w;
    img.h = img_dsc->header.h;
    obj = lv_gl_obj_create(parent, GL_OBJ_TYPE_2D,
        img.w, img.h);
    lv_gl_obj_import_img(obj, &img);

    return obj;
}

void common_anim_start(void)
{
    lv_obj_add_flag(img1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(photo_box, LV_OBJ_FLAG_HIDDEN);
    lv_gl_obj_idle(obj_cube);
    lv_gl_obj_idle(obj_fold);
    lv_gl_obj_idle(obj_roller);
    lv_gl_set_viewport(NULL, NULL);
    anim_photo_stream_stop();
}

void gallery(void)
{
    int i;

    font_init();

    scr = lv_scr_act();
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_refr_size(scr);

    img1 = lv_img_create(scr);
    lv_img_set_src(img1, (void *)&pic1);
    lv_obj_center(img1);

    img2 = lv_img_create(scr);
    lv_img_set_src(img2, (void *)&pic2);
    lv_obj_center(img2);

    btn_mat = lv_btnmatrix_create(scr);
    lv_obj_set_size(btn_mat, lv_pct(100), lv_pct(20));
    lv_btnmatrix_set_map(btn_mat, btnm_map);
    lv_obj_align(btn_mat, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(btn_mat, event_handler,
        LV_EVENT_ALL, NULL);
    lv_obj_refr_size(btn_mat);

    crop.x = 0;
    crop.y = lv_obj_get_height(btn_mat);
    crop.w = lv_obj_get_width(scr);
    crop.h = lv_obj_get_height(scr) - crop.y;
    printf("%d %d %d %d\n", crop.x, crop.y, crop.w, crop.h);
    lv_gl_set_crop(NULL, &crop, 1);
    lv_gl_get_viewport(NULL, &view);

    lv_gl_img_t imgs[6];
    imgs[CUBE_LEFT].pixels   = pic1.data;
    imgs[CUBE_LEFT].w        = pic1.header.w;
    imgs[CUBE_LEFT].h        = pic1.header.h;
    imgs[CUBE_RIGHT].pixels  = pic1.data;
    imgs[CUBE_RIGHT].w       = pic1.header.w;
    imgs[CUBE_RIGHT].h       = pic1.header.h;
    imgs[CUBE_TOP].pixels    = pic1.data;
    imgs[CUBE_TOP].w         = pic1.header.w;
    imgs[CUBE_TOP].h         = pic1.header.h;
    imgs[CUBE_BOTTOM].pixels = pic2.data;
    imgs[CUBE_BOTTOM].w      = pic2.header.w;
    imgs[CUBE_BOTTOM].h      = pic2.header.h;
    imgs[CUBE_FRONT].pixels  = pic2.data;
    imgs[CUBE_FRONT].w       = pic2.header.w;
    imgs[CUBE_FRONT].h       = pic2.header.h;
    imgs[CUBE_BACK].pixels   = pic2.data;
    imgs[CUBE_BACK].w        = pic2.header.w;
    imgs[CUBE_BACK].h        = pic2.header.h;

    obj_cube = lv_gl_obj_create(NULL, GL_OBJ_TYPE_CUBE,
        480, 480);
    obj_cube->base.r.w /= 2;
    lv_gl_obj_import_img(obj_cube, imgs);
    printf("cube %p\n", obj_cube);

    imgs[0].pixels = pic1.data;
    imgs[0].w      = pic1.header.w;
    imgs[0].h      = pic1.header.h;
    imgs[1].pixels = pic2.data;
    imgs[1].w      = pic2.header.w;
    imgs[1].h      = pic2.header.h;
    obj_fold = lv_gl_obj_create(NULL, GL_OBJ_TYPE_FOLD,
        480, 480);
    lv_gl_obj_import_img(obj_fold, imgs);
    obj_fold->base.r.w = obj_fold->base.w * 0.5;
    obj_fold->base.r.h = obj_fold->base.h * 0.5;
    obj_fold->base.r.x = (crop.w - obj_fold->base.r.w) / 2;
    obj_fold->base.r.y = (crop.h - obj_fold->base.r.h) / 2;
    printf("fold %p\n", obj_fold);

    imgs[0].pixels = helloworld.data;
    imgs[0].w      = helloworld.header.w;
    imgs[0].h      = helloworld.header.h;
    obj_roller = lv_gl_obj_create(NULL, GL_OBJ_TYPE_2D,
        imgs[0].w, imgs[0].h);
    lv_gl_obj_import_img(obj_roller, imgs);
    printf("roller %p\n", obj_roller);

    slider = lv_slider_create(scr);
    lv_obj_set_style_bg_opa(slider, LV_OPA_TRANSP,
        LV_PART_KNOB);
    lv_obj_set_size(slider, lv_pct(100), 2);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 0, LV_ANIM_ON);
    lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 0, 0);

    label_canvas *lc =
        create_canvas(lv_palette_main(LV_PALETTE_ORANGE),
            ttf_main.font);
    obj_lyrics = lv_mem_alloc(lines * sizeof(*obj_lyrics));
    for (i = 0; i < lines; i++)
    {
        uint32_t byte_id = 0;
        uint32_t last_byte_id = 0;
        uint32_t word;
        uint32_t words = 0;
        char buf[5];
        while (1)
        {
            word = _lv_txt_encoded_next(lyric[i], &byte_id);
            if (!word)
                break;
            words++;
        }
        obj_lyrics[i].objs =
            lv_mem_alloc(words * sizeof(lv_gl_obj_t *));
        obj_lyrics[i].len = words;
        byte_id = 0;
        for (int j = 0; j < words; j++)
        {
            last_byte_id = byte_id;
            _lv_txt_encoded_next(lyric[i], &byte_id);
            memcpy(buf, lyric[i] + last_byte_id,
                byte_id - last_byte_id);
            buf[byte_id - last_byte_id] = 0;
            obj_lyrics[i].objs[j] =
                utf8_to_obj(obj_cube, lc, buf);
        }
    }

    int box_w, box_h;
    int start_x, start_y;
    box_h = lines / 2 * obj_lyrics[0].objs[0]->base.r.h;
    start_y = (obj_cube->base.w - box_h) / 2;
    for (i = 0; i < lines / 2; i++)
    {
        box_w = obj_lyrics[i].len *
            obj_lyrics[i].objs[0]->base.r.w;
        for (int j = 0; j < obj_lyrics[i].len; j++)
        {
            start_x = (obj_cube->base.w - box_w) / 2;
            obj_lyrics[i].objs[j]->out_type =
                GL_TEXTURE_CUBE_MAP_NEGATIVE_X + CUBE_FRONT;
            obj_lyrics[i].objs[j]->base.r.x = start_x +
                j * obj_lyrics[i].objs[j]->base.r.w;
            obj_lyrics[i].objs[j]->base.r.y = start_y +
                i * obj_lyrics[i].objs[j]->base.r.h;
        }
    }
    box_h = (lines - lines / 2) *
        obj_lyrics[0].objs[0]->base.r.h;
    start_y = (obj_cube->base.w - box_h) / 2;
    for (; i < lines; i++)
    {
        box_w = obj_lyrics[i].len *
            obj_lyrics[i].objs[0]->base.r.w;
        for (int j = 0; j < obj_lyrics[i].len; j++)
        {
            start_x = (obj_cube->base.w - box_w) / 2;
            obj_lyrics[i].objs[j]->out_type =
                GL_TEXTURE_CUBE_MAP_NEGATIVE_X + CUBE_LEFT;
            obj_lyrics[i].objs[j]->base.r.x = start_x +
                j * obj_lyrics[i].objs[j]->base.r.w;
            obj_lyrics[i].objs[j]->base.r.y = start_y +
                (i - lines / 2) * obj_lyrics[i].objs[j]->base.r.h;
        }
    }

    photo_box = lv_obj_create(scr);
    lv_obj_remove_style_all(photo_box);
    lv_obj_set_size(photo_box, crop.w, crop.h);
    lv_obj_add_flag(photo_box, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(photo_box, LV_OBJ_FLAG_SCROLLABLE);
    for (int i = 0; i < 4; i++)
    {
        photos[i] = lv_img_create(photo_box);
        lv_obj_set_pos(photos[i], 0, 500 * i);
    }
    lv_img_set_src(photos[0], &pic1);
    lv_img_set_src(photos[1], &pic2);
    lv_img_set_src(photos[2], &pic3);
    lv_img_set_src(photos[3], &pic4);
}

