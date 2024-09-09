/**
 * @file lv_png.h
 *
 */

#ifndef LV_PNG_H
#define LV_PNG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../../lv_conf_internal.h"
#if LV_USE_PNG

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Register the PNG decoder functions in LVGL
 */
lv_img_decoder_t* lv_png_init(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_PNG*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_PNG_H*/
