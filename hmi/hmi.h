#ifndef __HMI_H__
#define __HMI_H__

#include <lvgl/lvgl.h>

#define ALIGN(x, a)     (((x) + (a - 1)) & ~(a - 1))
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))

#define MAJOR_FONT_COLOR    \
    lv_color_white()
#define MINOR_FONT_COLOR    \
    lv_palette_lighten(LV_PALETTE_GREY, 2)

#define MAJOR_FONT(x)   do  \
{   \
    lv_obj_set_style_text_font(x, ttf_main_m.font,  \
        LV_PART_MAIN);                              \
    lv_obj_set_style_text_color(x,                  \
        MAJOR_FONT_COLOR, LV_PART_MAIN);            \
} while(0);

#define MINOR_FONT(x)   do  \
{   \
    lv_obj_set_style_text_font(x, ttf_main.font,    \
        LV_PART_MAIN);                              \
    lv_obj_set_style_text_color(x,                  \
        MINOR_FONT_COLOR, LV_PART_MAIN);            \
} while(0);

#define MAJOR_FONT_S(x)   do  \
{   \
    lv_obj_set_style_text_font(x, ttf_main.font,    \
        LV_PART_MAIN);                              \
    lv_obj_set_style_text_color(x,                  \
        MAJOR_FONT_COLOR, LV_PART_MAIN);            \
} while(0);

typedef enum
{
    PAGE_HOME,
    PAGE_EP_SYS,
    PAGE_MAX,
} page_id;

extern lv_ft_info_t ttf_main;
extern lv_ft_info_t ttf_main_m;
extern lv_obj_t *scr;

#define PATH_PREFIX     "base_demo/hmi/res"
#define DEFAUL_FONT     PATH_PREFIX"/SmileySans-Oblique.ttf"
#define PIC             "A:"PATH_PREFIX"/pic"
#define ICON            "A:"PATH_PREFIX"/icon"
#define PIC_BG          PIC"/bg.png"

/* status bar icons */
#define ICON_5G_0       ICON"/status-bar-5G-0.png"
#define ICON_5G_1       ICON"/status-bar-5G-1.png"
#define ICON_5G_2       ICON"/status-bar-5G-2.png"
#define ICON_5G_3       ICON"/status-bar-5G-3.png"
#define ICON_5G_4       ICON"/status-bar-5G-4.png"
#define ICON_5G_FULL    ICON"/status-bar-5G-full.png"
#define ICON_BAT_CHARG  ICON"/status-bar-battery-charging.png"
#define ICON_BAT        ICON"/status-bar-battery.png"
#define ICON_HOME       ICON"/status-bar-home.png"
#define ICON_L          ICON"/status-bar-L.png"

/* home icons */
#define ICON_LOAD       ICON"/load.png"
#define ICON_RECT       ICON"/rectangle.png"
#define ICON_CTL_1      ICON"/control_round_1.png"
#define ICON_CTL_2      ICON"/control_round_2.png"
#define ICON_CTL_3      ICON"/control_round_3.png"
#define ICON_CTL_4      ICON"/control_round_4.png"
#define ICON_PATH_1     ICON"/remote_path_1.png"
#define ICON_PATH_2     ICON"/remote_path_2.png"
#define ICON_PATH_3     ICON"/remote_path_3.png"
#define ICON_PATH_4     ICON"/remote_path_4.png"

/* applications icons */
#define ICON_APP_1      ICON"/app1.png"
#define ICON_APP_2      ICON"/app2.png"
#define ICON_APP_3      ICON"/app3.png"
#define ICON_APP_4      ICON"/app4.png"
#define ICON_APP_5      ICON"/app5.png"
#define ICON_APP_6      ICON"/app6.png"
#define ICON_APP_7      ICON"/app7.png"
#define ICON_APP_8      ICON"/app8.png"
#define ICON_APP_9      ICON"/app9.png"
#define ICON_APP_10     ICON"/app10.png"

/* endpoint system icons */
#define EP_SYS_ITEM_1_1 ICON"/ep_sys_item_1_1.png"
#define EP_SYS_ITEM_1_2 ICON"/ep_sys_item_1_2.png"
#define EP_SYS_ITEM_1_3 ICON"/ep_sys_item_1_3.png"
#define EP_SYS_ITEM_1_4 ICON"/ep_sys_item_1_4.png"

#define EP_SYS_ITEM_2_1 ICON"/ep_sys_item_2_1.png"
#define EP_SYS_ITEM_2_2 ICON"/ep_sys_item_2_2.png"
#define EP_SYS_ITEM_2_3 ICON"/ep_sys_item_2_3.png"
#define EP_SYS_ITEM_2_4 ICON"/ep_sys_item_2_4.png"

#define EP_SYS_ITEM_3_1 ICON"/ep_sys_item_3_1.png"
#define EP_SYS_ITEM_3_2 ICON"/ep_sys_item_3_2.png"

#define EP_SYS_ITEM_4_1 ICON"/ep_sys_item_4_1.png"

#define EP_SYS_ITEM_5_1 ICON"/ep_sys_item_5_1.png"
#define EP_SYS_ITEM_5_2 ICON"/ep_sys_item_5_2.png"

#define EP_SYS_ITEM_6_1 ICON"/ep_sys_item_6_1.png"
#define EP_SYS_ITEM_6_2 ICON"/ep_sys_item_6_2.png"
#define EP_SYS_ITEM_6_3 ICON"/ep_sys_item_6_3.png"
#define EP_SYS_ITEM_6_4 ICON"/ep_sys_item_6_4.png"
#define EP_SYS_ITEM_6_5 ICON"/ep_sys_item_6_5.png"
#define EP_SYS_ITEM_6_6 ICON"/ep_sys_item_6_6.png"

void goto_page(page_id id);
void page_back(void);

#endif

