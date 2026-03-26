#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <lvgl/lvgl.h>
#include <lvgl/lv_conf.h>

#include "main.h"

static int quit = 0;

#ifndef LV_USE_DEMO_EEZ
#define LV_USE_DEMO_EEZ 0
#endif

#if LV_USE_DEMO_BENCHMARK
extern void lv_demo_benchmark(void);
#elif LV_USE_DEMO_WIDGETS
extern void lv_demo_widgets(void);
#elif LV_USE_DEMO_MUSIC
extern void lv_demo_music(void);
#elif LV_USE_DEMO_EEZ
extern void ui_init();
extern void ui_tick();
#endif

static void sigterm_handler(int sig)
{
    fprintf(stderr, "signal %d\n", sig);
    quit = 1;
}

static void usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s [options]\n"
            "  --width <px>     window width (PC simulator / SDL)\n"
            "  --height <px>    window height\n"
            "  --rotate <deg>   display rotation: 0, 90, 180, or 270\n"
            "  --fullscreen     borderless fullscreen (SDL)\n"
            "  -h, --help       show this help\n",
            prog);
}

static int parse_args(int argc, char **argv, int *width, int *height, int *rotate, int *fullscreen)
{
    *fullscreen = 0;
#ifdef PC_SIMULATOR
    *width = 800;
    *height = 480;
#else
    *width = 0;
    *height = 0;
#endif
    *rotate = 0;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--width") == 0)
        {
            if (++i >= argc)
            {
                fprintf(stderr, "error: --width needs a value\n");
                return -1;
            }
            *width = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "--height") == 0)
        {
            if (++i >= argc)
            {
                fprintf(stderr, "error: --height needs a value\n");
                return -1;
            }
            *height = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "--rotate") == 0)
        {
            if (++i >= argc)
            {
                fprintf(stderr, "error: --rotate needs a value\n");
                return -1;
            }
            *rotate = atoi(argv[i]);
            if (*rotate != 0 && *rotate != 90 && *rotate != 180 && *rotate != 270)
            {
                fprintf(stderr, "error: --rotate must be 0, 90, 180, or 270\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "--fullscreen") == 0)
        {
            *fullscreen = 1;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            usage(argv[0]);
            exit(0);
        }
        else
        {
            fprintf(stderr, "error: unknown option '%s'\n", argv[i]);
            usage(argv[0]);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    int width, height, rotation, fullscreen;

    signal(SIGINT, sigterm_handler);

    if (parse_args(argc, argv, &width, &height, &rotation, &fullscreen) != 0)
        return 1;

    lv_port_init(width, height, rotation, fullscreen);

#if LV_USE_DEMO_BENCHMARK
    lv_demo_benchmark();
#elif LV_USE_DEMO_WIDGETS
    lv_demo_widgets();
#elif LV_USE_DEMO_MUSIC
    lv_demo_music();
#elif LV_USE_DEMO_EEZ
    ui_init();
#endif

    while (!quit)
    {
        lv_task_handler();
        usleep(100);
#if LV_USE_DEMO_EEZ
        ui_tick();
#endif
    }

    return 0;
}
