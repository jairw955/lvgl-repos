#include <lvgl/lvgl.h>

#include "hmi.h"

#include "applcations.h"
#include "endpoint_system.h"
#include "load_management.h"

typedef struct lv_page {
    lv_obj_t *parent;
    lv_obj_t *obj;
    struct lv_page *prev;
} lv_page_t;

lv_ft_info_t ttf_main;
lv_ft_info_t ttf_main_m;
lv_obj_t *scr;
lv_obj_t *top;
lv_obj_t *img_bg;
lv_obj_t *status_bar;
lv_obj_t *icon_signal;
lv_obj_t *icon_bat;
lv_obj_t *icon_home;
lv_obj_t *icon_l;
lv_obj_t *home;
lv_obj_t *rectangle;
lv_obj_t *circle[2];
lv_obj_t *ep_sys;

lv_page_t pages[PAGE_MAX];
lv_page_t *page_cur;

typedef enum {
    SIGNAL_STRENGTH_0,
    SIGNAL_STRENGTH_1,
    SIGNAL_STRENGTH_2,
    SIGNAL_STRENGTH_3,
    SIGNAL_STRENGTH_4,
    SIGNAL_STRENGTH_FULL,
} signal_strength;

typedef enum {
    BATTERY_STATUS_NORMAL,
    BATTERY_STATUS_CHARGING,
} battery_status;

static const char *icons_signal[] =
{
    ICON_5G_0,
    ICON_5G_1,
    ICON_5G_2,
    ICON_5G_3,
    ICON_5G_4,
    ICON_5G_FULL
};

static const char *icons_bat[] =
{
    ICON_BAT,
    ICON_BAT_CHARG,
};

static inline void ui_signal_update(lv_obj_t *obj,
    signal_strength strength)
{
    lv_img_set_src(obj, icons_signal[strength]);
}

static inline void ui_battery_update(lv_obj_t *obj,
    battery_status status)
{
    lv_img_set_src(obj, icons_bat[status]);
}

static void font_init(void)
{
    lv_freetype_init(64, 1, 0);

    ttf_main.weight = 20;
    ttf_main.name = DEFAUL_FONT;
    ttf_main.style = FT_FONT_STYLE_NORMAL;
    lv_ft_font_init(&ttf_main);

    ttf_main_m.weight = 26;
    ttf_main_m.name = DEFAUL_FONT;
    ttf_main_m.style = FT_FONT_STYLE_NORMAL;
    lv_ft_font_init(&ttf_main_m);
}

static void scroll_cb(lv_event_t *event)
{
    lv_obj_t *home = lv_event_get_target(event);
    lv_coord_t val = lv_obj_get_scroll_left(home);
    if (val == 0)
    {
        lv_obj_set_style_bg_opa(circle[0],
            LV_OPA_100, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(circle[1],
            LV_OPA_20, LV_PART_MAIN);
    }
    if (val == lv_obj_get_width(home))
    {
        lv_obj_set_style_bg_opa(circle[0],
            LV_OPA_20, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(circle[1],
            LV_OPA_100, LV_PART_MAIN);
    }
}

void goto_page(page_id id)
{
    if (id > PAGE_MAX)
        return;

    pages[id].prev = page_cur;
    lv_obj_add_flag(page_cur->obj, LV_OBJ_FLAG_HIDDEN);
    page_cur = &pages[id];
    lv_obj_clear_flag(page_cur->obj, LV_OBJ_FLAG_HIDDEN);
}

void page_back(void)
{
    if (!page_cur->prev)
        return;

    lv_obj_add_flag(page_cur->obj, LV_OBJ_FLAG_HIDDEN);
    page_cur = page_cur->prev;
    lv_obj_clear_flag(page_cur->obj, LV_OBJ_FLAG_HIDDEN);
}

static void page_home_init(void)
{
    lv_obj_t *tileview;
    lv_obj_t *obj;

    home = lv_obj_create(scr);
    lv_obj_add_flag(home, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_style_all(home);
    lv_obj_set_size(home, lv_pct(100),
        lv_obj_get_height(scr) - 30);
    lv_obj_align(home, LV_ALIGN_BOTTOM_MID, 0, 0);

    tileview = lv_tileview_create(home);
    lv_obj_remove_style_all(tileview);
    lv_obj_set_size(tileview, lv_pct(100), lv_pct(100));
    lv_obj_align(tileview, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(tileview, scroll_cb,
        LV_EVENT_SCROLL, NULL);
    obj = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR);
    load_management_create(obj);
    obj = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR);
    applications_create(obj);

    pages[PAGE_HOME].parent = scr;
    pages[PAGE_HOME].obj = home;
    pages[PAGE_HOME].prev = NULL;

    rectangle = lv_img_create(home);
    lv_img_set_src(rectangle, ICON_RECT);
    lv_obj_align(rectangle, LV_ALIGN_BOTTOM_MID, 0, 0);

    circle[0] = lv_obj_create(rectangle);
    lv_obj_remove_style_all(circle[0]);
    lv_obj_set_style_bg_opa(circle[0],
        LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_radius(circle[0], 5, LV_PART_MAIN);
    lv_obj_set_size(circle[0], 10, 10);
    lv_obj_align(circle[0], LV_ALIGN_BOTTOM_MID, -10, -10);

    circle[1] = lv_obj_create(rectangle);
    lv_obj_remove_style_all(circle[1]);
    lv_obj_set_style_bg_opa(circle[1],
        LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_radius(circle[1], 5, LV_PART_MAIN);
    lv_obj_set_size(circle[1], 10, 10);
    lv_obj_align(circle[1], LV_ALIGN_BOTTOM_MID, 10, -10);
}

static void page_ep_sys_init(void)
{
    ep_sys = lv_obj_create(scr);
    lv_obj_add_flag(ep_sys, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_style_all(ep_sys);
    lv_obj_set_size(ep_sys, lv_pct(100),
        lv_obj_get_height(scr) - 30);
    lv_obj_align(ep_sys, LV_ALIGN_BOTTOM_MID, 0, 0);
    pages[PAGE_EP_SYS].obj = ep_sys;
    pages[PAGE_EP_SYS].parent = scr;
    pages[PAGE_EP_SYS].prev = NULL;

    endpoint_system_create(ep_sys);
}

void ui_main(void)
{
    font_init();

    scr = lv_scr_act();
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_refr_size(scr);

    top = lv_layer_top();
    lv_obj_clear_flag(top, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_refr_size(top);

    status_bar = lv_obj_create(top);
    lv_obj_remove_style_all(status_bar);
    lv_obj_set_size(status_bar, lv_pct(100), 30);
    lv_obj_set_style_bg_color(status_bar, lv_color_black(),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_20,
        LV_PART_MAIN);
    lv_obj_set_flex_flow(status_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_bar, LV_FLEX_ALIGN_END,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(status_bar, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(status_bar, 16, LV_PART_MAIN);

    icon_signal = lv_img_create(status_bar);
    ui_signal_update(icon_signal, SIGNAL_STRENGTH_FULL);

    icon_l = lv_img_create(status_bar);
    lv_img_set_src(icon_l, ICON_L);

    icon_home = lv_img_create(status_bar);
    lv_img_set_src(icon_home, ICON_HOME);

    icon_bat = lv_img_create(status_bar);
    ui_battery_update(icon_bat, BATTERY_STATUS_NORMAL);

    img_bg = lv_img_create(scr);
    lv_img_set_src(img_bg, PIC_BG);
    lv_obj_center(img_bg);

    page_home_init();

    page_ep_sys_init();

    page_cur = &pages[PAGE_HOME];
    lv_obj_clear_flag(page_cur->obj, LV_OBJ_FLAG_HIDDEN);
}

