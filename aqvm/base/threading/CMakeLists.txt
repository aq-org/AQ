# Copyright 2024 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/threading.c)

add_subdirectory(file_lock)
add_subdirectory(mutex)

add_library(AqvmBaseThreading STATIC ${SOURCES})

target_link_libraries(AqvmBaseThreading PRIVATE AqvmBaseProcessFileLock)
target_link_libraries(AqvmBaseThreading PRIVATE AqvmBaseThreadingMutex)