/**
 * @file lv_draw_buf_rga.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_draw_rga.h"

#if LV_USE_DRAW_RGA

#include <rk_mpi_mmz.h>
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

static lv_draw_buf_t * lv_draw_buf_create_rockit(uint32_t w, uint32_t h, lv_color_format_t cf, uint32_t stride);
static void lv_draw_buf_destroy_rockit(lv_draw_buf_t * buf);

static void lv_draw_buf_clear_rga(lv_draw_buf_t * draw_buf, const lv_area_t * a);
static void lv_draw_buf_copy_rga(lv_draw_buf_t * dest, const lv_area_t * dest_area, const lv_draw_buf_t * src, const lv_area_t * src_area);
static lv_draw_buf_t * lv_draw_buf_dup_rga(const lv_draw_buf_t * draw_buf);
static uint32_t _calculate_draw_buf_size(uint32_t w, uint32_t h, lv_color_format_t cf, uint32_t stride);

/**********************
 *  STATIC VARIABLES
 **********************/

static lv_draw_buf_handlers_t default_handlers;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_buf_rga_init_handlers(void)
{
    lv_draw_buf_handlers_t * handlers = lv_draw_buf_get_handlers();

    memcpy(&default_handlers, handlers, sizeof(lv_draw_buf_handlers_t));

    handlers->create_cb = lv_draw_buf_create_rockit;
    handlers->destroy_cb = lv_draw_buf_destroy_rockit;

    handlers->clear_cb = lv_draw_buf_clear_rga;
    handlers->copy_cb = lv_draw_buf_copy_rga;
    handlers->dup_cb = lv_draw_buf_dup_rga;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_draw_buf_t * lv_draw_buf_create_rockit(uint32_t w, uint32_t h, lv_color_format_t cf, uint32_t stride)
{
    MB_BLK blk;
    uint32_t size;

    /* It's too small has no need to use dma buffer */
    //if ((w < LV_HOR_RES) || (h < LV_VER_RES))
    //    goto fallback;
    if ((w < 64) || (h < 64))
        goto fallback;

    lv_draw_buf_t * draw_buf = lv_malloc_zeroed(sizeof(lv_draw_buf_t));
    LV_ASSERT_MALLOC(draw_buf);
    if(draw_buf == NULL) return NULL;

    if(stride == 0) stride = lv_draw_buf_width_to_stride(w, cf);

    size = _calculate_draw_buf_size(w, h, cf, stride);

    if (RK_MPI_MMZ_Alloc(&blk, size, RK_MMZ_ALLOC_CACHEABLE) != 0) {
        LV_LOG_WARN("RK_MPI_MMZ_Alloc failed, fallback to default creator, w=%d h=%d", w, h);
        lv_free(draw_buf);
        goto fallback;
    }

    draw_buf->header.w = w;
    draw_buf->header.h = h;
    draw_buf->header.cf = cf;
    draw_buf->header.flags = LV_IMAGE_FLAGS_MODIFIABLE | LV_IMAGE_FLAGS_ALLOCATED | LV_IMAGE_FLAGS_RGA;
    draw_buf->header.stride = stride;
    draw_buf->header.magic = LV_IMAGE_HEADER_MAGIC;
    draw_buf->data = lv_draw_buf_align(RK_MPI_MMZ_Handle2VirAddr(blk), cf);
    draw_buf->unaligned_data = (void *)blk;
    draw_buf->data_size = size;

    return draw_buf;

fallback:

    return default_handlers.create_cb(w, h, cf, stride);
}

static void lv_draw_buf_destroy_rockit(lv_draw_buf_t * buf)
{
    if(!lv_draw_buf_has_flag(buf, LV_IMAGE_FLAGS_RGA)) {
        default_handlers.destroy_cb(buf);
        return;
    }
    RK_MPI_MMZ_Free((MB_BLK)buf->unaligned_data);
    lv_free(buf);
}

static void lv_draw_buf_clear_rga(lv_draw_buf_t * draw_buf, const lv_area_t * a)
{
    int dst_width, dst_height, dst_stride, dst_format;
    rga_buffer_t src;
    im_rect src_rect;
    rga_buffer_t dst;
    im_rect dst_rect;
    int dst_fd;
    int ret;

    if(!lv_draw_buf_has_flag(draw_buf, LV_IMAGE_FLAGS_RGA)) goto fallback;

    memset(&src, 0, sizeof(src));
    memset(&src_rect, 0, sizeof(src_rect));

    dst_width = draw_buf->header.w;
    dst_height = draw_buf->header.h;
    dst_stride = draw_buf->header.stride / (lv_color_format_get_bpp(draw_buf->header.cf) >> 3);
    dst_format = fmt_lv_to_rga(draw_buf->header.cf);
    dst_fd = RK_MPI_MMZ_Handle2Fd((MB_BLK)draw_buf->unaligned_data);
    dst = wrapbuffer_fd(dst_fd, dst_width, dst_height, dst_format, dst_stride, dst_height);

    if (a) {
        dst_rect.x = a->x1;
        dst_rect.y = a->y1;
        dst_rect.width = lv_area_get_width(a);
        dst_rect.height = lv_area_get_height(a);
    } else {
        memset(&dst_rect, 0, sizeof(dst_rect));
    }

    ret = imcheck(src, dst, src_rect, dst_rect, IM_COLOR_FILL);
    if (ret != IM_STATUS_NOERROR) {
        LV_LOG_INFO("imcheck error! %s", imStrError((IM_STATUS)ret));
        goto fallback;
    }

    ret = imfill(dst, dst_rect, 0x0);
    if (ret == IM_STATUS_SUCCESS) {
        LV_LOG_INFO("imrectangle error! %s", imStrError((IM_STATUS)ret));
        goto fallback;
    }

    return;

fallback:
    default_handlers.clear_cb(draw_buf, a);
}

static void lv_draw_buf_copy_rga(lv_draw_buf_t * dest, const lv_area_t * dest_area, const lv_draw_buf_t * src, const lv_area_t * src_area)
{
    int src_width, src_height, src_stride, src_format;
    int dst_width, dst_height, dst_stride, dst_format;
    rga_buffer_t src_img;
    rga_buffer_t dst_img;
    im_rect src_rect;
    im_rect dst_rect;
    int src_fd;
    int dst_fd;
    int ret;

    if(!lv_draw_buf_has_flag(dest, LV_IMAGE_FLAGS_RGA) ||
       !lv_draw_buf_has_flag((lv_draw_buf_t *)src, LV_IMAGE_FLAGS_RGA)) {
        goto fallback;
    }

    src_width = src->header.w;
    src_height = src->header.h;
    src_stride = src->header.stride / (lv_color_format_get_bpp(src->header.cf) >> 3);
    src_format = fmt_lv_to_rga(src->header.cf);
    src_fd = RK_MPI_MMZ_Handle2Fd((MB_BLK)src->unaligned_data);
    src_img = wrapbuffer_fd(src_fd, src_width, src_height, src_format, src_stride, src_height);

    if (src_area) {
        src_rect.x = src_area->x1;
        src_rect.y = src_area->y1;
        src_rect.width = lv_area_get_width(src_area);
        src_rect.height = lv_area_get_height(src_area);
    } else {
        memset(&src_rect, 0, sizeof(src_rect));
    }

    dst_width = dest->header.w;
    dst_height = dest->header.h;
    dst_stride = dest->header.stride / (lv_color_format_get_bpp(dest->header.cf) >> 3);
    dst_format = fmt_lv_to_rga(dest->header.cf);
    dst_fd = RK_MPI_MMZ_Handle2Fd((MB_BLK)dest->unaligned_data);
    dst_img = wrapbuffer_fd(dst_fd, dst_width, dst_height, dst_format, dst_stride, dst_height);

    if (dest_area) {
        dst_rect.x = dest_area->x1;
        dst_rect.y = dest_area->y1;
        dst_rect.width = lv_area_get_width(dest_area);
        dst_rect.height = lv_area_get_height(dest_area);
    } else {
        memset(&dst_rect, 0, sizeof(dst_rect));
    }

    ret = imcheck(src_img, dst_img, src_rect, dst_rect);
    if (ret != IM_STATUS_NOERROR) {
        LV_LOG_INFO("imcheck error! %s", imStrError((IM_STATUS)ret));
        goto fallback;
    }

    ret = imcopy(src_img, dst_img);
    if (ret == IM_STATUS_SUCCESS) {
        LV_LOG_INFO("imrectangle error! %s", imStrError((IM_STATUS)ret));
        goto fallback;
    }

    return;

fallback:
    default_handlers.copy_cb(dest, dest_area, src, src_area);
}

static lv_draw_buf_t * lv_draw_buf_dup_rga(const lv_draw_buf_t * draw_buf)
{
    lv_draw_buf_t * new_buf;

    if(!lv_draw_buf_has_flag((lv_draw_buf_t *)draw_buf, LV_IMAGE_FLAGS_RGA))
        return default_handlers.dup_cb(draw_buf);

    new_buf = lv_draw_buf_create_rockit(draw_buf->header.w, draw_buf->header.h, draw_buf->header.cf, draw_buf->header.stride);

    if (!new_buf) return NULL;

    lv_draw_buf_copy_rga(new_buf, NULL, draw_buf, NULL);
}

static uint32_t _calculate_draw_buf_size(uint32_t w, uint32_t h, lv_color_format_t cf, uint32_t stride)
{
    uint32_t size;

    if(stride == 0) stride = lv_draw_buf_width_to_stride(w, cf);

    size = stride * h;
    if(cf == LV_COLOR_FORMAT_RGB565A8) {
        size += (stride / 2) * h; /*A8 mask*/
    }
    else if(LV_COLOR_FORMAT_IS_INDEXED(cf)) {
        /*@todo we have to include palette right before image data*/
        size += LV_COLOR_INDEXED_PALETTE_SIZE(cf) * 4;
    }

    return size;
}
#endif /*LV_USE_DRAW_ROCKIT*/

