cmake_minimum_required(VERSION 3.12.4)

if(NOT ESP_PLATFORM)
  project(lvgl HOMEPAGE_URL https://github.com/lvgl/lvgl)
endif()

set(LVGL_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/lv_conf_default.mk)
if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/lv_conf.mk)
    include(${CMAKE_CURRENT_LIST_DIR}/lv_conf.mk)
endif()

configure_file(
    ${PROJECT_SOURCE_DIR}/lv_conf.h.in
    ${PROJECT_SOURCE_DIR}/lv_conf.h
)

if(ESP_PLATFORM)
  include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/esp.cmake)
elseif(ZEPHYR_BASE)
  include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/zephyr.cmake)
elseif(MICROPY_DIR)
  include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/micropython.cmake)
else()
  include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/custom.cmake)
endif()
