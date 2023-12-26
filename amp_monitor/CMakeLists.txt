cmake_minimum_required(VERSION 3.8)

project(amp_monitor)

include(FindPkgConfig)
find_package(PkgConfig)

include_directories(
	${PROJECT_SOURCE_DIR}/./
	)

aux_source_directory(. SRCS)

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME}
	lvgl pthread m lv_drivers freetype
)

if (LV_DRV_USE_OPENGL)
	pkg_check_modules(PKG_GL glesv2)
	target_link_libraries(${PROJECT_NAME} ${PKG_GL_LIBRARIES})
endif()
if (LV_DRV_USE_SDL_GPU)
	target_link_libraries(${PROJECT_NAME} SDL2)
endif()
if (LV_DRV_USE_DRM)
	target_link_libraries(${PROJECT_NAME} drm)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

