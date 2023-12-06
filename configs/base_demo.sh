PLAYER_CFG=${ext:-lv_demo}

export DEMO="base_demo"
case $PLAYER_CFG in
    lv_demo)
    export LVGL_CFG="-DLV_USE_GPU_SDL=y \
                     -DLV_USE_DEMO_MUSIC=1"
    export LV_DRV_CFG="-DLV_DRV_USE_SDL_GPU=y"
    export DEMO_CFG="-DLV_USE_DEMO_MUSIC=y \
                     -DLV_DRV_USE_SDL_GPU=y"
    ;;
esac
