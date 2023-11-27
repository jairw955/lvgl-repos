#ifndef __GL_H__
#define __GL_H__

#include <lvgl/lvgl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengles2.h>

enum {
    GL_OBJ_FMT_RGBA,
    GL_OBJ_FMT_BGRA,
};

enum {
    GL_OBJ_TYPE_2D,
    GL_OBJ_TYPE_CUBE,
    GL_OBJ_TYPE_FOLD,
};

enum {
    CUBE_LEFT,
    CUBE_RIGHT,
    CUBE_TOP,
    CUBE_BOTTOM,
    CUBE_FRONT,
    CUBE_BACK,
};

typedef struct {
    const void *pixels;
    int w;
    int h;
} lv_gl_img_t;

typedef struct {
    GLuint gl_tex;
    GLuint FBO;
    int w;
    int h;
} lv_gl_tex_t;

typedef struct {
    int x, y, z;
    int w, h, l;
} lv_gl_rect_t;

typedef struct {
    void *parent;
    int format;
    int type;
    int w;
    int h;
    int reverse_y;
    lv_ll_t ready_ll;
    lv_ll_t idle_ll;
    void *ll;
    lv_gl_tex_t *tex;
    SDL_Rect r;
    SDL_Rect view;
    SDL_Rect crop;
    int view_dirty;
    int crop_dirty;
    int crop_en;
} lv_gl_base_t;

typedef struct {
    lv_gl_base_t base;
    int always_ready;
    GLuint out_type;

    float alpha;
    float scale;
    float offset_z;
    float ax;
    float ay;
    float az;
    float view_ax;
    float view_ay;
    float view_az;
    int steps;
    int pos;
} lv_gl_obj_t;

static inline void lv_gl_obj_set_angle(lv_gl_obj_t *obj,
    float x, float y, float z)
{
    obj->ax = x;
    obj->ay = y;
    obj->az = z;
}

static inline void lv_gl_obj_set_view_angle(lv_gl_obj_t *obj,
    float x, float y, float z)
{
    obj->view_ax = x;
    obj->view_ay = y;
    obj->view_az = z;
}

int lv_gl_ctx_init(void);
void lv_gl_set_crop(lv_gl_obj_t *obj, SDL_Rect *r, int en);
void lv_gl_get_crop(lv_gl_obj_t *obj, SDL_Rect *r);
void lv_gl_set_viewport(lv_gl_obj_t *obj, SDL_Rect *r);
void lv_gl_get_viewport(lv_gl_obj_t *obj, SDL_Rect *r);
lv_gl_obj_t *lv_gl_obj_create(lv_gl_obj_t *parent,
    int type, int w, int h);
void lv_gl_obj_del(lv_gl_obj_t *obj);
lv_gl_tex_t *lv_gl_tex_create(int type, int w, int h);
void lv_gl_tex_del(lv_gl_tex_t *tex);
void lv_gl_obj_import_img(lv_gl_obj_t *obj,
    lv_gl_img_t *img);
void lv_gl_obj_ready(lv_gl_obj_t *obj, int always);
void lv_gl_obj_idle(lv_gl_obj_t *obj);
void lv_gl_render(void);

#endif

