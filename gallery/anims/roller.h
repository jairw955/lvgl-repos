#ifndef __ROLLER_H__
#define __ROLLER_H__

void anim_roller_start(lv_anim_t *a);
void anim_roller(void *var, int32_t v);
void anim_roller_end(lv_anim_t *a);

#define ANIM_ROLLER   {             \
    .time = 1000,                   \
    .start_value = 400,             \
    .current_value = 400,           \
    .end_value = 200,               \
    .repeat_cnt = 1,                \
    .start_cb = anim_roller_start,  \
    .path_cb = lv_anim_path_linear, \
    .exec_cb = anim_roller,         \
    .deleted_cb = anim_roller_end,  \
}

#endif

