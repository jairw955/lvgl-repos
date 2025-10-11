.PHONY: all menu menu8 menu9 lvgl lvgl8 lvgl9 app lv_demo fruit_ninja ai_home

all: lvgl app
app: lv_demo

menu:
	./compile.sh menuconfig

menu8:
	./compile.sh menuconfig -8

menu9:
	./compile.sh menuconfig -9

lvgl:
	./compile.sh lvgl -a

lvgl8:
	./compile.sh lvgl -a8
	./compile.sh lv_drivers -a8

lvgl9:
	./compile.sh lvgl -a9

lv_demo:
	./compile.sh app -a -s lv_demo

fruit_ninja:
	./compile.sh app -a -s fruit_ninja

ai_home:
	./compile.sh app -a -s ai_home
