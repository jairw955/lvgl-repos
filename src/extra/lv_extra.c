/**
 * @file lv_extra.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../lvgl.h"

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

lv_img_decoder_t *lv_bmp_extra = NULL;
lv_img_decoder_t *lv_sjpeg_extra = NULL;
lv_img_decoder_t *lv_png_extra = NULL;

void lv_extra_init(void)
{
#if LV_USE_FLEX
    lv_flex_init();
#endif

#if LV_USE_GRID
    lv_grid_init();
#endif

#if LV_USE_MSG
    lv_msg_init();
#endif

#if LV_USE_FS_FATFS != '\0'
    lv_fs_fatfs_init();
#endif

#if LV_USE_FS_STDIO != '\0'
    lv_fs_stdio_init();
#endif

#if LV_USE_FS_POSIX != '\0'
    lv_fs_posix_init();
#endif

#if LV_USE_FS_WIN32 != '\0'
    lv_fs_win32_init();
#endif

#if LV_USE_FFMPEG
    lv_ffmpeg_init();
#endif

#if LV_USE_PNG
    lv_png_extra = lv_png_init();
#endif

#if LV_USE_SJPG
    lv_sjpeg_extra = lv_split_jpeg_init();
#endif

#if LV_USE_BMP
    lv_bmp_extra = lv_bmp_init();
#endif

#if LV_USE_FREETYPE
    /*Init freetype library*/
#  if LV_FREETYPE_CACHE_SIZE >= 0
    lv_freetype_init(LV_FREETYPE_CACHE_FT_FACES, LV_FREETYPE_CACHE_FT_SIZES, LV_FREETYPE_CACHE_SIZE);
#  else
    lv_freetype_init(0, 0, 0);
#  endif
#endif
}

void lv_extra_deinit(void) {
#if LV_USE_FREETYPE
    lv_freetype_destroy();
#endif

#if LV_USE_BMP
    if (lv_bmp_extra == NULL) {
        lv_img_decoder_delete(lv_bmp_extra);
        lv_bmp_extra = NULL;
    }
#endif

#if LV_USE_SJPG
    if (lv_bmp_extra == NULL) {
        lv_img_decoder_delete(lv_sjpeg_extra);
        lv_png_extra = NULL;
    }
#endif

#if LV_USE_PNG
    if (lv_png_extra == NULL) {
        lv_img_decoder_delete(lv_png_extra);
        lv_png_extra = NULL;
    }
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
