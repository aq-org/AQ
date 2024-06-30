# Copyright 2024 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)
project(AQ C)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(aqvm)
add_subdirectory(aq)