# Copyright 2025 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)
project(AQ C CXX)
# enable_testing()

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_COMPILER "gcc")

include_directories(${PROJECT_SOURCE_DIR})

set(COMPILER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/prototype/compiler.cc)
set(VM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/prototype/vm.c)

add_executable(aq_compiler ${COMPILER_SOURCES})
add_executable(aq_vm ${VM_SOURCES})
