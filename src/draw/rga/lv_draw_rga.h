/**
 * @file lv_draw_rga.h
 *
 */

#ifndef LV_DRAW_RGA_H
#define LV_DRAW_RGA_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../../lv_conf_internal.h"

#if LV_USE_DRAW_RGA

#include "lv_draw_buf_rga.h"
#include "lv_draw_rga_blend.h"
#include "lv_draw_rga_utils.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_draw_rga_init(void);

void lv_draw_rga_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_RGA*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_RGA_H*/
