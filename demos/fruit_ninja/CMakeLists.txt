cmake_minimum_required(VERSION 3.8)

project(fruit_ninja)

include_directories(${PROJECT_SOURCE_DIR}/./)

if (PC_SIMULATOR)
	add_definitions(-DPC_SIMULATOR=1)
endif()

aux_source_directory(. SRCS)

add_executable(${PROJECT_NAME} ${SRCS})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

