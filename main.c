/*
 * Copyright (c) 2021 Rockchip, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "main.h"
#include "lvgl/lv_drivers/wayland/wayland.h"

#define LVGL_TICK      5

static int g_ui_rotation = 0;
static int g_indev_rotation = 0;

static int quit = 0;

static void sigterm_handler(int sig) {
    fprintf(stderr, "signal %d\n", sig);
    quit = 1;
}

static void lvgl_init(void)
{
    lv_init();
    lv_port_disp_init(g_ui_rotation);
    lv_port_indev_init(g_indev_rotation);
}
extern void lv_demo_music(void);

int main(int argc, char **argv)
{
#define FPS     1
    uint32_t sst = 0;
    uint32_t st0, et0;
    float maxfps = 0.0, minfps = 1000.0;
    float fps;
    float fps0 = 0, fps1 = 0;
    uint32_t st, et;
    signal(SIGINT, sigterm_handler);
    lvgl_init();

    lv_demo_music();

    st0 = sst = clock_ms();
    while(!quit) {
        et = clock_ms();
        lv_tick_inc(et - sst);
        sst = et;

        st = clock_ms();
        lv_task_handler();
        et = clock_ms();
#if FPS
        fps = 1000 / (et - st);
        if (fps != 0.0 && fps < minfps) {
            minfps = fps;
            printf("Update minfps %f\n", minfps);
        }
        if (fps < 60 && fps > maxfps) {
            maxfps = fps;
            printf("Update maxfps %f\n", maxfps);
        }
        if (fps > 0.0 && fps < 60) {
            fps0 = (fps0 + fps) / 2;
            fps1 = (fps0 + fps1) / 2;
        }
        et0 = clock_ms();
        if ((et0 - st0) > 1000) {
            printf("avg:%f\n", fps1);
            st0 = et0;
        }
#endif

        usleep(5 * 1000);
    }

    return 0;
}
