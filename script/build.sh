#!/bin/bash -i
#

FILE=$(realpath $0)
TARGET=$1
DIR=$(dirname $FILE)
ROOT_PATH=${ROOT_PATH:-$(dirname $DIR)}
BUILD_TYPE=${BUILD_TYPE:-Debug}
FLAGS="-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
HOOKS=$(find ${DIR} -type f -name '*.sh' ! -name 'build.sh')

# 切换到项目根目录
cd "$ROOT_PATH" || exit 1

# 加载 build helper 函数
if [ -f "$DIR/build-helper" ]; then
    source "$DIR/build-helper"
else
    echo "Error: build-helper not found"
    exit 1
fi

# 解析 target，提取 hook 名称和操作参数
# 例如: lvgl8-build -> hook=lvgl8.sh, action=build
#      lvgl8-reconfigure -> hook=lvgl8.sh, action=reconfigure
#      lvgl8 -> hook=lvgl8.sh, action=build (默认)
parse_target(){
    local target=$1
    local hook_name=""
    local action="build"  # 默认操作

    # 检查 target 是否包含 "-"
    if [[ "$target" == *"-"* ]]; then
        # 提取 hook 名称（第一个 "-" 之前的部分）
        hook_name="${target%%-*}"
        # 提取操作（第一个 "-" 之后的部分）
        action="${target#*-}"
    else
        # 没有 "-"，整个 target 就是 hook 名称
        hook_name="$target"
        # 使用默认操作 build
    fi

    echo "${hook_name}:${action}"
}

# 处理 menu 相关的目标
if [ "$TARGET" = "menu" ]; then
    # 直接打开 menuconfig
    if [ -f "$ROOT_PATH/configs/mconf" ]; then
        "$ROOT_PATH/configs/mconf" "$ROOT_PATH/configs/Kconfig"
    elif [ -f "$ROOT_PATH/configs/Config.in" ]; then
        # 如果没有 mconf，尝试使用其他方式
        echo "Error: mconf not found. Please build it first."
        exit 1
    else
        echo "Error: configs/Kconfig not found"
        exit 1
    fi
    exit 0
elif [ "$TARGET" = "menu8" ]; then
    # 拷贝 lvgl8_defconfig 到 .config，然后打开 menuconfig
    if [ -f "$ROOT_PATH/configs/lvgl8_defconfig" ]; then
        cp "$ROOT_PATH/configs/lvgl8_defconfig" "$ROOT_PATH/.config"
        echo "Copied configs/lvgl8_defconfig to .config"
    else
        echo "Error: configs/lvgl8_defconfig not found"
        exit 1
    fi
    if [ -f "$ROOT_PATH/configs/mconf" ]; then
        "$ROOT_PATH/configs/mconf" "$ROOT_PATH/configs/Kconfig"
    else
        echo "Error: mconf not found. Please build it first."
        exit 1
    fi
    exit 0
elif [ "$TARGET" = "menu9" ]; then
    # 拷贝 lvgl9_defconfig 到 .config，然后打开 menuconfig
    if [ -f "$ROOT_PATH/configs/lvgl9_defconfig" ]; then
        cp "$ROOT_PATH/configs/lvgl9_defconfig" "$ROOT_PATH/.config"
        echo "Copied configs/lvgl9_defconfig to .config"
    else
        echo "Error: configs/lvgl9_defconfig not found"
        exit 1
    fi
    if [ -f "$ROOT_PATH/configs/mconf" ]; then
        "$ROOT_PATH/configs/mconf" "$ROOT_PATH/configs/Kconfig"
    else
        echo "Error: mconf not found. Please build it first."
        exit 1
    fi
    exit 0
fi

# 执行对应的 hook
if [ -z "$TARGET" ]; then
    echo "Error: TARGET is required"
    exit 1
fi

# 解析 target
TARGET_INFO=$(parse_target "$TARGET")
HOOK_NAME="${TARGET_INFO%%:*}"
ACTION="${TARGET_INFO#*:}"

# 查找对应的 hook 文件
HOOK_FILE=""
for hook in ${HOOKS}; do
    hook_basename=$(basename "$hook" .sh)
    if [ "$hook_basename" = "$HOOK_NAME" ]; then
        HOOK_FILE="$hook"
        break
    fi
done

# 使用统一函数检查配置并添加编译选项
add_flag_if_config "BR2_PACKAGE_LVGL_DISABLE_THORVG" "-DCONFIG_LV_USE_THORVG_INTERNAL=n"
add_flag_if_config "BR2_PACKAGE_LVGL_DISABLE_EXAMPLES" "-DCONFIG_LV_BUILD_EXAMPLES=n"
add_flag_if_config "BR2_LV_USE_DEMO_WIDGETS" "-DLV_USE_DEMO_WIDGETS=1"
add_flag_if_config "BR2_LV_USE_DEMO_KEYPAD_AND_ENCODER" "-DLV_USE_DEMO_KEYPAD_AND_ENCODER=1"
add_flag_if_config "BR2_LV_USE_DEMO_BENCHMARK" "-DLV_USE_DEMO_BENCHMARK=1"
add_flag_if_config "BR2_LV_USE_DEMO_STRESS" "-DLV_USE_DEMO_STRESS=1"
add_flag_if_config "BR2_LV_USE_DEMO_MUSIC" "-DLV_USE_DEMO_MUSIC=1"
add_flag_if_config "BR2_LV_USE_GPU_SDL" "-DLV_USE_GPU_SDL=1 -DLV_DRV_USE_SDL_GPU=1"
add_flag_if_config "BR2_LV_USE_SDL" "-DLV_USE_GPU_SDL=1 -DLV_DRV_USE_SDL_GPU=1"
add_flag_if_config "BR2_LV_USE_LINUX_DRM" "-DLV_DRV_USE_DRM=1"
add_flag_if_config "BR2_LV_DRM_USE_RGA" "-DLV_DRV_USE_RGA=1"
add_flag_if_config "BR2_LV_DRM_USE_EGL" "-DLV_DRV_USE_OPENGLES=1"
add_flag_if_config "BR2_LV_USE_RKADK" "-DLV_DRV_USE_RKADK=1"

# 如果找到 hook 文件，则调用它并传入 action 参数
if [ -n "$HOOK_FILE" ] && [ -f "$HOOK_FILE" ]; then
    echo "Executing hook: $HOOK_FILE with action: $ACTION"
    # 导出 FLAGS 变量，以便 hook 脚本可以访问
    export FLAGS
    export BUILD_TYPE
    export ROOT_PATH=${ROOT_PATH:-${DIR}/..}
    bash "$HOOK_FILE" "$ACTION"
else
    echo "Error: Hook file for '$HOOK_NAME' not found"
    exit 1
fi
