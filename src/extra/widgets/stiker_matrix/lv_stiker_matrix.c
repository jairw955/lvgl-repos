/**
 * @file lv_stiker_matrix.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_stiker_matrix.h"
#if LV_USE_STIKER_MATRIX

#include "../../../misc/lv_assert.h"

/*********************
 *      DEFINES
 *********************/
#define ITEM_CLASS  &lv_stiker_matrix_item_class
#define MY_CLASS    &lv_stiker_matrix_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_stiker_matrix_item_class = {
    .base_class = &lv_obj_class,
    .width_def = 200,
    .height_def = 200,
    .instance_size = sizeof(lv_stiker_matrix_item_t)
};

const lv_obj_class_t lv_stiker_matrix_class = {
    .base_class = &lv_obj_class,
    .width_def = LV_DPI_DEF * 2,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(lv_stiker_matrix_t)
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_stiker_matrix_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");

    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    LV_ASSERT_MALLOC(obj);
    if(obj == NULL) return NULL;
    lv_obj_class_init_obj(obj);
    lv_stiker_matrix_t * stiker = (lv_stiker_matrix_t *)obj;

    stiker->fixed_width = 200;
    stiker->fixed_height = 200;
    lv_obj_set_size(obj, stiker->fixed_width, stiker->fixed_height);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_width(obj, 0, LV_PART_SCROLLBAR);

    return obj;
}

void lv_stiker_matrix_set_size(lv_obj_t * obj, lv_coord_t w, lv_coord_t h)
{
    lv_stiker_matrix_t * stiker = (lv_stiker_matrix_t *)obj;

    if ((w == LV_SIZE_CONTENT) && (h == LV_SIZE_CONTENT))
    {
        LV_LOG_ERROR("w and h cannot all be LV_SIZE_CONTENT");
        return;
    }
    lv_obj_set_size(obj, w, h);
    lv_obj_refr_size(obj);

    if (w != LV_SIZE_CONTENT)
        stiker->fixed_width = lv_obj_get_width(obj);
    else
        stiker->fixed_width = LV_SIZE_CONTENT;
    if (h != LV_SIZE_CONTENT)
        stiker->fixed_height = lv_obj_get_height(obj);
    else
        stiker->fixed_height = LV_SIZE_CONTENT;
}

static void lv_stiker_matrix_update_items(lv_stiker_matrix_t * obj, lv_coord_t max_w, lv_coord_t max_h)
{
    lv_stiker_matrix_item_t * item;
    uint32_t max;

    max = lv_obj_get_child_cnt(&obj->obj);
    for (int i = 0; i < max; i++)
    {
        item = (lv_stiker_matrix_item_t *)lv_obj_get_child(&obj->obj, i);
        lv_obj_set_size(&item->lv_obj,
                        (lv_coord_t)(max_w * item->pct_w / 100.0),
                        (lv_coord_t)(max_h * item->pct_h / 100.0));
    }
}

lv_obj_t * lv_stiker_matrix_item_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(ITEM_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

lv_obj_t * lv_stiker_matrix_new_item(lv_obj_t * obj, lv_coord_t pct_w, lv_coord_t pct_h)
{
    lv_stiker_matrix_t * stiker = (lv_stiker_matrix_t *)obj;
    lv_stiker_matrix_item_t * _item;
    lv_obj_t * item;
    lv_coord_t max_w;
    lv_coord_t max_h;
    lv_coord_t parent_w;
    lv_coord_t parent_h;

    if ((pct_w == 100) || ((pct_w + stiker->wfill > 100)))
    {
        stiker->cnt = 0;
        stiker->line++;
        stiker->wfill = pct_w;
    }
    else
    {
        stiker->wfill += pct_w;
    }

    item = lv_stiker_matrix_item_create(obj);
    _item = (lv_stiker_matrix_item_t *)item;
    _item->pct_w = pct_w;
    _item->pct_h = pct_h;
    stiker->items[stiker->cnt] = item;
    stiker->cnt++;

    parent_w = stiker->fixed_width == LV_SIZE_CONTENT ? stiker->fixed_height : stiker->fixed_width;
    parent_h = stiker->fixed_height == LV_SIZE_CONTENT ? stiker->fixed_width : stiker->fixed_height;

    max_w = parent_w
            - lv_obj_get_style_pad_left(obj, LV_PART_MAIN)
            - lv_obj_get_style_pad_right(obj, LV_PART_MAIN)
            - lv_obj_get_style_pad_row(obj, LV_PART_MAIN) * (stiker->cnt - 1)
            - lv_obj_get_style_width(item, LV_PART_SCROLLBAR);
    max_h = parent_h
            - lv_obj_get_style_pad_top(obj, LV_PART_MAIN)
            - lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN);
    if (stiker->fixed_height != LV_SIZE_CONTENT)
        max_h -= lv_obj_get_style_pad_column(obj, LV_PART_MAIN) * (stiker->line - 1);

    lv_stiker_matrix_update_items(stiker, max_w, max_h);

    return item;
}

#endif /*LV_USE_STIKER_MATRIX*/
