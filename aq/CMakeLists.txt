# Copyright 2024 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/aq.c)

add_executable(aq ${SOURCES})

target_link_libraries(aq PRIVATE Aqvm)