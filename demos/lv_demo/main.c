#include <lvgl/lvgl.h>
#include <lvgl/lv_conf.h>

#include "main.h"

static int quit = 0;

#ifndef USE_DEMO_EEZ
#define USE_DEMO_EEZ 0
#endif

#if USE_DEMO_WIDGETS
extern void lv_demo_widgets(void);
#elif USE_DEMO_BENCHMARK
extern void lv_demo_benchmark(void);
#elif USE_DEMO_MUSIC
extern void lv_demo_music(void);
#elif USE_DEMO_EEZ
extern void ui_init();
extern void ui_tick();
#endif

static void sigterm_handler(int sig)
{
    fprintf(stderr, "signal %d\n", sig);
    quit = 1;
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigterm_handler);
#ifdef PC_SIMULATOR
    lv_port_init(800, 480, 0);
#else
    lv_port_init(0, 0, 0);
#endif

#if USE_DEMO_WIDGETS
    lv_demo_widgets();
#elif USE_DEMO_BENCHMARK
    lv_demo_benchmark();
#elif USE_DEMO_MUSIC
    lv_demo_music();
#elif USE_DEMO_EEZ
    ui_init();
#endif

    while (!quit)
    {
        lv_task_handler();
        usleep(100);
#if USE_DEMO_EEZ
        ui_tick();
#endif
    }

    return 0;
}
