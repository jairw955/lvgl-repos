#!/bin/bash -i
#

demo=
configure=false
debug=false
build=true
install=false
lvgl_version=lvgl8
lvgl_path=lvgl8/lvgl
lv_drivers_path=lvgl8/lv_drivers
ext=

if [ -f .version ];then
    source .version
fi

function help(){
    echo -e "$1 <target> <options>"
    echo -e "  target\tlvgl|lv_drv|demos"
    echo -e "  -a\t\tall, -c -b -i"
    echo -e "  -b\t\tbuild(true by default)"
    echo -e "  -c\t\tconfigure(false by default)"
    echo -e "  -d\t\tdebug, cmake build type"
    echo -e "  -e\t\tenvironment, lvgl and lv_drivers"
    echo -e "  -h\t\tthis help"
    echo -e "  -i\t\tinstall(false by default)"
    echo -e "  -s\t\tdemo select"
    echo -e "  -8\t\tlvgl8"
    echo -e "  -9\t\tlvgl9"
    echo -e "Usage:"
    echo -e "$1 lvgl_demo -c -b"
    echo -e "$1 lvgl_demo -cbi"
}

function build_target(){
    make -C $1/build -j$(nproc)
}

function install_target(){
    sudo make -C $1/build install
}

function reconfigure_target(){
    rm $1/build -rf
    mkdir $1/build -p
    cmake -S $1 -B $1/build ${@:1}
}

function parse_args(){
    len=${#1}
    for ((i=1;i<len;i++));do
        case ${1:i:1} in
        "a")
            configure=true
            build=true
            install=true
            ;;
        "b")
            build=true
            ;;
        "c")
            configure=true
            ;;
        "d")
            debug=true
            ;;
        "i")
            install=true
            ;;
        "8")
            lvgl_version=lvgl8
            ;;
        "9")
            lvgl_version=lvgl9
            ;;
        esac
    done
}

while [ ! -z "$1" ]
do
    case $1 in
    "-a")
        configure=true
        build=true
        install=true
        shift 1
        ;;
    "-b")
        build=true
        shift 1
        ;;
    "-c")
        configure=true
        shift 1
        ;;
    "-d")
        debug=true
        shift 1
        ;;
    "-h")
        help $0
        exit
        ;;
    "-i")
        install=true
        shift 1
        ;;
    "-s")
        ext=$2
        shift 2
        ;;
    "-8")
        lvgl_version=lvgl8
        shift 1
        ;;
    "-9")
        lvgl_version=lvgl9
        shift 1
        ;;
    *)
        if [ "${1:0:1}" == "-" ];then
            parse_args $1
        else
            demo=$1
        fi
        shift 1
        ;;
    esac
done

if [ "$demo" == "" ];then
    echo "nothing to compile"
    help $0
    exit
fi

if [ "$lvgl_version" == "lvgl9" ];then
    if [ "$demo" == "lv_drivers" ];then
        echo "lv_drivers already merge to lvgl9"
        exit
    fi
    lvgl_path=lvgl9
fi

cp ./configs/${lvgl_version}.base .config

if [ "$demo" == "menuconfig" ];then
    ./configs/mconf ./configs/Kconfig.${lvgl_version}
    echo "lvgl_version=${lvgl_version}" > .version
    exit
fi

echo -e "target\t\t:$demo"
echo -e "version\t\t:$lvgl_version"
echo -e "configure\t:$configure"
echo -e "build\t\t:$build"
echo -e "install\t\t:$install"

if [ "$demo" == "lvgl" -o "$demo" == "lv_drivers" ];then
    source configs/lvgl.in
else
    if [ -e configs/${demo}.in ];then
        echo "found configs/${demo}.in"
        source configs/${demo}.in
    elif [ -e ${demo}/config.in ];then
        echo "found ${demo}/config.in"
        source ${demo}/config.in
    else
        echo "config file no found"
        exit
    fi
fi

if [ "$lvgl_version" == "lvgl9" ];then
    thorvg_off=`cat .config | grep BR2_PACKAGE_LVGL_DISABLE_THORVG=y`
    examples_off=`cat .config | grep BR2_PACKAGE_LVGL_DISABLE_EXAMPLES=y`
    demos_off=`cat .config | grep BR2_PACKAGE_LVGL_DISABLE_DEMOS=y`
    if [ "$thorvg_off" != "" ];then
        LVGL_CFG=${LVGL_CFG}" -DLV_CONF_BUILD_DISABLE_THORVG_INTERNAL=1"
    fi
    if [ "$examples_off" != "" ];then
        LVGL_CFG=${LVGL_CFG}" -DLV_CONF_BUILD_DISABLE_EXAMPLES=1"
    fi
    if [ "$demos_off" != "" ];then
        LVGL_CFG=${LVGL_CFG}" -DLV_CONF_BUILD_DISABLE_DEMOS=y"
    fi
    DEMO_CFG=${DEMO_CFG}" -DLVGL_V9=1"
fi

if [ "$debug" == "true" ];then
    LVGL_CFG=${LVGL_CFG}" -DCMAKE_BUILD_TYPE=Debug"
    LV_DRV_CFG=${LV_DRV_CFG}" -DCMAKE_BUILD_TYPE=Debug"
    DEMO_CFG=${DEMO_CFG}" -DCMAKE_BUILD_TYPE=Debug"
fi

if [ "$demo" == "lvgl" ];then
    if [ "$configure" == "true" ];then
        echo "LVGL_CFG: "${LVGL_CFG}
        ./configs/conf.sh .config ${lvgl_path}/lv_conf.mk
        reconfigure_target ${lvgl_path} ${LVGL_CFG} -DCMAKE_INSTALL_PREFIX=lib/${lvgl_version}
    fi
    if [ "$build" == "true" ];then
        build_target ${lvgl_path}
    fi
    if [ "$install" == "true" ];then
        install_target ${lvgl_path}
    fi
elif [ "$demo" == "lv_drivers" ];then
    if [ "$configure" == "true" ];then
        echo "LV_DRV_CFG: "${LV_DRV_CFG}
        reconfigure_target ${lv_drivers_path} ${LV_DRV_CFG} -DCMAKE_INSTALL_PREFIX=lib/${lvgl_version}
    fi
    if [ "$build" == "true" ];then
        build_target ${lv_drivers_path}
    fi
    if [ "$install" == "true" ];then
        install_target ${lv_drivers_path}
    fi
elif [ "$demo" != "" ];then
    if [ "$configure" == "true" ];then
        echo "DEMO_CFG: "${DEMO_CFG}
        reconfigure_target ${demo} ${DEMO_CFG} -DCMAKE_INSTALL_PREFIX=.
    fi
    if [ "$build" == "true" ];then
        build_target ${demo}
    fi
    if [ "$install" == "true" ];then
        install_target ${demo}
    fi
fi

echo "lvgl_version=${lvgl_version}" > .version

