#!/bin/bash -i
#

FILE=$(realpath $0)
DIR=$(dirname $FILE)
ROOT_PATH=${ROOT_PATH:-${DIR}/..}
INSTALL_PATH=${INSTALL_PATH:-${ROOT_PATH}/lib/lvgl8}

# 加载 build helper 函数
if [ -f "$DIR/build-helper" ]; then
    source "$DIR/build-helper"
fi

LVGL_FLAGS=${FLAGS:-"-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"}
append_host_arch_cmake_flags LVGL_FLAGS || exit 1

# 检查版本配置，如果未设置则拷贝默认配置
check_lvgl8_config(){
    if ! check_config_enabled "BR2_PACKAGE_LVGL_VERSION_8"; then
        echo "BR2_PACKAGE_LVGL_VERSION_8 not set, copying configs/lvgl8.base to .config"
        if [ -f "$ROOT_PATH/configs/lvgl8.base" ]; then
            cp "$ROOT_PATH/configs/lvgl8.base" "$ROOT_PATH/.config"
        else
            echo "Error: configs/lvgl8.base not found"
            exit 1
        fi
    fi
}

lvgl8_configure(){
    check_lvgl8_config
    rm ${ROOT_PATH}/lvgl8/lvgl/build -rf
    LVGL_FLAGS="${LVGL_FLAGS} -DBUILD_SHARED_LIBS=ON"
    ${ROOT_PATH}/configs/conf.sh ${ROOT_PATH}/.config ${ROOT_PATH}/lvgl8/lvgl/lv_conf.mk
    cmake -S ${ROOT_PATH}/lvgl8/lvgl -B ${ROOT_PATH}/lvgl8/lvgl/build ${LVGL_FLAGS} -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH}

    rm ${ROOT_PATH}/lvgl8/lv_drivers/build -rf
    cmake -S ${ROOT_PATH}/lvgl8/lv_drivers -B ${ROOT_PATH}/lvgl8/lv_drivers/build ${LVGL_FLAGS} -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH}
}

lvgl8_build(){
    if [ ! -d ${ROOT_PATH}/lvgl8/lvgl/build ]; then
        lvgl8_configure
    fi
    make -C ${ROOT_PATH}/lvgl8/lvgl/build install -j

    if [ ! -d ${ROOT_PATH}/lvgl8/lv_drivers/build ]; then
        lvgl8_configure
    fi
    make -C ${ROOT_PATH}/lvgl8/lv_drivers/build install -j
}

case $1 in
    "configure")
        lvgl8_configure
        ;;
    "build")
        lvgl8_build
        ;;
    *)
        echo "Usage: $0 <configure|build>"
        exit 1
    ;;
esac
