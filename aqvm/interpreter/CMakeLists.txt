# Copyright 2024 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/interpreter.c)

add_library(AqvmInterpreter STATIC ${SOURCES})

add_subdirectory(operator)

target_link_libraries(AqvmInterpreter PRIVATE AqvmBase)
target_link_libraries(AqvmInterpreter PRIVATE AqvmMemory)
target_link_libraries(AqvmInterpreter PRIVATE AqvmBytecode)
target_link_libraries(AqvmInterpreter PRIVATE AqvmInterpreterOperator)