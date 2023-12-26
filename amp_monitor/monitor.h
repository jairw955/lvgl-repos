#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <lvgl/lvgl.h>
#include <lvgl/lv_drivers/sdl/gl/gl.h>

#define ALIGN(x, a) (((x) + (a - 1)) & ~(a - 1))

extern lv_ft_info_t ttf_main;
extern lv_obj_t *scr;
extern lv_obj_t *chart;

#endif

