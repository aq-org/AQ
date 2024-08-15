# Copyright 2024 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/file.c)

add_subdirectory(identifier)

if(UNIX)
add_subdirectory(unix)
elseif (WIN32)
add_subdirectory(windows)
endif()

add_library(AqvmBaseFile STATIC ${SOURCES})

target_link_libraries(AqvmBaseFile PRIVATE AqvmBaseFileIdentifier)
target_link_libraries(AqvmBaseFile PRIVATE AqvmBaseMemory)
target_link_libraries(AqvmBaseFile PRIVATE AqvmBaseThreadingFileLock)

if(UNIX)
target_link_libraries(AqvmBaseFile PRIVATE AqvmBaseFileUnix)
elseif (WIN32)
target_link_libraries(AqvmBaseFile PRIVATE AqvmBaseFileWindows)
endif()