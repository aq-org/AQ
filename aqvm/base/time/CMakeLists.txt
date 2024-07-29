# Copyright 2024 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/time.c)

add_subdirectory(unix)
add_subdirectory(windows)

add_library(AqvmBaseTime STATIC ${SOURCES})

target_link_libraries(AqvmBaseTime PRIVATE AqvmBaseTimeUnix)
target_link_libraries(AqvmBaseTime PRIVATE AqvmBaseTimeWindows)