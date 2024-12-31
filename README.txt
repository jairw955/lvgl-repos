make menuconfig -- 打开menuconfig，自动根据上一次选择版本
make menuconfig8 -- 打开lvgl8 menuconfig，保存版本信息
make menuconfig9 -- 打开lvgl9 menuconfig，保存版本信息

make base8 -- 根据configs/lvgl8.base编译lvgl和lv_drivers，保存版本信息
make base9 -- 根据configs/lvgl9.base编译lvgl，保存版本信息

make lvgl -- 增量编译lvgl9，保存版本信息
make lvgl-install -- 增量编译并安装lvgl9，保存版本信息
make lvgl-reconfigure -- 完整重编并安装lvgl9，保存版本信息

make lvgl8 -- 增量编译lvgl8，保存版本信息
make lvgl8-install -- 增量编译并安装lvgl8，保存版本信息
make lvgl8-reconfigure -- 完整重编并安装lvgl8，保存版本信息

make lv_drivers -- 增量编译lv_drivers，保存版本信息
make lv_drivers-install -- 增量编译并安装lv_drivers，保存版本信息
make lv_drivers-reconfigure -- 完整重编并安装lv_drivers，保存版本信息

make app -- 增量编译app，自动根据上一次选择版本
make app-install -- 增量编译并安装app，自动根据上一次选择版本
make app-reconfigure -- 完整重编并安装app，自动根据上一次选择版本
