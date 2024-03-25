#!/bin/bash -i
#

lvgl=false
lv_drv=false
demo=
configure=false
build=true
install=false
ext=

function help(){
    echo -e "$1 <target> <options>"
    echo -e "  target\tlvgl|lv_drv|demos"
    echo -e "  -c\t\tconfigure(false by default)"
    echo -e "  -b\t\tbuild(true by default)"
    echo -e "  -i\t\tinstall(false by default)"
    echo -e "  -a\t\tall, -c -b -i"
    echo -e "  -e\t\tenvironment, lvgl and lv_drivers"
    echo -e "  -s\t\tdemo select"
    echo -e "  -h\t\tthis help"
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
    cmake -S $1 -B $1/build -DCMAKE_INSTALL_PREFIX=/usr ${@:2}
}

function parse_args(){
    len=${#1}
    for ((i=1;i<len;i++));do
        case ${1:i:1} in
        "c")
            configure=true
            ;;
        "b")
            build=true
            ;;
        "a")
            lvgl=true
            lv_drv=true
            ;;
        "e")
            lvgl=true
            lv_drv=true
            ;;
        "i")
            install=true
            ;;
        esac
    done
}

while [ ! -z "$1" ]
do
    case $1 in
    "-c")
        configure=true
        shift 1
        ;;
    "-b")
        build=true
        shift 1
        ;;
    "-i")
        install=true
        shift 1
        ;;
    "-a")
        configure=true
        build=true
        install=true
        shift 1
        ;;
    "-e")
        lvgl=true
        lv_drv=true
        shift 1
        ;;
    "-s")
        ext=$2
        shift 2
        ;;
    "-h")
        help $0
        exit
        ;;
    *)
        if [ "$1" == "lvgl" ];then
            lvgl=true
        elif [ "$1" == "lv_drv" ];then
            lv_drv=true
        elif [ "${1:0:1}" == "-" ];then
            parse_args $1
        else
            demo=$1
        fi
        shift 1
        ;;
    esac
done

if [ "$demo" == "" -a "$lvgl" == "false" -a "$lv_drv" == "false" ];then
    echo "nothing to compile"
    help $0
    exit
fi

echo -e "target\t\t:$demo"
echo -e "lvgl\t\t:$lvgl"
echo -e "lv_drv\t\t:$lv_drv"
echo -e "configure\t:$configure"
echo -e "build\t\t:$build"
echo -e "install\t\t:$install"

if [ "$demo" != "" ];then
    if [ -e configs/${demo}.in ];then
        source configs/${demo}.in
    elif [ -e ${demo}/config.in ];then
        source ${demo}/config.in
    else
        echo "config file no found"
        exit
    fi
else
    source configs/lvgl.in
fi

if [ "$lvgl" == "true" ];then
    if [ "$configure" == "true" ];then
        echo "LVGL_CFG: "${LVGL_CFG}
        reconfigure_target lvgl ${LVGL_CFG}
    fi
    if [ "$build" == "true" ];then
        build_target lvgl
    fi
    if [ "$install" == "true" ];then
        install_target lvgl
    fi
fi

if [ "$lv_drv" == "true" ];then
    if [ "$configure" == "true" ];then
        echo "LV_DRV_CFG: "${LV_DRV_CFG}
        reconfigure_target lv_drivers ${LV_DRV_CFG}
    fi
    if [ "$build" == "true" ];then
        build_target lv_drivers
    fi
    if [ "$install" == "true" ];then
        install_target lv_drivers
    fi
fi

if [ "$demo" != "" ];then
    if [ "$configure" == "true" ];then
        echo "DEMO_CFG: "${DEMO_CFG}
        reconfigure_target ${DEMO} ${DEMO_CFG}
    fi
    if [ "$build" == "true" ];then
        build_target ${DEMO}
    fi
    if [ "$install" == "true" ];then
        install_target ${DEMO}
    fi
fi

