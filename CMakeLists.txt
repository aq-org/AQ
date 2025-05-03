# Copyright 2025 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)
project(AQ C CXX)
# enable_testing()

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_COMPILER "gcc")

add_subdirectory(third-party)

include_directories(${PROJECT_SOURCE_DIR})

set(COMPILER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/prototype/compiler.cc)
set(VM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/prototype/vm.c)

add_executable(aq_compiler ${COMPILER_SOURCES})
add_executable(aq_vm ${VM_SOURCES})


if(WIN32)
    add_custom_command(TARGET aq_vm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CPYTHON_BINARY_DIR}/amd64/python${PYTHON_VER}.dll
            $<TARGET_FILE_DIR:aq_vm>
    )
else()
    target_link_libraries(aq_vm PRIVATE
        cpython::static
        ${PYTHON_SYSTEM_LIBS}
    )

    target_include_directories(aq_vm PRIVATE ${PYTHON_INCLUDE_DIRS})
endif()