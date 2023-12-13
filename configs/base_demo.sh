DEMO_SEL=${ext:-lv_demo}

export DEMO="base_demo"
case $DEMO_SEL in
    gallery)
    export LVGL_CFG="-DLV_USE_GPU_SDL=y"
    export LV_DRV_CFG="-DLV_DRV_USE_SDL_GPU=y \
                       -DLV_DRV_USE_OPENGL=y \
                       -DLV_DRV_SDL_DIS_FULLSCREEN=y"
    export DEMO_CFG="-DLV_USE_GALLERY=y \
                     -DLV_DRV_USE_SDL_GPU=y \
                     -DLV_DRV_USE_OPENGL=y"
    ;;
    lv_demo)
    export LVGL_CFG="-DLV_USE_GPU_SDL=y \
                     -DLV_USE_DEMO_WIDGETS=1 \
                     -DLV_USE_DEMO_KEYPAD_AND_ENCODER=1 \
                     -DLV_USE_DEMO_BENCHMARK=1 \
                     -DLV_USE_DEMO_STRESS=1 \
                     -DLV_USE_DEMO_MUSIC=1"
    export LV_DRV_CFG="-DLV_DRV_USE_SDL_GPU=y \
                       -DLV_DRV_SDL_DIS_FULLSCREEN=y"
    export DEMO_CFG="-DLV_USE_DEMO_MUSIC=y \
                     -DLV_DRV_USE_SDL_GPU=y"
    ;;
esac
