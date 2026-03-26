#include <lvgl/lvgl.h>
#include <lvgl/lv_conf.h>

#include "main.h"

static int quit = 0;

static void sigterm_handler(int sig)
{
    fprintf(stderr, "signal %d\n", sig);
    quit = 1;
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigterm_handler);
#ifdef PC_SIMULATOR
    lv_port_init(800, 480, 0, 0);
#else
    lv_port_init(0, 0, 0, 0);
#endif

    extern void init_fruit_ninja(void);
    init_fruit_ninja();

    while (!quit)
    {
        lv_task_handler();
        usleep(100);
    }

    return 0;
}
