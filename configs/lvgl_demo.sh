PLAYER_CFG=${ext:-lv_demo}

export DEMO="lvgl_demo"
case $PLAYER_CFG in
    player)
    export LVGL_CFG=""
    export LV_DRV_CFG="-DLV_DRV_USE_WAYLAND=y"
    export DEMO_CFG="-DLV_USE_RK_PLAYER=y \
                     -DLV_DEMO_PLAYER_SMALL=y \
                     -DLV_DRV_USE_WAYLAND=y"
    ;;
    lv_demo)
    export LVGL_CFG="-DLV_USE_GPU_SDL=y \
                     -DLV_USE_DEMO_MUSIC=1"
    export LV_DRV_CFG="-DLV_DRV_USE_SDL_GPU=y"
    export DEMO_CFG="-DLV_USE_DEMO_MUSIC=y \
                     -DLV_DRV_USE_SDL_GPU=y"
    ;;
    rk_demo)
    export LVGL_CFG="-DLV_USE_GPU_SDL=y"
    export LV_DRV_CFG="-DLV_DRV_USE_SDL_GPU=y"
    export DEMO_CFG="-DLV_USE_RK_DEMO=y \
                     -DLV_DRV_USE_SDL_GPU=y"
    ;;
esac
