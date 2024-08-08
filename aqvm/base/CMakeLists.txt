# Copyright 2024 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/base.c)

add_subdirectory(file)
add_subdirectory(time)
add_subdirectory(process)
add_subdirectory(threading)
add_subdirectory(hash)
add_subdirectory(io)
add_subdirectory(linked_list)
add_subdirectory(logging)

add_library(AqvmBase STATIC ${SOURCES})

target_link_libraries(AqvmBase PRIVATE AqvmBaseFile)
target_link_libraries(AqvmBase PRIVATE AqvmBaseTime)
target_link_libraries(AqvmBase PRIVATE AqvmBaseProcess)
target_link_libraries(AqvmBase PRIVATE AqvmBaseThreading)
target_link_libraries(AqvmBase PRIVATE AqvmBaseHash)
target_link_libraries(AqvmBase PRIVATE AqvmBaseIo)
target_link_libraries(AqvmBase PRIVATE AqvmBaseLinkedList)
target_link_libraries(AqvmBase PRIVATE AqvmBaseLogging)