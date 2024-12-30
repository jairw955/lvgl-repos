/**
 * @file lv_port_init.c
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <lvgl/lvgl.h>
#include <lvgl/lv_conf.h>

#include "lv_port_disp.h"
#include "lv_port_indev.h"

/* 0, 90, 180, 270 */
static int g_indev_rotation = 0;

static int log_level = LV_LOG_LEVEL_WARN;
static void print_cb(const char *buf)
{
    printf("%s", buf);
}

void lv_port_init(int width, int height, int rotation)
{
    const char *buf;

    printf("LVGL-V%d.%d.%d\n", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
    lv_init();

    buf = getenv("LV_DEBUG");
    if (buf)
        log_level = buf[0] - '0';
    lv_log_register_print_cb(print_cb);

    lv_port_disp_init(width, height, rotation);
    lv_port_indev_init(g_indev_rotation + rotation);
}

