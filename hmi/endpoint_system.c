#include <lvgl/lvgl.h>

#include "hmi.h"

typedef struct
{
    lv_obj_t *obj;
    lv_obj_t *ctx;
    void (*cb)(lv_event_t *);
    char *img;
    char *text;
    char *minor_text;
    int long_cell;
} ep_sys_item;

typedef struct
{
    ep_sys_item *item;
    int items;
} ep_sys_group;

static void ethernet_cb(lv_event_t *e);
static void call_control_cb(lv_event_t *e);

static ep_sys_item items_1[] =
{
    {.img = EP_SYS_ITEM_1_1, .text = "终端参数",
        .minor_text = "终端地址\n34234565657566"},
    {.img = EP_SYS_ITEM_1_2, .text = "以太网",
        .cb = ethernet_cb},
    {.img = EP_SYS_ITEM_1_3, .text = "无线网"},
    {.img = EP_SYS_ITEM_1_4, .text = "无线电台"}
};

static ep_sys_item items_2[] =
{
    {.img = EP_SYS_ITEM_2_1, .text = "测量点参数"},
    {.img = EP_SYS_ITEM_2_2, .text = "脉冲参数"},
    {.img = EP_SYS_ITEM_2_3, .text = "总加组参数"},
    {.img = EP_SYS_ITEM_2_4, .text = "交采参数"}
};

static ep_sys_item items_3[] =
{
    {.img = EP_SYS_ITEM_3_1, .text = "召测控制参数",
        .cb = call_control_cb},
    {.img = EP_SYS_ITEM_3_2, .text = "模块控制"}
};

static ep_sys_item items_4[] =
{
    {.img = EP_SYS_ITEM_4_1, .text = "模组信息参数"}
};

static ep_sys_item items_5[] =
{
    {.img = EP_SYS_ITEM_5_1, .text = "测量点数据"},
    {.img = EP_SYS_ITEM_5_2, .text = "脉冲数据"}
};

static ep_sys_item items_6[] =
{
    {.img = EP_SYS_ITEM_6_1, .text = "菜单项6.1"},
    {.img = EP_SYS_ITEM_6_2, .text = "菜单项6.2"},
    {.img = EP_SYS_ITEM_6_3, .text = "菜单项6.3"},
    {.img = EP_SYS_ITEM_6_4, .text = "菜单项6.4"},
    {.img = EP_SYS_ITEM_6_5, .text = "菜单项6.5"},
    {.img = EP_SYS_ITEM_6_6, .text = "菜单项6.6"}
};

static ep_sys_item ethernet_items[] =
{
    {.text = "工作模式",
     .minor_text = "客户机模式"},
    {.text = "心跳周期(s)",
     .minor_text = "60"},
    {.text = "超时时间(s)",
     .minor_text = "30"},
    {.text = "重发次数",
     .minor_text = "3"},
    {.text = "连接方式",
     .minor_text = "TCP方式"},
    {.text = "连接应用方式",
     .minor_text = "主备模式"},
    {.text = "侦听端口",
     .minor_text = "6000", .long_cell = 1},
    {.text = "代理IP",
     .minor_text = "0.0.0.0"},
    {.text = "代理端口",
     .minor_text = "0"},
    {.text = "主用IP",
     .minor_text = "192.168.2.206"},
    {.text = "主用端口",
     .minor_text = "2233"},
    {.text = "备用IP",
     .minor_text = "192.168.2.206"},
    {.text = "备用端口",
     .minor_text = "2233"},
    {.text = "IP配置方式",
     .minor_text = "静态"},
    {.text = "终端IP",
     .minor_text = "192.168.2.180"},
};

static const ep_sys_group groups[] =
{
    {items_1, ARRAY_SIZE(items_1)},
    {items_2, ARRAY_SIZE(items_2)},
    {items_3, ARRAY_SIZE(items_3)},
    {items_4, ARRAY_SIZE(items_4)},
    {items_5, ARRAY_SIZE(items_5)},
    {items_6, ARRAY_SIZE(items_6)}
};

static lv_obj_t *right;
static lv_obj_t *cur = NULL;

static void title_cb(lv_event_t *e)
{
    page_back();
}

static void ethernet_fill_items(lv_obj_t *cont,
    ep_sys_item *item, int items)
{
    lv_obj_t *box;
    lv_obj_t *text;
    lv_obj_t *text_minor;
    lv_obj_t *line;
    lv_coord_t cont_width;
    lv_coord_t box_width;
    lv_coord_t left_pad;
    lv_coord_t right_pad;

    lv_obj_refr_size(cont);
    cont_width = lv_obj_get_width(cont);
    left_pad = lv_obj_get_style_pad_left(cont, LV_PART_MAIN);
    right_pad = lv_obj_get_style_pad_right(cont,
        LV_PART_MAIN);
    box_width = cont_width - left_pad - right_pad;

    for (int i = 0; i < items; i++)
    {
        box = lv_obj_create(cont);
        lv_obj_remove_style_all(box);
        lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_width(box, 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(box, lv_color_white(),
            LV_PART_MAIN);
        lv_obj_set_style_border_opa(box, LV_OPA_10,
            LV_PART_MAIN);
        lv_obj_set_style_border_side(box,
            LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
        if (item[i].long_cell)
            lv_obj_set_size(box, box_width, 77);
        else
            lv_obj_set_size(box, box_width / 2, 77);
        if (item[i].cb)
        {
            lv_obj_add_flag(box, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(box, item[i].cb,
                LV_EVENT_CLICKED, &item[i]);
        }

        text = lv_label_create(box);
        lv_label_set_text(text, item[i].text);
        MAJOR_FONT_S(text);
        lv_obj_align(text,
            LV_ALIGN_LEFT_MID, 24, -10);

        text_minor = lv_label_create(box);
        lv_label_set_text(text_minor,
            item[i].minor_text);
        MINOR_FONT(text_minor);
        lv_obj_align_to(text_minor, text,
            LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

        text_minor = lv_label_create(box);
        lv_label_set_text(text_minor, ">");
        MINOR_FONT(text_minor);
        lv_obj_set_style_text_align(text_minor,
            LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
        lv_obj_align(text_minor,
            LV_ALIGN_RIGHT_MID, 0, 0);
    }
}

static lv_obj_t *ethernet_create(lv_obj_t *parent)
{
    lv_obj_t *main;
    lv_obj_t *title;
    lv_obj_t *title_text;
    lv_obj_t *title_btn;
    lv_obj_t *title_btn_text;
    lv_obj_t *cont;

    main = lv_obj_create(parent);
    lv_obj_remove_style_all(main);
    lv_obj_set_size(main, lv_pct(100), lv_pct(100));
    lv_obj_center(main);
    lv_obj_refr_size(main);

    title = lv_obj_create(main);
    lv_obj_remove_style_all(title);
    lv_obj_set_size(title, lv_pct(100), lv_pct(11));
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    title_text = lv_label_create(title);
    lv_label_set_text(title_text, "通信参数");
    MINOR_FONT(title_text);
    lv_obj_align(title_text, LV_ALIGN_LEFT_MID, 0, 0);

    title_btn = lv_btn_create(title);
    lv_obj_set_size(title_btn, 80, 44);
    lv_obj_align(title_btn, LV_ALIGN_RIGHT_MID, -3, 0);

    title_btn_text = lv_label_create(title_btn);
    lv_label_set_text(title_btn_text, "保存");
    MINOR_FONT(title_btn_text);
    lv_obj_center(title_btn_text);

    cont = lv_obj_create(main);
    lv_obj_set_style_bg_opa(cont, LV_OPA_30,
        LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(cont, 0, LV_PART_MAIN);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(89));
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);

    ethernet_fill_items(cont, ethernet_items,
        ARRAY_SIZE(ethernet_items));

    return main;
}

static void ethernet_cb(lv_event_t *e)
{
    ep_sys_item *item = lv_event_get_user_data(e);
    if (item->ctx)
    {
        lv_obj_add_flag(cur, LV_OBJ_FLAG_HIDDEN);
        cur = item->ctx;
        lv_obj_clear_flag(cur, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    item->ctx = ethernet_create(right);
    if (cur)
        lv_obj_add_flag(cur, LV_OBJ_FLAG_HIDDEN);
    cur = item->ctx;
}

static void call_control_power(lv_obj_t *parent)
{
    lv_obj_t *cont;
    lv_obj_t *label[2];

    cont = lv_obj_create(parent);
    lv_obj_set_size(cont, lv_pct(100), 72);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(cont, 20, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 20, LV_PART_MAIN);

    label[0] = lv_label_create(cont);
    lv_label_set_text(label[0], "时段功率定值");
    MINOR_FONT(label[0]);
    lv_obj_align(label[0], LV_ALIGN_LEFT_MID, 0, 0);

    label[1] = lv_label_create(cont);
    lv_label_set_text(label[1], ">");
    MINOR_FONT(label[1]);
    lv_obj_align(label[1], LV_ALIGN_RIGHT_MID, 0, 0);
}

static void call_control_safety(lv_obj_t *parent)
{
    lv_obj_t *cont;
    lv_obj_t *part[2];
    lv_obj_t *label[4];

    cont = lv_obj_create(parent);
    lv_obj_set_size(cont, lv_pct(100), 88);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 20, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 20, LV_PART_MAIN);

    part[0] = lv_obj_create(cont);
    lv_obj_remove_style_all(part[0]);
    lv_obj_set_size(part[0], lv_pct(50), lv_pct(100));
    lv_obj_align(part[0], LV_ALIGN_LEFT_MID, 0, 0);

    label[0] = lv_label_create(part[0]);
    lv_label_set_text(label[0], "终端保安定值(kW)");
    MAJOR_FONT_S(label[0]);
    lv_obj_align(label[0], LV_ALIGN_TOP_LEFT, 0, 0);

    label[1] = lv_label_create(part[0]);
    lv_label_set_text(label[1], "0.6");
    MINOR_FONT(label[1]);
    lv_obj_align(label[1], LV_ALIGN_BOTTOM_LEFT, 0, 0);

    part[1] = lv_obj_create(cont);
    lv_obj_remove_style_all(part[1]);
    lv_obj_set_size(part[1], lv_pct(50), lv_pct(100));
    lv_obj_align(part[1], LV_ALIGN_RIGHT_MID, 0, 0);

    label[2] = lv_label_create(part[1]);
    lv_label_set_text(label[2], "工控定值浮动系数(%)");
    MAJOR_FONT_S(label[2]);
    lv_obj_align(label[2], LV_ALIGN_TOP_LEFT, 0, 0);

    label[3] = lv_label_create(part[1]);
    lv_label_set_text(label[3], "-");
    MINOR_FONT(label[3]);
    lv_obj_align(label[3], LV_ALIGN_BOTTOM_LEFT, 0, 0);
}

static void call_control_draw_timeline(lv_obj_t *parent,
    int rs, int vs, int ve, lv_color_t color)
{
    static lv_coord_t col_dsc[] =
        {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
         LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
         LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
         LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
         LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
         LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] =
        {LV_GRID_FR(1), 28, LV_GRID_TEMPLATE_LAST};
    lv_obj_t *obj;
    lv_obj_t *tag;
    lv_obj_t *text;

    lv_obj_set_style_pad_column(parent, 2, LV_PART_MAIN);
    lv_obj_set_style_grid_column_dsc_array(parent,
        col_dsc, LV_PART_MAIN);
    lv_obj_set_style_grid_row_dsc_array(parent,
        row_dsc, LV_PART_MAIN);
    lv_obj_set_layout(parent, LV_LAYOUT_GRID);
    lv_obj_refr_size(parent);
    lv_obj_refr_pos(parent);

    tag = lv_obj_create(parent);
    lv_obj_remove_style_all(tag);
    lv_obj_set_grid_cell(tag,
        LV_GRID_ALIGN_STRETCH, 0, 16,
        LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_style_bg_opa(tag,
        LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(tag,
        lv_color_hex(0xd8d8d8), LV_PART_MAIN);

    for (int i = 0; i < 16; i++)
    {
        obj = lv_obj_create(parent);
        lv_obj_remove_style_all(obj);
        lv_obj_set_grid_cell(obj,
            LV_GRID_ALIGN_STRETCH, i, 1,
            LV_GRID_ALIGN_STRETCH, 0, 1);
        lv_obj_set_style_bg_opa(obj,
            LV_OPA_100, LV_PART_MAIN);
        if (i >= vs && i < ve)
        {
            lv_obj_set_style_bg_color(obj,
                color, LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_bg_color(obj,
                lv_color_hex(0xf6c344), LV_PART_MAIN);
        }
        if (i % 2 == 0)
        {
            text = lv_label_create(parent);
            lv_label_set_text_fmt(text, "%02d", i / 2 + rs);
            lv_obj_set_grid_cell(text,
                LV_GRID_ALIGN_STRETCH, i, 1,
                LV_GRID_ALIGN_CENTER, 1, 1);
        }
    }
}

static void call_control_fill_timeline(lv_obj_t *parent)
{
    static lv_coord_t col_dsc[] =
        {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] =
        {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
         LV_GRID_TEMPLATE_LAST};
    lv_obj_t *timelines[3];

    lv_obj_set_style_grid_column_dsc_array(parent,
        col_dsc, LV_PART_MAIN);
    lv_obj_set_style_grid_row_dsc_array(parent,
        row_dsc, LV_PART_MAIN);
    lv_obj_set_layout(parent, LV_LAYOUT_GRID);

    for (int i = 0; i < ARRAY_SIZE(timelines); i++)
    {
        timelines[i] = lv_obj_create(parent);
        lv_obj_remove_style_all(timelines[i]);
        lv_obj_set_grid_cell(timelines[i],
            LV_GRID_ALIGN_STRETCH, 0, 1,
            LV_GRID_ALIGN_STRETCH, i, 1);
    }
    call_control_draw_timeline(timelines[0],
        0, 7, 11, lv_color_hex(0xe2592f));
    call_control_draw_timeline(timelines[1],
        8, 10, 14, lv_color_hex(0x67ad5b));
    call_control_draw_timeline(timelines[2],
        16, 12, 16, lv_color_hex(0x4988fd));
}

static void call_control_timeline(lv_obj_t *parent)
{
    static lv_coord_t col_dsc[] =
        {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
         LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] =
        {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    char *tag_name[4] = {"不控", "控1", "控2", "保留"};
    lv_color_t tag_color[4];
    lv_obj_t *cont;
    lv_obj_t *title;
    lv_obj_t *timeline;
    lv_obj_t *tag;
    lv_obj_t *tags[4];
    lv_obj_t *squre;
    lv_obj_t *label;

    tag_color[0] = lv_color_hex(0xf6c344);
    tag_color[1] = lv_color_hex(0xe2592f);
    tag_color[2] = lv_color_hex(0x67ad5b);
    tag_color[3] = lv_color_hex(0x4988fd);

    cont = lv_obj_create(parent);
    lv_obj_set_size(cont, lv_pct(100), 450);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 20, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 20, LV_PART_MAIN);

    title = lv_label_create(cont);
    lv_label_set_text(title, "总加组1功控时段");
    MAJOR_FONT_S(title);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    timeline = lv_obj_create(cont);
    lv_obj_remove_style_all(timeline);
    lv_obj_set_style_pad_row(timeline, 22, LV_PART_MAIN);
    lv_obj_set_size(timeline, lv_pct(100), 310);
    lv_obj_center(timeline);
    call_control_fill_timeline(timeline);

    tag = lv_obj_create(cont);
    lv_obj_remove_style_all(tag);
    lv_obj_set_size(tag, lv_pct(100), 50);
    lv_obj_align(tag, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_pad_gap(tag, 55, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(tag, 160, LV_PART_MAIN);
    lv_obj_set_style_grid_column_dsc_array(tag,
        col_dsc, LV_PART_MAIN);
    lv_obj_set_style_grid_row_dsc_array(tag,
        row_dsc, LV_PART_MAIN);
    lv_obj_set_layout(tag, LV_LAYOUT_GRID);

    for (int i = 0; i < ARRAY_SIZE(tag_name); i++)
    {
        tags[i] = lv_obj_create(tag);
        lv_obj_remove_style_all(tags[i]);
        lv_obj_set_grid_cell(tags[i],
            LV_GRID_ALIGN_STRETCH, i, 1,
            LV_GRID_ALIGN_STRETCH, 0, 1);
        squre = lv_obj_create(tags[i]);
        lv_obj_remove_style_all(squre);
        lv_obj_set_size(squre, 20, 20);
        lv_obj_set_style_bg_color(squre, tag_color[i],
            LV_PART_MAIN);
        lv_obj_set_style_bg_opa(squre, LV_OPA_100,
            LV_PART_MAIN);
        lv_obj_align(squre, LV_ALIGN_LEFT_MID, 0, 0);
        label = lv_label_create(tags[i]);
        lv_label_set_text(label, tag_name[i]);
        MINOR_FONT(label);
        lv_obj_align(label, LV_ALIGN_RIGHT_MID, 0, 0);
    }
}

static void call_control_tags(lv_obj_t *parent)
{
    static lv_coord_t col_dsc[] =
        {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
         LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
         LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] =
        {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_t *cont;
    lv_obj_t *tag[7];
    lv_obj_t *tag_label[7];
    char *tag_name[] = {
        "时段控", "厂休控", "报停控", "月电控",
        "购电控", "轮次", "下浮控"
    };

    cont = lv_obj_create(parent);
    lv_obj_set_size(cont, lv_pct(100), 44);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 22, LV_PART_MAIN);
    lv_obj_set_style_grid_column_dsc_array(cont,
        col_dsc, LV_PART_MAIN);
    lv_obj_set_style_grid_row_dsc_array(cont,
        row_dsc, LV_PART_MAIN);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    for (int i = 0; i < ARRAY_SIZE(tag_name); i++)
    {
        tag[i] = lv_obj_create(cont);
        lv_obj_remove_style_all(tag[i]);
        lv_obj_set_grid_cell(tag[i],
            LV_GRID_ALIGN_STRETCH, i, 1,
            LV_GRID_ALIGN_STRETCH, 0, 1);
        tag_label[i] = lv_label_create(tag[i]);
        lv_label_set_text(tag_label[i], tag_name[i]);
        MINOR_FONT(tag_label[i]);
        lv_obj_center(tag_label[i]);
    }

    MAJOR_FONT_S(tag_label[0]);
    lv_obj_set_style_radius(tag[0], 22, LV_PART_MAIN);
    lv_obj_set_style_bg_color(tag[0], lv_color_hex(0x5c84cd),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tag[0], LV_OPA_100,
        LV_PART_MAIN);
}

static lv_obj_t *call_control_create(lv_obj_t *parent)
{
    lv_obj_t *main;
    lv_obj_t *title;
    lv_obj_t *title_text;
    lv_obj_t *title_btn[3];
    lv_obj_t *title_btn_text[3];
    lv_obj_t *cont;

    main = lv_obj_create(parent);
    lv_obj_remove_style_all(main);
    lv_obj_set_size(main, lv_pct(100), lv_pct(100));
    lv_obj_center(main);
    lv_obj_refr_size(main);

    title = lv_obj_create(main);
    lv_obj_remove_style_all(title);
    lv_obj_set_size(title, lv_pct(100), lv_pct(11));
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    title_text = lv_label_create(title);
    lv_label_set_text(title_text, "召测控制参数");
    MINOR_FONT(title_text);
    lv_obj_align(title_text, LV_ALIGN_LEFT_MID, 0, 0);

    title_btn[0] = lv_btn_create(title);
    lv_obj_set_size(title_btn[0], 32, 32);
    lv_obj_set_style_radius(title_btn[0], 4, LV_PART_MAIN);
    lv_obj_align(title_btn[0], LV_ALIGN_RIGHT_MID, -3, 0);

    title_btn_text[0] = lv_label_create(title_btn[0]);
    lv_label_set_text(title_btn_text[0], ">");
    MINOR_FONT(title_btn_text[0]);
    lv_obj_center(title_btn_text[0]);

    title_btn[1] = lv_btn_create(title);
    lv_obj_set_size(title_btn[1], 124, 32);
    lv_obj_set_style_radius(title_btn[1], 4, LV_PART_MAIN);
    lv_obj_align_to(title_btn[1], title_btn[0],
        LV_ALIGN_OUT_LEFT_MID, -8, 0);

    title_btn_text[1] = lv_label_create(title_btn[1]);
    lv_label_set_text(title_btn_text[1], "1/8 总加组");
    MINOR_FONT(title_btn_text[1]);
    lv_obj_center(title_btn_text[1]);

    title_btn[2] = lv_btn_create(title);
    lv_obj_set_size(title_btn[2], 32, 32);
    lv_obj_set_style_radius(title_btn[2], 4, LV_PART_MAIN);
    lv_obj_align_to(title_btn[2], title_btn[1],
        LV_ALIGN_OUT_LEFT_MID, -8, 0);

    title_btn_text[2] = lv_label_create(title_btn[2]);
    lv_label_set_text(title_btn_text[2], "<");
    MINOR_FONT(title_btn_text[2]);
    lv_obj_center(title_btn_text[2]);

    cont = lv_obj_create(main);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(89));
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_pad_gap(cont, 20, LV_PART_MAIN);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);

    call_control_tags(cont);
    call_control_timeline(cont);
    call_control_safety(cont);
    call_control_power(cont);

    return main;
}

static void call_control_cb(lv_event_t *e)
{
    ep_sys_item *item = lv_event_get_user_data(e);
    if (item->ctx)
    {
        lv_obj_add_flag(cur, LV_OBJ_FLAG_HIDDEN);
        cur = item->ctx;
        lv_obj_clear_flag(cur, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    item->ctx = call_control_create(right);
    if (cur)
        lv_obj_add_flag(cur, LV_OBJ_FLAG_HIDDEN);
    cur = item->ctx;
}

static void fill_items(lv_obj_t *cont,
    ep_sys_item *item, int items)
{
    static lv_point_t points[2];
    lv_obj_t *box;
    lv_obj_t *img;
    lv_obj_t *text;
    lv_obj_t *text_minor;
    lv_obj_t *line;

    lv_obj_set_size(cont, lv_pct(100), 77 * items);
    for (int i = 0; i < items; i++)
    {
        box = lv_obj_create(cont);
        lv_obj_remove_style_all(box);
        lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(box, lv_pct(100), 77);
        if (item[i].cb)
        {
            lv_obj_add_flag(box, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(box, item[i].cb,
                LV_EVENT_CLICKED, &item[i]);
        }

        img = lv_img_create(box);
        lv_img_set_src(img, item[i].img);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0);

        text = lv_label_create(box);
        lv_label_set_text(text, item[i].text);
        MAJOR_FONT(text);
        lv_obj_align_to(text, img,
            LV_ALIGN_OUT_RIGHT_MID, 24, 0);

        if (item[i].minor_text)
        {
            text_minor = lv_label_create(box);
            lv_label_set_text(text_minor,
                item[i].minor_text);
            MINOR_FONT(text_minor);
            lv_obj_set_style_text_align(text_minor,
                LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
            lv_obj_align(text_minor,
                LV_ALIGN_RIGHT_MID, 0, 0);
        }

        if (i == (items - 1))
            continue;
        lv_obj_refr_pos(box);
        lv_obj_refr_pos(text);
        points[0].x = lv_obj_get_x(text);
        points[0].y = 75;
        points[1].x = lv_obj_get_x2(box);
        points[1].y = 75;
        line = lv_line_create(box);
        lv_obj_set_style_line_width(line, 2, LV_PART_MAIN);
        lv_obj_set_style_line_opa(line, LV_OPA_10,
            LV_PART_MAIN);
        lv_obj_set_style_line_color(line, lv_color_white(),
            LV_PART_MAIN);
        lv_obj_set_style_line_rounded(line,
            true, LV_PART_MAIN);
        lv_line_set_points(line, points, 2);
    }
}

lv_obj_t *endpoint_system_create(lv_obj_t *parent)
{
    lv_obj_t *title;
    lv_obj_t *time;
    lv_obj_t *title_left;
    lv_obj_t *left;
    lv_obj_t *cont;

    title_left = lv_obj_create(parent);
    lv_obj_remove_style_all(title_left);
    lv_obj_set_style_pad_hor(title_left, 15, LV_PART_MAIN);
    lv_obj_set_style_pad_row(title_left, 15, LV_PART_MAIN);
    lv_obj_set_size(title_left, lv_pct(40), lv_pct(11));
    lv_obj_align(title_left, LV_ALIGN_TOP_LEFT, 0, 0);

    left = lv_obj_create(parent);
    lv_obj_remove_style_all(left);
    lv_obj_set_style_pad_hor(left, 15, LV_PART_MAIN);
    lv_obj_set_style_pad_row(left, 15, LV_PART_MAIN);
    lv_obj_set_size(left, lv_pct(40), lv_pct(89));
    lv_obj_align(left, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);

    cont = lv_obj_create(title_left);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, lv_pct(100), 55);
    title = lv_label_create(cont);
    lv_label_set_text(title, "< 终端配置");
    MINOR_FONT(title);
    lv_obj_align(title, LV_ALIGN_BOTTOM_LEFT, 12, 0);
    lv_obj_add_flag(title, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(title, title_cb,
        LV_EVENT_CLICKED, NULL);
    time = lv_label_create(cont);
    lv_label_set_text(time, "10:15:30");
    MINOR_FONT(time);
    lv_obj_align(time, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    for (int i = 0; i < ARRAY_SIZE(groups); i++)
    {
        cont = lv_obj_create(left);
        lv_obj_remove_style_all(cont);
        lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_opa(cont, LV_OPA_30,
            LV_PART_MAIN);
        lv_obj_set_style_radius(cont, 12, LV_PART_MAIN);
        lv_obj_set_style_pad_hor(cont, 30, LV_PART_MAIN);
        lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
        fill_items(cont, groups[i].item, groups[i].items);
    }

    right = lv_obj_create(parent);
    lv_obj_remove_style_all(right);
    lv_obj_set_style_pad_hor(right, 15, LV_PART_MAIN);
    lv_obj_set_size(right, lv_pct(60), lv_pct(100));
    lv_obj_align(right, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}

