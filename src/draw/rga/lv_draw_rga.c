/**
 * @file lv_rga_draw.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_draw_rga.h"

#if LV_USE_DRAW_RGA

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_rga_init(void)
{
    lv_draw_buf_rga_init_handlers();
    lv_draw_rga_blend_init();
}

void lv_draw_rga_deinit(void)
{
    lv_draw_rga_blend_deinit();
}

#endif /*LV_USE_DRAW_RGA*/

