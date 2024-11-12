/**
 * @file lv_draw_rga_utils.h
 *
 */

#ifndef LV_DRAW_RGA_UTILS_H
#define LV_DRAW_RGA_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../../lvgl.h"

#if LV_USE_DRAW_RGA

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

uint32_t lv_color_to_rga(lv_color_t color);

int fmt_lv_to_rga(lv_color_format_t lv_fmt);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_RGA*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_RGA_UTILS_H*/
