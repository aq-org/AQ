# Copyright 2024 AQ authors, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/linked_list.cc)

add_library(AqCompilerLinkedList STATIC ${SOURCES})

target_link_libraries(AqCompilerLinkedList PRIVATE AqDebugger)