# Copyright 2024 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/identifier.c)

if(UNIX)
add_subdirectory(unix)
elseif (WIN32)
add_subdirectory(windows)
endif()

add_library(AqvmBaseFileIdentifier STATIC ${SOURCES})

if(unix)
target_link_libraries(AqvmBaseFileIdentifier PRIVATE AqvmBaseFileIdentifierUnix)
elseif (WIN32)
target_link_libraries(AqvmBaseFileIdentifier PRIVATE AqvmBaseFileIdentifierWindows)
endif()