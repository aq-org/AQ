# Copyright 2024 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/io.c)

add_library(AqvmBaseIo STATIC ${SOURCES})

target_link_libraries(AqvmBaseIo PRIVATE AqvmBaseThreadingMutex)
target_link_libraries(AqvmBaseIo PRIVATE AqvmBaseProcessFileLock)