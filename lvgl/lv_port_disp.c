/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/

#include "main.h"

static int lcd_w;
static int lcd_h;
static int lcd_sw;
static char* drm_buff;
static lv_color_t *buf_1;

static int quit = 0;
static pthread_t disp_thread_pid;
static pthread_mutex_t draw_mutex;
static int draw_update = 0;
static struct drm_bo *gbo;

static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

static void *disp_thread(void *arg)
{
    while (!quit) {
        pthread_mutex_lock(&draw_mutex);
        if (draw_update) {
            setdrmdisp(gbo);
            draw_update = 0;
        }
        pthread_mutex_unlock(&draw_mutex);
        usleep(10000);
    }
    return NULL;
}

void lv_port_disp_init(int rot)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */

    /* Example for 1) */
    static lv_disp_draw_buf_t draw_buf_dsc_1;
    buf_1 = memalign(64, lcd_w * lcd_h * 4);

    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, lcd_w * lcd_h);   /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = lcd_w;
    disp_drv.ver_res = lcd_h;

    disp_drv.sw_rotate = 0;
    disp_drv.rotated = LV_DISP_ROT_NONE;
    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_1;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
    pthread_mutex_init(&draw_mutex, NULL);
    pthread_create(&disp_thread_pid, NULL, disp_thread, NULL);
}

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
    drm_init(32);
    getdrmresolve(&lcd_w, &lcd_h);
    gbo = malloc_drm_bo(lcd_w, lcd_h, DRM_FORMAT_ARGB8888);
    drm_buff = gbo->ptr;
    lcd_sw = gbo->pitch / 4;
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    int32_t x;
    int32_t y;

    pthread_mutex_lock(&draw_mutex);
    for(y = area->y1; y <= area->y2; y++) {
        int area_w = area->x2 - area->x1 + 1;
        lv_color_t *disp = (lv_color_t*)(drm_buff + (y * lcd_sw + area->x1) * 4);
        memcpy(disp, color_p, area_w * 4);
        color_p += area_w;
    }
    draw_update = 1;
    pthread_mutex_unlock(&draw_mutex);
    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

