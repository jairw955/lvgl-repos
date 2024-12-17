/**
 * @file lv_draw_rk_transform.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_sw.h"
#if LV_USE_DRAW_RK_TRANSFORM

#include <unistd.h>
#include "../../misc/lv_area.h"
#include "libRkScalerApi.h"

/*********************
 *      DEFINES
 *********************/

#ifndef LV_DRAW_RK_TRANSFORM_SCALER_METHOD
    #define LV_DRAW_RK_TRANSFORM_SCALER_METHOD SCALER_METHOD_NEAREST
#endif

#ifndef LV_DRAW_RK_TRANSFORM_SCALER_CORES
    #define LV_DRAW_RK_TRANSFORM_SCALER_CORES 3
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static RkScalerContext scalerContext;
static bool rkscaler_inited = false;
static int rkscaler_cores = LV_DRAW_RK_TRANSFORM_SCALER_CORES;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_rk_transform_init(void)
{
    if(!rkscaler_inited) {
        if(rkscaler_cores <= 0)
            rkscaler_cores = sysconf(_SC_NPROCESSORS_CONF);
        if(rkscaler_cores <= 0) {
            LV_LOG_WARN("Invalid number of cores");
            rkscaler_cores = 1;
        }
        int ret = RkScalerInit(&scalerContext, rkscaler_cores);
        if(ret) {
            LV_LOG_ERROR("Failed to init scaler context %d", ret);
            return;
        }
        RkScalerSetLoglevel(&scalerContext, 0);
        rkscaler_inited = true;
    }
}

void lv_draw_rk_transform_deinit(void)
{
    if(!rkscaler_inited)
        return;

    int ret = RkScalerDeinit(scalerContext);
    if(ret)
        LV_LOG_ERROR("Failed to deinit scaler context %d", ret);
    rkscaler_inited = false;
}

int lv_draw_rk_transform(lv_draw_unit_t * draw_unit, const lv_area_t * dest_area, const void * src_buf,
                         int32_t src_w, int32_t src_h, int32_t src_stride,
                         const lv_draw_image_dsc_t * draw_dsc, const lv_draw_image_sup_t * sup, lv_color_format_t src_cf, void * dest_buf)
{
    LV_UNUSED(draw_unit);
    LV_UNUSED(sup);

    /* Do not support rotation */
    if(draw_dsc->rotation)
        return LV_RESULT_INVALID;

    /* Only support ARGB for now */
    if((src_cf != LV_COLOR_FORMAT_ARGB8888) && (src_cf != LV_COLOR_FORMAT_XRGB8888))
        return LV_RESULT_INVALID;

    if(!rkscaler_inited) {
        lv_draw_rk_transform_init();
        if(!rkscaler_inited) {
            LV_LOG_ERROR("Failed to init scaler context");
            return LV_RESULT_INVALID;
        }
    }

    RkScalerParams scalerParam = {0};
    scalerParam.nMethodLuma = LV_DRAW_RK_TRANSFORM_SCALER_METHOD;
    scalerParam.nMethodChrm = LV_DRAW_RK_TRANSFORM_SCALER_METHOD;
    scalerParam.nCores = rkscaler_cores;
    scalerParam.nSrcWid = src_w;
    scalerParam.nSrcHgt = src_h;
    scalerParam.nSrcWStrides[0] = src_stride;
    scalerParam.nSrcHStrides[0] = 0;
    scalerParam.nSrcFmt = SCALER_FMT_ARGB8888;
    scalerParam.pSrcBufs[0] = (RK_U8 *)src_buf;
    scalerParam.pSrcBufs[1] = NULL;
    scalerParam.pSrcBufs[2] = NULL;
    scalerParam.nDstWid = lv_area_get_width(dest_area);
    scalerParam.nDstHgt = lv_area_get_height(dest_area);
    scalerParam.nDstWStrides[0] = 0;
    scalerParam.nDstHStrides[0] = 0;
    scalerParam.nDstFmt = SCALER_FMT_ARGB8888;
    scalerParam.pDstBufs[0] = (RK_U8 *)dest_buf;
    scalerParam.pDstBufs[1] = NULL;
    scalerParam.pDstBufs[2] = NULL;
    int ret = RkScalerProcessor(scalerContext, &scalerParam);
    if(ret) {
        LV_LOG_ERROR("failed to run processor %d", ret);
        return LV_RESULT_INVALID;
    }

    return LV_RESULT_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_USE_DRAW_RK_TRANSFORM*/
