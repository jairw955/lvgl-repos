/**
 * @file lv_draw_rga_utils.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_draw_rga_utils.h"

#if LV_USE_DRAW_RGA

#include <rga/rga.h>

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

uint32_t lv_color_to_rga(lv_color_t color)
{
    return (uint32_t)((uint32_t)0xff << 24) + (color.blue << 16) + (color.green << 8) + (color.red);
}

int fmt_lv_to_rga(lv_color_format_t lv_fmt)
{
    switch (lv_fmt)
    {
    case LV_COLOR_FORMAT_L8: return RK_FORMAT_YCbCr_400;
    case LV_COLOR_FORMAT_I1: return RK_FORMAT_BPP1;
    case LV_COLOR_FORMAT_I2: return RK_FORMAT_BPP2;
    case LV_COLOR_FORMAT_I4: return RK_FORMAT_BPP4;
    case LV_COLOR_FORMAT_I8: return RK_FORMAT_BPP8;
    case LV_COLOR_FORMAT_A8: return RK_FORMAT_A8;

    case LV_COLOR_FORMAT_RGB565  : return RK_FORMAT_BGR_565;
    case LV_COLOR_FORMAT_ARGB8565: return RK_FORMAT_BGRA_8888;

    case LV_COLOR_FORMAT_RGB888  : return RK_FORMAT_BGR_888;
    case LV_COLOR_FORMAT_ARGB8888: return RK_FORMAT_BGRA_8888;
    case LV_COLOR_FORMAT_XRGB8888: return RK_FORMAT_BGRX_8888;

    case LV_COLOR_FORMAT_I420: return RK_FORMAT_YCbCr_420_P;
    case LV_COLOR_FORMAT_I422: return RK_FORMAT_YCbCr_422_P;
    /* HACK: no YUV444 in rga.h */
    case LV_COLOR_FORMAT_I444: return RK_FORMAT_BGR_888;
    case LV_COLOR_FORMAT_I400: return RK_FORMAT_YCbCr_400;
    case LV_COLOR_FORMAT_NV21: return RK_FORMAT_YCrCb_420_SP;
    case LV_COLOR_FORMAT_NV12: return RK_FORMAT_YCbCr_420_SP;
    case LV_COLOR_FORMAT_YUY2: return RK_FORMAT_YUYV_422;
    case LV_COLOR_FORMAT_UYVY: return RK_FORMAT_UYVY_422;

    default: break;
    }

    LV_LOG_ERROR("Unsupport format 0x%x", lv_fmt);

    return RK_FORMAT_UNKNOWN;
}
#endif

