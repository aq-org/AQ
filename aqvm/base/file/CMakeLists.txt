# Copyright 2024 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/file.c)

if (WIN32)
add_subdirectory(windows)
endif()

add_subdirectory(identifier)

add_library(AqvmBaseFile STATIC ${SOURCES})

if (WIN32)
target_link_libraries(AqvmBaseFile PRIVATE AqvmBaseFileWindows)
endif()

target_link_libraries(AqvmBaseFile PRIVATE AqvmBaseFileIdentifier)