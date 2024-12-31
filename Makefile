.PHONY: menuconfig
menuconfig:
	./compile.sh menuconfig

.PHONY: menuconfig8
menuconfig8:
	./compile.sh menuconfig -8

.PHONY: menuconfig9
menuconfig9:
	./compile.sh menuconfig -9

.PHONY: base8
base8:
	./compile.sh lvgl -a8
	./compile.sh lv_drivers -a8

.PHONY: base9
base9:
	./compile.sh lvgl -a9

.PHONY: lvgl
lvgl:
	./compile.sh lvgl -b9
.PHONY: lvgl-install
lvgl-install:
	./compile.sh lvgl -bi9
.PHONY: lvgl-reconfigure
lvgl-reconfigure:
	./compile.sh lvgl -a9

.PHONY: lvgl8
lvgl8:
	./compile.sh lvgl -b8
.PHONY: lvgl8-install
lvgl8-install:
	./compile.sh lvgl -bi8
.PHONY: lvgl8-reconfigure
lvgl8-reconfigure:
	./compile.sh lvgl -a8
.PHONY: lv_drivers
lv_drivers:
	./compile.sh lv_drivers -b8
.PHONY: lv_drivers-install
lv_drivers-install:
	./compile.sh lv_drivers -bi8
.PHONY: lv_drivers-reconfigure
lv_drivers-reconfigure:
	./compile.sh lv_drivers -a8

.PHONY: app
app:
	./compile.sh app -b
.PHONY: app-install
app-install:
	./compile.sh app -bi
.PHONY: app-reconfigure
app-reconfigure:
	./compile.sh app -a

