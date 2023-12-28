#include <lvgl/lvgl.h>

#include "hmi.h"

static lv_obj_t *remote_singal_status(lv_obj_t *parent)
{
    static const char *imgs[] =
    {
        ICON_PATH_1,
        ICON_PATH_2,
        ICON_PATH_3,
        ICON_PATH_4
    };
    static const char *titles[] =
    {
        "第一路",
        "第二路",
        "第三路",
        "第四路"
    };
    static const char *labels[] =
    {
        "合",
        "分",
        "未接入",
        "分"
    };
    lv_color_t colors[4];
    lv_obj_t *main;
    lv_obj_t *obj;
    lv_obj_t *label;
    lv_obj_t *bottom;
    lv_obj_t *cont;
    lv_obj_t *img;
    lv_obj_t *line;

    colors[0] = lv_color_hex(0x8efa56);
    colors[1] = lv_color_hex(0xff2121);
    colors[2] = lv_color_hex(0xc9cdd4);
    colors[3] = lv_color_hex(0xff2121);

    main = parent;
    lv_obj_clear_flag(main, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(main, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(main, lv_color_white(),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(main,
        LV_OPA_10, LV_PART_MAIN);
    lv_obj_set_style_radius(main, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(main, 0, LV_PART_MAIN);

    obj = lv_obj_create(main);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 5, 17);
    lv_obj_set_pos(obj, 22, 18);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x54d0e8),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN);

    label = lv_label_create(main);
    lv_label_set_text(label, "遥信干接点当前状态");
    MINOR_FONT(label);
    lv_obj_align_to(label, obj,
        LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    bottom = lv_obj_create(main);
    lv_obj_remove_style_all(bottom);
    lv_obj_set_size(bottom, lv_pct(100), 252);
    lv_obj_set_style_pad_all(bottom, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(bottom, 8, LV_PART_MAIN);
    lv_obj_set_flex_flow(bottom, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_align(bottom, LV_ALIGN_BOTTOM_MID, 0, 0);

    static lv_point_t points[2];
    points[0].x = 0;
    points[0].y = lv_obj_get_y2(label) + 10;
    points[1].x = 72;
    points[1].y = lv_obj_get_y2(label) + 10;

    for (int i = 0; i < 4; i++)
    {
        cont = lv_obj_create(bottom);
        lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(cont, LV_OPA_10, LV_PART_MAIN);
        lv_obj_set_style_pad_all(cont, 14, LV_PART_MAIN);
        lv_obj_set_size(cont, 100, 236);

        label = lv_label_create(cont);
        lv_label_set_text(label, titles[i]);
        MINOR_FONT(label);
        lv_obj_align(label,
            LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_refr_pos(label);

        line = lv_line_create(cont);
        lv_obj_set_style_line_width(line, 2, LV_PART_MAIN);
        lv_obj_set_style_line_opa(line, LV_OPA_20,
            LV_PART_MAIN);
        lv_obj_set_style_line_color(line, lv_color_white(),
            LV_PART_MAIN);
        lv_obj_set_style_line_rounded(line,
            true, LV_PART_MAIN);
        lv_line_set_points(line, points, 2);

        label = lv_label_create(cont);
        lv_label_set_text(label, labels[i]);
        MINOR_FONT(label);
        lv_obj_set_style_text_color(label,
            colors[i], LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 0);

        img = lv_img_create(cont);
        lv_img_set_src(img, imgs[i]);
        lv_obj_align_to(img, label,
            LV_ALIGN_OUT_TOP_MID, 0, -20);
    }

    return main;
}

static lv_obj_t *control_status(lv_obj_t *parent)
{
    const char *imgs[] =
    {
        ICON_CTL_1,
        ICON_CTL_2,
        ICON_CTL_3,
        ICON_CTL_4
    };
    const char *titles[] =
    {
        "第一轮",
        "第二轮",
        "第三轮",
        "第四轮"
    };
    const char *labels[] =
    {
        "跳闸",
        "合闸",
        "05:59",
        "未投入"
    };
    lv_color_t colors[4];
    lv_obj_t *main;
    lv_obj_t *obj;
    lv_obj_t *label;
    lv_obj_t *bottom;
    lv_obj_t *cont;
    lv_obj_t *img;
    lv_obj_t *line;

    colors[0] = lv_color_hex(0xff2121);
    colors[1] = lv_color_hex(0x8efa56);
    colors[2] = lv_color_hex(0xffc975);
    colors[3] = lv_color_hex(0xe2e2e2);

    main = parent;
    lv_obj_clear_flag(main, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(main, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(main, lv_color_white(),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(main,
        LV_OPA_10, LV_PART_MAIN);
    lv_obj_set_style_radius(main, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(main, 0, LV_PART_MAIN);

    obj = lv_obj_create(main);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 5, 17);
    lv_obj_set_pos(obj, 22, 18);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x54d0e8),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN);

    label = lv_label_create(main);
    lv_label_set_text(label, "控制轮次当前状态");
    MINOR_FONT(label);
    lv_obj_align_to(label, obj,
        LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    bottom = lv_obj_create(main);
    lv_obj_remove_style_all(bottom);
    lv_obj_set_size(bottom, lv_pct(100), 252);
    lv_obj_set_style_pad_all(bottom, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(bottom, 8, LV_PART_MAIN);
    lv_obj_set_flex_flow(bottom, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_align(bottom, LV_ALIGN_BOTTOM_MID, 0, 0);

    static lv_point_t points[2];
    points[0].x = 0;
    points[0].y = lv_obj_get_y2(label) + 10;
    points[1].x = 72;
    points[1].y = lv_obj_get_y2(label) + 10;

    for (int i = 0; i < 4; i++)
    {
        cont = lv_obj_create(bottom);
        lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(cont, LV_OPA_10, LV_PART_MAIN);
        lv_obj_set_style_pad_all(cont, 14, LV_PART_MAIN);
        lv_obj_set_size(cont, 100, 236);

        label = lv_label_create(cont);
        lv_label_set_text(label, titles[i]);
        MINOR_FONT(label);
        lv_obj_align(label,
            LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_refr_pos(label);

        line = lv_line_create(cont);
        lv_obj_set_style_line_width(line, 2, LV_PART_MAIN);
        lv_obj_set_style_line_opa(line, LV_OPA_20,
            LV_PART_MAIN);
        lv_obj_set_style_line_color(line, lv_color_white(),
            LV_PART_MAIN);
        lv_obj_set_style_line_rounded(line,
            true, LV_PART_MAIN);
        lv_line_set_points(line, points, 2);

        label = lv_label_create(cont);
        lv_label_set_text(label, labels[i]);
        MINOR_FONT(label);
        lv_obj_set_style_text_color(label,
            colors[i], LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 0);

        img = lv_img_create(cont);
        lv_img_set_src(img, imgs[i]);
        lv_obj_align_to(img, label,
            LV_ALIGN_OUT_TOP_MID, 0, -20);
    }

    return main;
}

static lv_obj_t *load_trend(lv_obj_t *parent)
{
    static lv_coord_t sample_x[] =
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    static lv_coord_t sample_y[] =
        {700, 800, 650, 640, 630, 620, 720,
         720, 800, 860, 810, 790, 870};
    static lv_coord_t sample2_y[] =
        {520, 520, 520, 520, 520, 520, 520,
         520, 520, 520, 520, 520, 520};
    lv_obj_t *main;
    lv_obj_t *obj;
    lv_obj_t *label;
    lv_obj_t *chart;
    lv_chart_series_t *ser[2];

    main = parent;
    lv_obj_clear_flag(main, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(main, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(main, lv_color_white(),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(main,
        LV_OPA_10, LV_PART_MAIN);
    lv_obj_set_style_radius(main, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(main, 0, LV_PART_MAIN);

    obj = lv_obj_create(main);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 5, 17);
    lv_obj_set_pos(obj, 22, 18);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x54d0e8),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN);

    label = lv_label_create(main);
    lv_label_set_text(label, "负荷趋势");
    MINOR_FONT(label);
    lv_obj_align_to(label, obj,
        LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    label = lv_label_create(main);
    lv_obj_set_pos(obj, 22, 18);
    lv_label_set_text(label, "今日负荷");
    MINOR_FONT(label);
    lv_obj_align(label,
        LV_ALIGN_TOP_RIGHT, -20, 18);

    obj = lv_obj_create(main);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 20, 10);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x8cfaa6),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN);
    lv_obj_align_to(obj, label,
        LV_ALIGN_OUT_LEFT_MID, -10, 0);

    label = lv_label_create(main);
    lv_label_set_text(label, "功率定值");
    MINOR_FONT(label);
    lv_obj_align_to(label, obj,
        LV_ALIGN_OUT_LEFT_MID, -30, 0);

    obj = lv_obj_create(main);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 20, 10);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff6262),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN);
    lv_obj_align_to(obj, label,
        LV_ALIGN_OUT_LEFT_MID, -10, 0);

    chart = lv_chart_create(main);
    lv_obj_set_size(chart, 760, 230);
    lv_obj_set_style_bg_opa(chart, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(chart, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(chart, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_color(chart,
        lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_line_color(chart,
        lv_color_white(), LV_PART_TICKS);
    lv_obj_set_style_text_color(chart,
        MINOR_FONT_COLOR, LV_PART_TICKS);
    lv_chart_set_div_line_count(chart, 6, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_range(chart,
        LV_CHART_AXIS_PRIMARY_X, 0, 24);
    lv_chart_set_range(chart,
        LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    lv_chart_set_axis_tick(chart,
        LV_CHART_AXIS_PRIMARY_X, 0, 0, 9, 1, true, 80);
    lv_chart_set_axis_tick(chart,
        LV_CHART_AXIS_PRIMARY_Y, 0, 0, 6, 1, true, 20);

    lv_chart_set_point_count(chart, ARRAY_SIZE(sample_x));
    ser[0] = lv_chart_add_series(chart,
        lv_color_hex(0x8cfaa6), LV_CHART_AXIS_PRIMARY_Y);
//    lv_chart_set_ext_x_array(chart, ser[0], sample_x);
    lv_chart_set_ext_y_array(chart, ser[0], sample_y);
    ser[1] = lv_chart_add_series(chart,
        lv_color_hex(0xff6262), LV_CHART_AXIS_PRIMARY_Y);
//    lv_chart_set_ext_x_array(chart, ser[1], sample_x);
    lv_chart_set_ext_y_array(chart, ser[1], sample2_y);
    lv_obj_align(chart, LV_ALIGN_BOTTOM_MID, 30, -30);

    return main;
}

static lv_obj_t *safety_load(lv_obj_t *parent)
{
    static lv_grad_dsc_t grad;
    lv_obj_t *main;
    lv_obj_t *label;
    lv_obj_t *label2;

    grad.dir = LV_GRAD_DIR_VER;
    grad.stops_count = 2;
    grad.stops[0].color = lv_color_hex(0x2970a5);
    grad.stops[1].color = lv_color_hex(0x4a79d4);
    grad.stops[0].frac  = 1;
    grad.stops[1].frac  = 255;

    main = parent;
    lv_obj_clear_flag(main, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(main, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(main,
        grad.stops[0].color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(main,
        LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_grad(main, &grad, LV_PART_MAIN);
    lv_obj_set_style_radius(main, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(main, 30, LV_PART_MAIN);

    label = lv_label_create(main);
    lv_label_set_text(label, "保安电荷");
    MINOR_FONT(label);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    label = lv_label_create(main);
    lv_label_set_text(label, "kW");
    MINOR_FONT(label);
    lv_obj_align(label, LV_ALIGN_RIGHT_MID, 0, 0);

    label2 = lv_label_create(main);
    lv_label_set_text(label2, "10000");
    MAJOR_FONT(label2);
    lv_obj_align_to(label2, label,
        LV_ALIGN_OUT_LEFT_MID, -6, 0);

    return main;
}

static lv_obj_t *status(lv_obj_t *parent)
{
    static lv_grad_dsc_t grad;
    lv_obj_t *main;
    lv_obj_t *label;

    grad.dir = LV_GRAD_DIR_VER;
    grad.stops_count = 2;
    grad.stops[0].color = lv_color_hex(0x4e559e);
    grad.stops[1].color = lv_color_hex(0x3761ab);
    grad.stops[0].frac  = 1;
    grad.stops[1].frac  = 255;

    main = parent;
    lv_obj_clear_flag(main, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(main, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(main,
        grad.stops[0].color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(main,
        LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_grad(main, &grad, LV_PART_MAIN);
    lv_obj_set_style_radius(main, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(main, 30, LV_PART_MAIN);

    label = lv_label_create(main);
    lv_label_set_text(label, "保电中");
    MINOR_FONT(label);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    label = lv_label_create(main);
    lv_label_set_text(label, "08:13:30");
    MAJOR_FONT(label);
    lv_obj_align(label, LV_ALIGN_RIGHT_MID, 0, 0);

    return main;
}

static lv_obj_t *add_item(lv_obj_t *parent,
    const char *name, int val)
{
    lv_obj_t *cont;
    lv_obj_t *label;
    lv_obj_t *label2;

    cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, lv_pct(100), 45);

    label = lv_label_create(cont);
    lv_label_set_text(label, name);
    MINOR_FONT(label);
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    label = lv_label_create(cont);
    lv_label_set_text(label, "kWh");
    MINOR_FONT(label);
    lv_obj_align(label, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    label2 = lv_label_create(cont);
    lv_label_set_text_fmt(label2, "%d", val);
    MAJOR_FONT(label2);
    lv_obj_align_to(label2, label,
        LV_ALIGN_OUT_LEFT_BOTTOM, -6, 0);

    return cont;
}

static lv_obj_t *group(lv_obj_t *parent)
{
    lv_obj_t *main;
    lv_obj_t *obj;
    lv_obj_t *cont;
    lv_obj_t *line;
    lv_obj_t *label;
    lv_obj_t *label2;

    main = lv_obj_create(parent);
    lv_obj_remove_style_all(main);
    lv_obj_set_size(main, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(main, lv_color_white(),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(main, LV_OPA_10, LV_PART_MAIN);
    lv_obj_set_style_radius(main, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(main, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(main, 15, LV_PART_MAIN);

    obj = lv_obj_create(main);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 5, 17);
    lv_obj_set_pos(obj, 22, 18);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x54d0e8),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN);

    label = lv_label_create(main);
    lv_label_set_text(label, "总加组1");
    MINOR_FONT(label);
    lv_obj_align_to(label, obj,
        LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    obj = lv_img_create(main);
    lv_img_set_src(obj, ICON_LOAD);
    lv_obj_set_pos(obj, 30, 65);

    label = lv_label_create(main);
    lv_label_set_text(label, "300009.9");
    MAJOR_FONT(label);
    lv_obj_align_to(label, obj,
        LV_ALIGN_OUT_RIGHT_TOP, 26, 0);

    label2 = lv_label_create(main);
    lv_label_set_text(label2, "kW");
    MINOR_FONT(label2);
    lv_obj_align_to(label2, label,
        LV_ALIGN_OUT_RIGHT_BOTTOM, 6, 0);

    label = lv_label_create(main);
    lv_label_set_text(label, "当前负荷");
    MINOR_FONT(label);
    lv_obj_align_to(label, obj,
        LV_ALIGN_OUT_RIGHT_BOTTOM, 26, 0);

    obj = lv_obj_create(main);
    lv_obj_remove_style_all(obj);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(obj, lv_pct(100), lv_pct(78));
    lv_obj_set_style_bg_color(obj, lv_color_white(),
        LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 18, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_refr_size(obj);
    lv_obj_refr_pos(obj);

    cont = lv_obj_create(obj);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, lv_pct(100), 45);

    label = lv_label_create(cont);
    lv_label_set_text(label, "购电控投入");
    MAJOR_FONT(label);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    cont = add_item(obj, "剩余电量", -100);
    cont = add_item(obj, "报警电量", 150);
    cont = add_item(obj, "跳闸电量", 1);

    label = lv_label_create(obj);
    lv_obj_add_flag(label, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_label_set_text(label, "厂休控投入、厂休控投入");
    MAJOR_FONT(label);
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_refr_size(label);
    lv_obj_refr_pos(label);

    static lv_point_t points[2];
    points[0].x = 0;
    points[0].y = lv_obj_get_y(label) - 5;
    points[1].x = lv_obj_get_width(obj) - 36;
    points[1].y = lv_obj_get_y(label) - 5;
    line = lv_line_create(obj);
    lv_obj_add_flag(line, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_style_line_width(line, 2, LV_PART_MAIN);
    lv_obj_set_style_line_opa(line, LV_OPA_20,
        LV_PART_MAIN);
    lv_obj_set_style_line_color(line, lv_color_white(),
        LV_PART_MAIN);
    lv_obj_set_style_line_rounded(line, true, LV_PART_MAIN);
    lv_line_set_points(line, points, 2);

    return main;
}

lv_obj_t *load_management_create(lv_obj_t *parent)
{
    static lv_coord_t col_dsc0[] =
        {350, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc0[] =
        {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t col_dsc1[] =
        {440, 440, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc1[] =
        {60, 310, 310, LV_GRID_TEMPLATE_LAST};
    lv_obj_t *main;
    lv_obj_t *submain;
    lv_obj_t *obj;
    lv_obj_t *left;
    lv_obj_t *right;
    lv_obj_t *label;

    main = lv_obj_create(parent);
    lv_obj_remove_style_all(main);
    lv_obj_set_size(main, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_hor(main, 15, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(main, 14, LV_PART_MAIN);

    label = lv_label_create(main);
    lv_label_set_text(label, "负荷管理");
    MAJOR_FONT(label);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    submain = lv_obj_create(main);
    lv_obj_remove_style_all(submain);
    lv_obj_set_size(submain, lv_pct(100), lv_pct(94));
    lv_obj_set_style_pad_gap(submain, 10, LV_PART_MAIN);
    lv_obj_set_style_grid_column_dsc_array(submain,
        col_dsc0, 0);
    lv_obj_set_style_grid_row_dsc_array(submain,
        row_dsc0, 0);
    lv_obj_set_layout(submain, LV_LAYOUT_GRID);
    lv_obj_align(submain, LV_ALIGN_BOTTOM_MID, 0, 0);

    left = lv_obj_create(submain);
    lv_obj_remove_style_all(left);
    lv_obj_set_grid_cell(left, LV_GRID_ALIGN_STRETCH, 0, 1,
                               LV_GRID_ALIGN_STRETCH, 0, 1);

    obj = group(left);

    right = lv_obj_create(submain);
    lv_obj_remove_style_all(right);
    lv_obj_set_grid_cell(right, LV_GRID_ALIGN_STRETCH, 1, 1,
                                LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_style_pad_gap(right, 10, LV_PART_MAIN);
    lv_obj_set_style_grid_column_dsc_array(right,
        col_dsc1, 0);
    lv_obj_set_style_grid_row_dsc_array(right,
        row_dsc1, 0);
    lv_obj_set_layout(right, LV_LAYOUT_GRID);

    obj = lv_obj_create(right);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                              LV_GRID_ALIGN_STRETCH, 0, 1);
    status(obj);

    obj = lv_obj_create(right);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1,
                              LV_GRID_ALIGN_STRETCH, 0, 1);
    safety_load(obj);

    obj = lv_obj_create(right);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 2,
                              LV_GRID_ALIGN_STRETCH, 1, 1);
    load_trend(obj);

    obj = lv_obj_create(right);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                              LV_GRID_ALIGN_STRETCH, 2, 1);
    control_status(obj);

    obj = lv_obj_create(right);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1,
                              LV_GRID_ALIGN_STRETCH, 2, 1);
    remote_singal_status(obj);

    return main;
}
