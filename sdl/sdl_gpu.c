/**
 * @file sdl_gpu.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "sdl_gpu.h"
#if USE_SDL_GPU

#if LV_USE_GPU_SDL == 0
# error "LV_USE_DRAW_SDL must be enabled"
#endif

#if USE_KEYBOARD
# warning "KEYBOARD is deprecated, use SDL instead. See lv_drivers/sdl/sdl.c"
#endif

#if USE_MOUSE
# warning "MOUSE is deprecated, use SDL instead. See lv_drivers/sdl/sdl.c"
#endif

#if USE_MOUSEWHEEL
# warning "MOUSEWHEEL is deprecated, use SDL instead that. See lv_drivers/sdl/sdl.c"
#endif

#if USE_MONITOR
# error "Cannot enable both MONITOR and SDL at the same time. "
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <lvgl/src/draw/sdl/lv_draw_sdl.h>
#include SDL_INCLUDE_PATH

#include "gl/gl.h"

/*********************
 *      DEFINES
 *********************/
#ifndef KEYBOARD_BUFFER_SIZE
#define KEYBOARD_BUFFER_SIZE SDL_TEXTINPUTEVENT_TEXT_SIZE
#endif

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_draw_sdl_drv_param_t drv_param;
    lv_coord_t hor_res;
    lv_coord_t ver_res;
    lv_coord_t d_hor_res;
    lv_coord_t d_ver_res;
    SDL_Window * window;
    SDL_Texture * texture;
#if USE_SDL_OPENGL
    SDL_GLContext context;
#endif
    int rotated;
}monitor_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void window_create(monitor_t * m);
static void window_update(lv_disp_drv_t *disp_drv, void * buf);
static void monitor_sdl_clean_up(void);
static void sdl_event_handler(lv_timer_t * t);

/***********************
 *   GLOBAL PROTOTYPES
 ***********************/

static volatile bool sdl_inited = false;


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void sdl_init(void)
{
    /*Initialize the SDL*/
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    SDL_SetEventFilter(quit_filter, NULL);

    sdl_inited = true;

    SDL_StartTextInput();

    lv_timer_create(sdl_event_handler, 1, NULL);
}

void sdl_disp_drv_init(lv_disp_drv_t * disp_drv, lv_coord_t hor_res, lv_coord_t ver_res)
{
    int rotated = disp_drv->rotated <= LV_DISP_ROT_270 ? disp_drv->rotated : LV_DISP_ROT_NONE;
    monitor_t *m = lv_mem_alloc(sizeof(monitor_t));
    m->hor_res = hor_res;
    m->ver_res = ver_res;
    m->rotated = rotated;
    window_create(m);
    hor_res = m->hor_res;
    ver_res = m->ver_res;
    lv_disp_drv_init(disp_drv);
    disp_drv->direct_mode = 1;
    disp_drv->flush_cb = monitor_flush;
    disp_drv->hor_res = m->d_hor_res;
    disp_drv->ver_res = m->d_ver_res;
    disp_drv->rotated = LV_DISP_ROT_NONE;
    lv_disp_draw_buf_t *disp_buf = lv_mem_alloc(sizeof(lv_disp_draw_buf_t));
    lv_disp_draw_buf_init(disp_buf, m->texture, NULL, hor_res * ver_res);
    disp_drv->draw_buf = disp_buf;
    disp_drv->antialiasing = 1;
    disp_drv->user_data = &m->drv_param;
}

/**
 * Flush a buffer to the marked area
 * @param disp_drv pointer to driver where this function belongs
 * @param area an area where to copy `color_p`
 * @param color_p an array of pixels to copy to the `area` part of the screen
 */
void sdl_display_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    lv_coord_t hres = disp_drv->hor_res;
    lv_coord_t vres = disp_drv->ver_res;

//    printf("x1:%d,y1:%d,x2:%d,y2:%d\n", area->x1, area->y1, area->x2, area->y2);

    /*Return if the area is out the screen*/
    if(area->x2 < 0 || area->y2 < 0 || area->x1 > hres - 1 || area->y1 > vres - 1) {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    /* TYPICALLY YOU DO NOT NEED THIS
     * If it was the last part to refresh update the texture of the window.*/
    if(lv_disp_flush_is_last(disp_drv)) {
        window_update(disp_drv, color_p);
    }

    /*IMPORTANT! It must be called to tell the system the flush is ready*/
    lv_disp_flush_ready(disp_drv);

}

void sdl_display_resize(lv_disp_t *disp, int width, int height)
{
    lv_disp_drv_t *driver = disp->driver;
    monitor_t *m = (monitor_t *)driver->user_data;
    SDL_Renderer *renderer = ((lv_draw_sdl_drv_param_t *) driver->user_data)->renderer;
    if (driver->draw_buf->buf1) {
        SDL_DestroyTexture(driver->draw_buf->buf1);
    }
    if (m->rotated == LV_DISP_ROT_90 ||
        m->rotated == LV_DISP_ROT_270) {
        m->d_hor_res = height;
        m->d_ver_res = width;
    } else {
        m->d_hor_res = width;
        m->d_ver_res = height;
    }
    m->hor_res = width;
    m->ver_res = height;
    SDL_Texture *texture = lv_draw_sdl_create_screen_texture(renderer, m->d_hor_res, m->d_ver_res);
    lv_disp_draw_buf_init(driver->draw_buf, texture, NULL, width * height);
    driver->hor_res = (lv_coord_t) m->d_hor_res;
    driver->ver_res = (lv_coord_t) m->d_ver_res;
    SDL_RendererInfo renderer_info;
    SDL_GetRendererInfo(renderer, &renderer_info);
    SDL_assert(renderer_info.flags & SDL_RENDERER_TARGETTEXTURE);
    SDL_SetRenderTarget(renderer, texture);
    lv_disp_drv_update(disp, driver);
}


/**********************
 *   STATIC FUNCTIONS
 **********************/


/**
 * SDL main thread. All SDL related task have to be handled here!
 * It initializes SDL, handles drawing and the mouse.
 */

static void sdl_event_handler(lv_timer_t * t)
{
    (void)t;

    /*Refresh handling*/
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        mouse_handler(&event);
        mousewheel_handler(&event);
        keyboard_handler(&event);

        switch (event.type) {
            case SDL_WINDOWEVENT: {
                SDL_Window * window = SDL_GetWindowFromID(event.window.windowID);
                switch (event.window.event) {
#if SDL_VERSION_ATLEAST(2, 0, 5)
                    case SDL_WINDOWEVENT_TAKE_FOCUS:
#endif
                    case SDL_WINDOWEVENT_EXPOSED:
                        for (lv_disp_t *cur = lv_disp_get_next(NULL); cur; cur = lv_disp_get_next(cur)) {
                            window_update(cur->driver, cur->driver->draw_buf->buf_act);
                        }
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        for (lv_disp_t *cur = lv_disp_get_next(NULL); cur; cur = lv_disp_get_next(cur)) {
                            lv_draw_sdl_drv_param_t *param = cur->driver->user_data;
                            SDL_Renderer *renderer = SDL_GetRenderer(window);
                            if (param->renderer != renderer) continue;
                            int w, h;
                            SDL_GetWindowSize(window, &w, &h);
                            sdl_display_resize(cur, w, h);
                        }
                        break;
                    }
                    case SDL_WINDOWEVENT_CLOSE: {
                        for (lv_disp_t *cur = lv_disp_get_next(NULL); cur; ) {
                            lv_disp_t * tmp = cur;
                            cur = lv_disp_get_next(tmp);
                            monitor_t * m = tmp->driver->user_data;
                            SDL_Renderer *renderer = SDL_GetRenderer(window);
                            if (m->drv_param.renderer != renderer) continue;
                            SDL_DestroyTexture(tmp->driver->draw_buf->buf1);
                            SDL_DestroyRenderer(m->drv_param.renderer);
                            lv_disp_remove(tmp);
                        }

                        break;
                    }
                    default:
                        break;
                }
                break;
            }
        }
    }

    /*Run until quit event not arrives*/
    if(sdl_quit_qry) {
        monitor_sdl_clean_up();
        exit(0);
    }
}

int monitor_rotated(void)
{
    int rotated = LV_DISP_ROT_NONE;

    lv_disp_t *cur = lv_disp_get_next(NULL);
    if (cur) {
        lv_disp_t * tmp = cur;
        monitor_t * m = tmp->driver->user_data;
        rotated = m->rotated;
    }

    return rotated;
}

static void monitor_sdl_clean_up(void)
{
    for (lv_disp_t *cur = lv_disp_get_next(NULL); cur; ) {
        lv_disp_t * tmp = cur;
        monitor_t * m = tmp->driver->user_data;
        SDL_DestroyTexture(tmp->driver->draw_buf->buf1);
        SDL_DestroyRenderer(m->drv_param.renderer);
        cur = lv_disp_get_next(cur);
        lv_disp_remove(tmp);
    }

    SDL_Quit();
}

static void window_create(monitor_t * m)
{
#if USE_SDL_OPENGL
    SDL_GLContext previousContext;
#endif
#if LV_COLOR_SCREEN_TRANSP && defined(SDL_HINT_VIDEO_EGL_ALLOW_TRANSPARENCY)
    SDL_SetHint(SDL_HINT_VIDEO_EGL_ALLOW_TRANSPARENCY, "1");
#endif
//    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_Rect rect;

    if (!m->hor_res || !m->ver_res) {
        SDL_GetDisplayBounds(0, &rect);
        m->hor_res = rect.w;
        m->ver_res = rect.h;
    }

    if (m->rotated == LV_DISP_ROT_90 ||
        m->rotated == LV_DISP_ROT_270) {
        m->d_hor_res = m->ver_res;
        m->d_ver_res = m->hor_res;
    } else {
        m->d_hor_res = m->hor_res;
        m->d_ver_res = m->ver_res;
    }
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
    m->window = SDL_CreateWindow("TFT Simulator",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              m->hor_res * SDL_ZOOM, m->ver_res * SDL_ZOOM,
#ifndef SDL_DIS_FULLSCREEN
                              SDL_WINDOW_FULLSCREEN |
#endif
                              SDL_WINDOW_OPENGL);

    m->drv_param.renderer = SDL_CreateRenderer(m->window, -1, SDL_RENDERER_ACCELERATED);

    m->texture = lv_draw_sdl_create_screen_texture(m->drv_param.renderer, m->d_hor_res, m->d_ver_res);
    /* For first frame */
    SDL_SetRenderTarget(m->drv_param.renderer, m->texture);

#if USE_SDL_OPENGL
    previousContext = SDL_GL_GetCurrentContext();
    m->context = SDL_GL_CreateContext(m->window);
    SDL_GL_MakeCurrent(m->window, m->context);
    lv_gl_ctx_init();
    SDL_GL_MakeCurrent(m->window, previousContext);
#endif
}

static void window_update(lv_disp_drv_t *disp_drv, void * buf)
{
#if USE_SDL_OPENGL
    SDL_GLContext previousContext;
#endif
    SDL_Renderer *renderer = ((lv_draw_sdl_drv_param_t *) disp_drv->user_data)->renderer;
    monitor_t *m = (monitor_t *)disp_drv->user_data;
    SDL_Texture *texture = buf;
    SDL_Rect dst;

    SDL_SetRenderTarget(renderer, NULL);
#if LV_COLOR_SCREEN_TRANSP
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
#endif
    SDL_RenderClear(renderer);

    dst.x = (m->hor_res - m->d_hor_res) / 2;
    dst.y = (m->ver_res - m->d_ver_res) / 2;
    dst.w = m->d_hor_res;
    dst.h = m->d_ver_res;
    /*Update the renderer with the texture containing the rendered image*/

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_RenderSetClipRect(renderer, NULL);
    SDL_RenderCopyEx(renderer, texture, NULL, &dst, 90.0 * m->rotated, NULL, SDL_FLIP_NONE);
#if USE_SDL_OPENGL
    SDL_RenderFlush(renderer);

    previousContext = SDL_GL_GetCurrentContext();
    SDL_GL_MakeCurrent(m->window, m->context);
    lv_gl_render();
    SDL_GL_MakeCurrent(m->window, previousContext);
#endif
    SDL_RenderPresent(renderer);
    SDL_SetRenderTarget(renderer, texture);
}

#endif /*USE_SDL_GPU*/
