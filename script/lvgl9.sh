#!/bin/bash -i
#

FILE=$(realpath $0)
DIR=$(dirname $FILE)
ROOT_PATH=${ROOT_PATH:-${DIR}/..}
LVGL_FLAGS=${FLAGS:-"-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"}
INSTALL_PATH=${INSTALL_PATH:-${ROOT_PATH}/lib/lvgl9}

# 加载 build helper 函数
if [ -f "$DIR/build-helper" ]; then
    source "$DIR/build-helper"
fi

# 检查版本配置，如果未设置则拷贝默认配置
check_lvgl9_config(){
    if ! check_config_enabled "BR2_PACKAGE_LVGL_VERSION_9"; then
        echo "BR2_PACKAGE_LVGL_VERSION_9 not set, copying configs/lvgl9.base to .config"
        if [ -f "$ROOT_PATH/configs/lvgl9.base" ]; then
            cp "$ROOT_PATH/configs/lvgl9.base" "$ROOT_PATH/.config"
        else
            echo "Error: configs/lvgl9.base not found"
            exit 1
        fi
    fi
}

lvgl9_configure(){
    check_lvgl9_config
    rm ${ROOT_PATH}/lvgl9/build -rf
    LVGL_FLAGS="${LVGL_FLAGS} -DBUILD_SHARED_LIBS=OFF -DCONFIG_LV_USE_PRIVATE_API=ON"
    ${ROOT_PATH}/configs/conf.sh ${ROOT_PATH}/.config ${ROOT_PATH}/lvgl9/lv_conf.mk
    cmake -S ${ROOT_PATH}/lvgl9 -B ${ROOT_PATH}/lvgl9/build ${LVGL_FLAGS} -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH}
}

lvgl9_build(){
    if [ ! -d ${ROOT_PATH}/lvgl9/build ]; then
        lvgl9_configure
    fi
    make -C ${ROOT_PATH}/lvgl9/build install -j
}

case $1 in
    "configure")
        lvgl9_configure
        ;;
    "build")
        lvgl9_build
        ;;
    *)
        echo "Usage: $0 <configure|build>"
        exit 1
    ;;
esac
