#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>

#if LV_USE_RKADK
#include <rkadk/rkadk_media_comm.h>
#include <rkadk/rkadk_ui.h>

// Concrete realization
/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_display_t *lv_rkadk_disp_create(lv_display_rotation_t rotate_disp);
void lv_rkadk_disp_delete(lv_display_t * disp);

#endif  /*USE_RKADK*/

#ifdef __cplusplus
} /* extern "C" */
#endif
