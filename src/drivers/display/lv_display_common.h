#ifndef __LV_DISPLAY_COMMON_H__
#define __LV_DISPLAY_COMMON_H__

typedef struct overlay_dma_buffer
{
    void * data;
    int fd;
    int stride;

    int w;
    int h;
    int vw;
    int vh;
    int ofs_x;
    int ofs_y;
    int dst_x;
    int dst_y;

    void * user_data;
} overlay_dma_buffer_t;

#if defined(LV_USE_LINUX_DRM) && LV_USE_LINUX_DRM
overlay_dma_buffer_t *lv_drm_disp_create_overlay(lv_display_t * disp,
                                                 int w, int h);
void lv_drm_disp_destroy_overlay(lv_display_t * disp,
                                 overlay_dma_buffer_t * overlay);
void lv_drm_disp_set_overlay(lv_display_t * disp,
                             overlay_dma_buffer_t * overlay);
#define lv_display_driver_create_overlay lv_drm_disp_create_overlay
#define lv_display_driver_destroy_overlay lv_drm_disp_destroy_overlay
#define lv_display_driver_set_overlay lv_drm_disp_set_overlay
#elif defined(LV_USE_RKADK) && LV_USE_RKADK
overlay_dma_buffer_t *lv_rkadk_disp_create_overlay(lv_display_t * disp,
                                                   int w, int h);
void lv_rkadk_disp_destroy_overlay(lv_display_t * disp,
                                   overlay_dma_buffer_t * overlay);
void lv_rkadk_disp_set_overlay(lv_display_t * disp,
                               overlay_dma_buffer_t * overlay);
#define lv_display_driver_create_overlay lv_rkadk_disp_create_overlay
#define lv_display_driver_destroy_overlay lv_rkadk_disp_destroy_overlay
#define lv_display_driver_set_overlay lv_rkadk_disp_set_overlay
#endif

#endif
