/**
 * @file lv_draw_rga_blend.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_draw_rga.h"

#if LV_USE_DRAW_RGA

#include <rk_mpi_mmz.h>
#include <rk_mpi_sys.h>
#include <rga/rga.h>
#include <rga/im2d.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void rga_fill(rga_buffer_t dst, im_rect dst_rect, uint32_t color);
static void rga_blend(rga_buffer_t dst, im_rect dst_rect, rga_buffer_t src, im_rect src_rect, int usage);

/**********************
 *  STATIC VARIABLES
 **********************/

static lv_draw_buf_t * rga_draw_buf = NULL;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_draw_rga_blend_init(void)
{
    rga_draw_buf = NULL;
}

void lv_draw_rga_blend_deinit(void)
{
    if (rga_draw_buf) {
        lv_draw_buf_destroy(rga_draw_buf);
        rga_draw_buf = NULL;
    }
}

int lv_draw_rga_blend_image(lv_layer_t * layer, const lv_draw_sw_blend_dsc_t * blend_dsc, lv_area_t *blend_area)
{
    lv_draw_buf_t * src_buf;
    lv_draw_buf_t * dst_buf;

    im_rect src_rect;
    im_rect dst_rect;
    rga_buffer_t src_img;
    rga_buffer_t dst_img;

    int src_width, src_height, src_stride, src_format;
    int dst_width, dst_height, dst_stride, dst_format;

    int src_fd;
    int dst_fd;
    int usage = 0;
    int ret = LV_RESULT_INVALID;

    LV_PROFILER_BEGIN;

    if(!blend_dsc->src_buf_is_draw_buf) goto end;

    src_buf = (lv_draw_buf_t *)blend_dsc->src_buf;
    if(!lv_draw_buf_has_flag(src_buf, LV_IMAGE_FLAGS_RGA)) goto end;

    dst_buf = layer->draw_buf;
    if(!lv_draw_buf_has_flag(dst_buf, LV_IMAGE_FLAGS_RGA)) goto end;

    if(dst_buf->header.w < 2 || dst_buf->header.h < 2) goto end;

    src_rect.x = blend_area->x1 - blend_dsc->src_area->x1;
    src_rect.y = blend_area->y1 - blend_dsc->src_area->y1;
    src_rect.width = lv_area_get_width(blend_area);
    src_rect.height = lv_area_get_height(blend_area);
    if(src_rect.x == 1 || src_rect.y == 1 || src_rect.width < 2 || src_rect.height < 2) goto end;

    dst_rect.x = blend_area->x1 - layer->buf_area.x1;
    dst_rect.y = blend_area->y1 - layer->buf_area.y1;
    dst_rect.width = lv_area_get_width(blend_area);
    dst_rect.height = lv_area_get_height(blend_area);
    if(dst_rect.x == 1 || dst_rect.y == 1 || dst_rect.width < 2 || dst_rect.height < 2) goto end;

    src_fd = RK_MPI_MMZ_Handle2Fd((MB_BLK)src_buf->unaligned_data);
    src_width = src_buf->header.w;
    src_height = src_buf->header.h;
    src_stride = blend_dsc->src_stride / (lv_color_format_get_bpp(dst_buf->header.cf) >> 3);
    src_format = fmt_lv_to_rga(blend_dsc->src_color_format);
    src_img = wrapbuffer_fd(src_fd, src_width, src_height, src_format, src_stride, src_height);
    imsetOpacity(&src_img, blend_dsc->opa);

    dst_fd = RK_MPI_MMZ_Handle2Fd((MB_BLK)dst_buf->unaligned_data);
    dst_width = dst_buf->header.w;
    dst_height = dst_buf->header.h;
    dst_stride = dst_buf->header.stride / (lv_color_format_get_bpp(dst_buf->header.cf) >> 3);
    dst_format = fmt_lv_to_rga(dst_buf->header.cf);
    dst_img = wrapbuffer_fd(dst_fd, dst_width, dst_height, dst_format, dst_stride, dst_height);

    if (!lv_draw_buf_has_flag(src_buf, LV_IMAGE_FLAGS_PREMULTIPLIED))
        usage |= IM_ALPHA_BLEND_PRE_MUL;
    RK_MPI_SYS_MmzFlushCache((MB_BLK)src_buf->unaligned_data, RK_FALSE);
    RK_MPI_SYS_MmzFlushCache((MB_BLK)dst_buf->unaligned_data, RK_FALSE);
    rga_blend(dst_img, dst_rect, src_img, src_rect, usage);
    RK_MPI_SYS_MmzFlushCache((MB_BLK)dst_buf->unaligned_data, RK_FALSE);

    ret = LV_RESULT_OK;
end:
    LV_PROFILER_END;

    return ret;
}

int lv_draw_rga_blend_color(lv_layer_t * layer, const lv_draw_sw_blend_dsc_t * blend_dsc, lv_area_t *blend_area)
{
    lv_draw_buf_t * src_buf = NULL;
    lv_draw_buf_t * dst_buf;

    im_rect src_rect;
    im_rect dst_rect;
    rga_buffer_t src_img;
    rga_buffer_t dst_img;

    int src_width, src_height, src_stride, src_format;
    int dst_width, dst_height, dst_stride, dst_format;

    int src_fd;
    int dst_fd;
    int usage = 0;
    int ret = LV_RESULT_INVALID;

    LV_PROFILER_BEGIN;

    dst_buf = layer->draw_buf;
    if(!lv_draw_buf_has_flag(dst_buf, LV_IMAGE_FLAGS_RGA)) goto end;

    if(dst_buf->header.w < 2 || dst_buf->header.h < 2) goto end;

    dst_rect.x = blend_area->x1 - layer->buf_area.x1;
    dst_rect.y = blend_area->y1 - layer->buf_area.y1;
    dst_rect.width = lv_area_get_width(blend_area);
    dst_rect.height = lv_area_get_height(blend_area);
    if(dst_rect.x == 1 || dst_rect.y == 1 || dst_rect.width < 2 || dst_rect.height < 2) goto end;

    dst_fd = RK_MPI_MMZ_Handle2Fd((MB_BLK)dst_buf->unaligned_data);
    dst_width = dst_buf->header.w;
    dst_height = dst_buf->header.h;
    dst_stride = dst_buf->header.stride / (lv_color_format_get_bpp(dst_buf->header.cf) >> 3);
    dst_format = fmt_lv_to_rga(dst_buf->header.cf);
    dst_img = wrapbuffer_fd(dst_fd, dst_width, dst_height, dst_format, dst_stride, dst_height);

    uint32_t color = lv_color_to_rga(blend_dsc->color);
    RK_MPI_SYS_MmzFlushCache((MB_BLK)dst_buf->unaligned_data, RK_FALSE);
    if (blend_dsc->opa >= LV_OPA_MAX) {
        rga_fill(dst_img, dst_rect, color);
    } else {
        src_width = dst_rect.width;
        src_height = dst_rect.height;
        src_format = RK_FORMAT_BGRA_8888;
        if (rga_draw_buf) {
            src_buf = lv_draw_buf_reshape(rga_draw_buf,
                                          LV_COLOR_FORMAT_ARGB8888,
                                          src_width, src_height, 0);
        }
        if (!src_buf) {
            src_buf = lv_draw_buf_create(src_width, src_height,
                                         LV_COLOR_FORMAT_ARGB8888, 0);
        }
        if (!src_buf) goto end;

        if (!lv_draw_buf_has_flag(src_buf, LV_IMAGE_FLAGS_RGA)) {
            lv_draw_buf_destroy(src_buf);
            goto end;
        }

        src_fd = RK_MPI_MMZ_Handle2Fd((MB_BLK)src_buf->unaligned_data);
        src_img = wrapbuffer_fd(src_fd, src_width, src_height, src_format);
        memset(&src_rect, 0, sizeof(src_rect));
        rga_fill(src_img, src_rect, color);

        usage |= IM_ALPHA_BLEND_PRE_MUL;
        imsetOpacity(&src_img, blend_dsc->opa);
        rga_blend(dst_img, dst_rect, src_img, src_rect, usage);
        if (rga_draw_buf != src_buf) {
            if (rga_draw_buf)
                lv_draw_buf_destroy(rga_draw_buf);
            rga_draw_buf = src_buf;
        }
    }
    ret = LV_RESULT_OK;

end:
    LV_PROFILER_END;

    return ret;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void rga_fill(rga_buffer_t dst, im_rect dst_rect, uint32_t color)
{
    rga_buffer_t src;
    im_rect src_rect;
    uint32_t st, et;
    int ret;

    st = lv_tick_get();
    memset(&src, 0, sizeof(src));
    memset(&src_rect, 0, sizeof(src_rect));

    LV_PROFILER_BEGIN_TAG("rga_fill");
    ret = imcheck(src, dst, src_rect, dst_rect, IM_COLOR_FILL);
    if (ret == IM_STATUS_NOERROR) {
        ret = imfill(dst, dst_rect, color);
        if (ret != IM_STATUS_SUCCESS)
            LV_LOG_ERROR("imrectangle error! %s", imStrError((IM_STATUS)ret));
    } else {
        LV_LOG_ERROR("imcheck error! %s", imStrError((IM_STATUS)ret));
    }

    LV_PROFILER_END_TAG("rga_fill");
    et = lv_tick_elaps(st);
//    LV_LOG_USER("[%d %d %d %d] %d ms", dst_rect.x, dst_rect.y, dst_rect.width, dst_rect.height, et);
}

static void rga_blend(rga_buffer_t dst, im_rect dst_rect, rga_buffer_t src, im_rect src_rect, int usage)
{
    rga_buffer_t pat;
    im_rect pat_rect;
    uint32_t st, et;
    int ret;

    LV_PROFILER_BEGIN_TAG("rga_blend");
    st = lv_tick_get();
    memset(&pat, 0, sizeof(src));
    memset(&pat_rect, 0, sizeof(src_rect));

    usage |= IM_SYNC | IM_ALPHA_BLEND_SRC_OVER;
    ret = imcheck_composite(src, dst, pat, src_rect, dst_rect, pat_rect);
    if (ret == IM_STATUS_NOERROR) {
        ret = improcess(src, dst, pat, src_rect, dst_rect, pat_rect, usage);
        if (ret != IM_STATUS_SUCCESS)
            LV_LOG_ERROR("running failed, %s", imStrError((IM_STATUS)ret));
    }
    else {
        LV_LOG_ERROR("check error! %s", imStrError((IM_STATUS)ret));
    }
    et = lv_tick_elaps(st);
    LV_PROFILER_END_TAG("rga_blend");
//    LV_LOG_USER("[%d %d %d %d] [%d %d %d %d] %d ms",
//                src_rect.x, src_rect.y, src_rect.width, src_rect.height,
//                dst_rect.x, dst_rect.y, dst_rect.width, dst_rect.height, et);
}

#endif /*LV_USE_DRAW_RGA*/

