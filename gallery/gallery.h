#ifndef __GALLERY_H__
#define __GALLERY_H__

#include <lvgl/lvgl.h>
#include <lvgl/lv_drivers/sdl/gl/gl.h>

#define ALIGN(x, a) (((x) + (a - 1)) & ~(a - 1))

typedef struct
{
    lv_draw_label_dsc_t label_dsc;
    lv_img_dsc_t *img_dsc;
    lv_obj_t *canvas;
    lv_point_t size;
} label_canvas;

typedef struct {
    lv_gl_obj_t **objs;
    int len;
} lyric_row;

extern lv_ft_info_t ttf_main;
extern lv_gl_obj_t *obj_cube;
extern lv_gl_obj_t *obj_fold;
extern lv_gl_obj_t *obj_roller;
extern lyric_row *obj_lyrics;
extern lv_obj_t *scr;
extern lv_obj_t *img1;
extern lv_obj_t *img2;
extern lv_obj_t *slider;
extern lv_obj_t *photo_box;
extern lv_obj_t *photos[4];
extern SDL_Rect crop;
extern SDL_Rect view;
extern int animing;
extern int lines;

extern const lv_img_dsc_t pic1;
extern const lv_img_dsc_t pic2;
extern const lv_img_dsc_t pic3;
extern const lv_img_dsc_t pic4;
extern const lv_img_dsc_t helloworld;

void common_anim_start(void);

#endif

