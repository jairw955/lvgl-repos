cmake_minimum_required(VERSION 3.8)

project(lvgl_app)

add_definitions(-g -DLV_CONF_INCLUDE_SIMPLE=1)

include_directories(
	${PROJECT_SOURCE_DIR}/sys/
	${PROJECT_SOURCE_DIR}/common/
	${PROJECT_SOURCE_DIR}/common/cJSON
	${CMAKE_SYSROOT}/usr/include/libdrm
	)

link_libraries(lvgl pthread m freetype drm)
if (LV_DRV_USE_SDL_GPU)
	link_libraries(SDL2)
endif()
if (LV_DRV_USE_OPENGLES)
	link_libraries(GLESv2 gbm)
endif()

if (LVGL_V9)
	add_definitions(-DLVGL_V9=1)
	set(LVGL_LIB_ROOT "${PROJECT_SOURCE_DIR}/../lib/lvgl9")
	include_directories(${PROJECT_SOURCE_DIR}/lvgl9/)
	include_directories(${LVGL_LIB_ROOT}/include/)
	include_directories(${LVGL_LIB_ROOT}/include/lvgl)
	aux_source_directory(${PROJECT_SOURCE_DIR}/lvgl9 SRCS)
else()
	set(LVGL_LIB_ROOT "${PROJECT_SOURCE_DIR}/../lib/lvgl8")
	include_directories(${PROJECT_SOURCE_DIR}/lvgl8/)
	include_directories(${LVGL_LIB_ROOT}/include/)
	include_directories(${LVGL_LIB_ROOT}/include/lvgl)
	include_directories(${LVGL_LIB_ROOT}/include/lvgl/lv_drivers)
	aux_source_directory(${PROJECT_SOURCE_DIR}/lvgl8 SRCS)
	if (LV_DRV_USE_SDL_GPU)
		add_definitions(-DUSE_SDL_GPU=1)
		add_definitions(-DLV_USE_GPU_SDL=1)
	if (LV_DRV_SDL_DIS_FULLSCREEN)
		add_definitions(-DSDL_DIS_FULLSCREEN)
	endif()
	endif()
endif()

file(GLOB LVGL_LIBS "${LVGL_LIB_ROOT}/lib/*.so*")
link_libraries(${LVGL_LIBS})

aux_source_directory(${PROJECT_SOURCE_DIR}/sys SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/cJSON SRCS)

add_subdirectory(demos)

if(LVGL_LIBS)
	install(FILES ${LVGL_LIBS} DESTINATION lib)
endif()

if (EXISTS "${PROJECT_SOURCE_DIR}/tools/pre-commit")
	set(GIT_HOOK_SRC "${PROJECT_SOURCE_DIR}/tools/pre-commit")
	if(EXISTS "${PROJECT_SOURCE_DIR}/.git/hooks")
		set(GIT_HOOK_DST "${PROJECT_SOURCE_DIR}/.git/hooks/")
		file(COPY ${GIT_HOOK_SRC} DESTINATION ${GIT_HOOK_DST})
		message(STATUS "Install git hooks done")
	endif()
endif()

