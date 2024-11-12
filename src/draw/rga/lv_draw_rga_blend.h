/**
 * @file lv_draw_rga_blend.h
 *
 */

#ifndef LV_DRAW_RGA_BLEND_H
#define LV_DRAW_RGA_BLEND_H

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

void lv_draw_rga_blend_init(void);

void lv_draw_rga_blend_deinit(void);

int lv_draw_rga_blend_image(lv_layer_t * layer, const lv_draw_sw_blend_dsc_t * blend_dsc, lv_area_t *blend_area);

int lv_draw_rga_blend_color(lv_layer_t * layer, const lv_draw_sw_blend_dsc_t * blend_dsc, lv_area_t *blend_area);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_RGA*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_RGA_BLEND_H*/

