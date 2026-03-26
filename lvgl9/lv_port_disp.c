/**
 * @file lv_port_disp.c
 *
 */

#include <stdlib.h>
#include <lvgl/lvgl.h>

#if defined(LV_USE_LINUX_DRM) && LV_USE_LINUX_DRM
#include <lvgl/src/drivers/display/drm/lv_linux_drm.h>
#endif

#if defined(LV_USE_SDL) && LV_USE_SDL
#include <lvgl/src/drivers/sdl/lv_sdl_window.h>
#include LV_SDL_INCLUDE_PATH
#endif

#if defined(LV_USE_RKADK) && LV_USE_RKADK
#include <lvgl/src/drivers/display/rkadk/rkadk.h>
#endif

void lv_port_disp_init(lv_coord_t hor_res, lv_coord_t ver_res, int rot, bool fullscreen)
{
    lv_display_rotation_t lvgl_rot = LV_DISPLAY_ROTATION_0;
    lv_display_t *disp;

    switch (rot)
    {
    case 0:
        lvgl_rot = LV_DISPLAY_ROTATION_0;
        break;
    case 90:
        lvgl_rot = LV_DISPLAY_ROTATION_90;
        break;
    case 180:
        lvgl_rot = LV_DISPLAY_ROTATION_180;
        break;
    case 270:
        lvgl_rot = LV_DISPLAY_ROTATION_270;
        break;
    default:
        LV_LOG_ERROR("Unsupported rotation %d", rot);
        break;
    }

#if defined(LV_USE_LINUX_DRM) && LV_USE_LINUX_DRM
    LV_LOG_USER("LV_USE_LINUX_DRM");
    const char *device = "/dev/dri/card0";
    disp = lv_linux_drm_create();
    lv_linux_drm_set_file(disp, device, -1);
#endif

#if defined(LV_USE_SDL) && LV_USE_SDL
    LV_LOG_USER("LV_USE_SDL %d %d %s", hor_res, ver_res, fullscreen ? "fullscreen" : "windowed");
    disp = lv_sdl_window_create(hor_res, ver_res);
    if (disp)
    {
        SDL_Window *sdl_win = lv_sdl_window_get_window(disp);
        if (sdl_win && fullscreen)
            SDL_SetWindowFullscreen(sdl_win, SDL_WINDOW_FULLSCREEN_DESKTOP);
        // lv_display_set_rotation(disp, lvgl_rot);
    }
#endif

#if defined(LV_USE_RKADK) && LV_USE_RKADK
    LV_LOG_USER("LV_USE_RKADK");
    disp = lv_rkadk_disp_create(lvgl_rot);
#endif
}

