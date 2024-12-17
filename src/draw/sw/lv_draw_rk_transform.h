/**
 * @file lv_draw_rk_transform.h
 *
 */

#ifndef LV_DRAW_RK_TRANSFORM_H
#define LV_DRAW_RK_TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw.h"
#if LV_USE_DRAW_RK_TRANSFORM

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
 * Initialize rk transform. Called in internally.
 */
void lv_draw_rk_transform_init(void);

/**
 * Deinitialize rk transform.
 */
void lv_draw_rk_transform_deinit(void);

/**
 * Used internally to get a transformed area of an image
 * @param draw_unit     pointer to a draw unit
 * @param dest_area     the area to calculate, i.e. get this area from the transformed image
 * @param src_buf       the source buffer
 * @param src_w         source buffer width in pixels
 * @param src_h         source buffer height in pixels
 * @param src_stride    source buffer stride in bytes
 * @param dsc           the draw descriptor
 * @param sup           supplementary data
 * @param cf            color format of the source buffer
 * @param dest_buf      the destination buffer
 */
int lv_draw_rk_transform(lv_draw_unit_t * draw_unit, const lv_area_t * dest_area, const void * src_buf,
                         int32_t src_w, int32_t src_h, int32_t src_stride,
                         const lv_draw_image_dsc_t * draw_dsc, const lv_draw_image_sup_t * sup, lv_color_format_t cf, void * dest_buf);

#endif /*LV_USE_DRAW_SW*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_SW_H*/
