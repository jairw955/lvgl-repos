cmake_minimum_required(VERSION 3.8)

project(lv_demo)

include_directories(${PROJECT_SOURCE_DIR}/./)

if (PC_SIMULATOR)
	add_definitions(-DPC_SIMULATOR=1)
endif()

if (LVGL_DEMO_WIDGETS)
	add_definitions(-DUSE_DEMO_WIDGETS=1)
	set(LINK_LVGL_DEMOS ON)
elseif (LVGL_DEMO_BENCHMARK)
	add_definitions(-DUSE_DEMO_BENCHMARK=1)
	set(LINK_LVGL_DEMOS ON)
elseif (LVGL_DEMO_MUSIC)
	add_definitions(-DUSE_DEMO_MUSIC=1)
	set(LINK_LVGL_DEMOS ON)
elseif (EEZ_DEMO)
	add_definitions(-DUSE_DEMO_EEZ=1)
	aux_source_directory(ui SRCS)
endif()

aux_source_directory(. SRCS)

add_executable(${PROJECT_NAME} ${SRCS})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

