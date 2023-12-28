#include <lvgl/lvgl.h>

#include "hmi.h"

static const char *imgs[] =
{
    ICON_APP_1,
    ICON_APP_2,
    ICON_APP_3,
    ICON_APP_4,
    ICON_APP_5,
    ICON_APP_6,
    ICON_APP_7,
    ICON_APP_8,
    ICON_APP_9,
    ICON_APP_10
};

static const char *labels[] =
{
    "系统",
    "模组",
    "无线拨号",
    "数据中心",
    "抄表管理",
    "负荷管理",
    "资源管理",
    "交易支撑",
    "互动服务",
    "运维管理"
};

static void app1_cb(lv_event_t *e)
{
    goto_page(PAGE_EP_SYS);
}

lv_obj_t *applications_create(lv_obj_t *parent)
{
    lv_obj_t *main;
    lv_obj_t *cont;
    lv_obj_t *img;
    lv_obj_t *label;

    main = lv_obj_create(parent);
    lv_obj_remove_style_all(main);
    lv_obj_set_size(main, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_hor(main, 110, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(main, 22, LV_PART_MAIN);
    lv_obj_set_style_pad_row(main, 60, LV_PART_MAIN);
    lv_obj_set_style_pad_column(main, 110, LV_PART_MAIN);
    lv_obj_set_flex_flow(main, LV_FLEX_FLOW_ROW_WRAP);

    for (int i = 0; i < ARRAY_SIZE(imgs); i++)
    {
        cont = lv_obj_create(main);
        lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_style_all(cont);
        lv_obj_set_size(cont, 120, 160);
        img = lv_img_create(cont);
        lv_img_set_src(img, imgs[i]);
        lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);
        label = lv_label_create(cont);
        lv_label_set_text(label, labels[i]);
        MINOR_FONT(label);
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 0);
        if (i == 0)
        {
            lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(cont, app1_cb,
                LV_EVENT_CLICKED, NULL);
        }
    }
}

