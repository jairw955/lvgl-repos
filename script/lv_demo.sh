#!/bin/bash -i
#

FILE=$(realpath $0)
DIR=$(dirname $FILE)
ROOT_PATH=${ROOT_PATH:-${DIR}/..}
FLAGS=${FLAGS:-"-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"}
INSTALL_PATH=${INSTALL_PATH:-${ROOT_PATH}/install}

# 加载 build helper 函数
if [ -f "$DIR/build-helper" ]; then
    source "$DIR/build-helper"
fi

FLAGS="${FLAGS} -DLV_DEMO=y -DPC_SIMULATOR=y"
add_flag_if_config "BR2_PACKAGE_LVGL_VERSION_8" "-DLVGL_V8=1"
add_flag_if_config "BR2_PACKAGE_LVGL_VERSION_9" "-DLVGL_V9=1"

configure(){
    rm ${ROOT_PATH}/app/build -rf
    cmake -S ${ROOT_PATH}/app -B ${ROOT_PATH}/app/build ${FLAGS} -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH}
}

build(){
    if [ ! -d ${ROOT_PATH}/app/build ]; then
        configure
    fi
    make -C ${ROOT_PATH}/app/build install -j
}

case $1 in
    "configure")
        configure
        ;;
    "build")
        build
        ;;
    *)
        echo "Usage: $0 <configure|build>"
        exit 1
    ;;
esac