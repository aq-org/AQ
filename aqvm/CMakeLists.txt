# Copyright 2024 AQ authors, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/aqvm.c)

add_subdirectory(runtime)
add_subdirectory(memory)
add_subdirectory(os)
add_subdirectory(interpreter)

add_library(Aqvm STATIC ${SOURCES})

target_link_libraries(Aqvm PRIVATE AqvmMemory)
target_link_libraries(Aqvm PRIVATE AqvmOs)
target_link_libraries(Aqvm PRIVATE AqvmRuntime)
target_link_libraries(Aqvm PRIVATE AqvmInterpreter)