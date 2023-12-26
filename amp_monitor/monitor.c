#include <lvgl/lvgl.h>
#include <lvgl/lv_drivers/sdl/gl/gl.h>

#include "monitor.h"

lv_ft_info_t ttf_main;
lv_obj_t *scr;
lv_obj_t *chart;
lv_obj_t *title;
lv_obj_t *axis_title_x;
lv_obj_t *axis_title_y;
lv_obj_t *tag[2];
lv_obj_t *tag_title[2];
lv_obj_t *tag_avg[2];
lv_obj_t *tag_max[2];
lv_chart_series_t *ser[2];

static void event_handler(lv_event_t * e)
{
}

static void font_init(void)
{
    printf("%s\n", __func__);
    lv_freetype_init(64, 1, 0);

    ttf_main.weight = 60;
    ttf_main.name = "/data/SmileySans-Oblique.ttf";
    ttf_main.style = FT_FONT_STYLE_NORMAL;
    lv_ft_font_init(&ttf_main);
}

static lv_coord_t sample_x[] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
static lv_coord_t sample_y[] = {1000, 800, 300, 500, 200, 100, 50, 30, 20, 15, 25};
static lv_coord_t sample2_x[] = {0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200};
static lv_coord_t sample2_y[] = {1000, 800, 300, 500, 200, 100, 50, 30, 20, 15, 25};
void monitor(void)
{
    font_init();

    scr = lv_scr_act();
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_refr_size(scr);

    title = lv_label_create(scr);
    lv_label_set_text(title, "cyclictest with kernel-5.15.24-rt31");

    axis_title_x = lv_label_create(scr);
    lv_label_set_text(axis_title_x, "latency in us");

    axis_title_y = lv_label_create(scr);
    lv_label_set_text(axis_title_y, "occurrence");
    lv_obj_refr_size(axis_title_y);
    lv_obj_set_style_transform_angle(axis_title_y, 270, 0);
    lv_obj_set_style_transform_pivot_x(axis_title_y,
        lv_obj_get_width(axis_title_y) / 2, 0);
    lv_obj_set_style_transform_pivot_y(axis_title_y,
        lv_obj_get_height(axis_title_y) / 2, 0);

    for (int i = 0; i < 2; i++)
    {
        tag[i] = lv_obj_create(scr);
        lv_obj_set_size(tag[i], LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(tag[i], LV_FLEX_FLOW_COLUMN);
        tag_title[i] = lv_label_create(tag[i]);
        tag_avg[i] = lv_label_create(tag[i]);
        tag_max[i] = lv_label_create(tag[i]);
    }

    lv_obj_set_style_bg_color(tag[0],
        lv_palette_lighten(LV_PALETTE_PINK, 1),
        LV_PART_MAIN);
    lv_label_set_text(tag_title[0], "5.15.24-rt31");
    lv_label_set_text(tag_avg[0], "avg: 14us");
    lv_label_set_text(tag_max[0], "max: 94us");

    lv_obj_set_style_bg_color(tag[1],
        lv_palette_lighten(LV_PALETTE_BLUE, 1),
        LV_PART_MAIN);
    lv_label_set_text(tag_title[1], "5.15.24");
    lv_label_set_text(tag_avg[1], "avg: 28us");
    lv_label_set_text(tag_max[1], "max: 199us");

    chart = lv_chart_create(scr);
    lv_obj_set_size(chart, 700, 460);
    lv_chart_set_div_line_count(chart, 5, 4);
    lv_chart_set_type(chart, LV_CHART_TYPE_SCATTER);
    lv_chart_set_range(chart,
        LV_CHART_AXIS_PRIMARY_X, 0, 200);
    lv_chart_set_range(chart,
        LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    lv_chart_set_axis_tick(chart,
        LV_CHART_AXIS_PRIMARY_X, 10, 5, 5, 2, true, 100);
    lv_chart_set_axis_tick(chart,
        LV_CHART_AXIS_PRIMARY_Y, 10, 5, 3, 2, true, 100);

    lv_chart_set_point_count(chart,
        sizeof(sample_x) / sizeof(sample_x[0]));
    ser[0] = lv_chart_add_series(chart,
        lv_palette_lighten(LV_PALETTE_PINK, 1), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_ext_x_array(chart, ser[0], sample_x);
    lv_chart_set_ext_y_array(chart, ser[0], sample_y);
    ser[1] = lv_chart_add_series(chart,
        lv_palette_lighten(LV_PALETTE_BLUE, 1), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_ext_x_array(chart, ser[1], sample2_x);
    lv_chart_set_ext_y_array(chart, ser[1], sample2_y);

    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_align_to(chart, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_align_to(axis_title_x, chart, LV_ALIGN_OUT_BOTTOM_MID, 0, 50);
    lv_obj_align_to(axis_title_y, chart, LV_ALIGN_OUT_LEFT_MID, -20, 0);
    lv_obj_align_to(tag[0], chart, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_align_to(tag[1], tag[0], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
}

