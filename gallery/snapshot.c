#include <lvgl/lvgl.h>

#include "gallery.h"

#ifdef USE_OPENGL
#include <lvgl/lv_drivers/sdl/gl/gl.h>
#endif

static int screen_shot = 0;

static void save_to_file(char *ptr, int stride, int h,
    int reverse, const char *func)
{
    static int idx = 0;
    char path[128];
    FILE *fd;

    snprintf(path, sizeof(path), "/tmp/%s_%d_%dx%d_rgba.rgb", func,
        idx++, stride / 4, h);
    fd = fopen(path, "wb+");
    if (!fd)
    {
        printf("save to %s failed\n", path);
        return;
    }

    if (reverse)
        ptr = ptr + stride * (h - 1);

    for (int i = 0; i < h; i++)
    {
        fwrite(ptr, 1, stride, fd);
        if (reverse)
            ptr -= stride;
        else
            ptr += stride;
    }
    fclose(fd);

    printf("save to %s\n", path);
}

static void take_shot_lv(const char *func)
{
    lv_img_dsc_t *snapshot;

    snapshot = lv_snapshot_take(anim_area, LV_IMG_CF_TRUE_COLOR);
    save_to_file((char *)snapshot->data,
        snapshot->header.w * 4, snapshot->header.h, 0, func);

    lv_snapshot_free(snapshot);
}

static void take_shot_gl(const char *func)
{
#ifdef USE_OPENGL
    char *ptr;
    int size;
    SDL_Rect v;
    SDL_Rect r;

    r.x = view.x;
    r.y = view.y;
    r.w = view.w;
    r.h = view.h;

    size = r.w * r.h * 4;
    ptr = malloc(size);
    lv_gl_read_pixels(ptr, &r, 0);

    save_to_file((char *)ptr, r.w * 4, r.h, 1, func);

    free(ptr);
#endif
}

void take_shot_mark(void)
{
    screen_shot = 1;
}

void _take_shot(const char *func)
{
    if (!screen_shot)
        return;
    screen_shot = 0;

    if (lv_obj_has_flag(anim_area, LV_OBJ_FLAG_HIDDEN))
        take_shot_gl(func);
    else
        take_shot_lv(func);
}

void _take_shot_now(const char *func)
{
    screen_shot = 1;
    _take_shot(func);
}

