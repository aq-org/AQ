# Copyright 2025 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "C compiler is required but not found")
endif()
if(NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "C++ compiler is required but not found")
endif()

find_program(MAKE_EXECUTABLE NAMES gmake nmake make)
if(NOT MAKE_EXECUTABLE)
    message(FATAL_ERROR "Make utility is required but not found")
endif()

if(WIN32)
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")
endif()

add_subdirectory(cpython)