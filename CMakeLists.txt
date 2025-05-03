# Copyright 2025 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)
project(AQ C CXX)
# enable_testing()

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_COMPILER "gcc")

if(WIN32)
    set(KNOWN_ARCH_MAPPINGS
        "AMD64:x64"
        "Win32:x86"
        "ARM:arm32"
        "ARM64:arm64"

        "x86_64:x64"
        "amd64:x64"
        "i[3-6]86:x86"
        "aarch64:arm64"
        "armv[5-8]:arm32"
        "arm64:arm64"
    )

    macro(detect_system_architecture)
        if(CMAKE_GENERATOR_PLATFORM)
            set(DETECTED_ARCH "${CMAKE_GENERATOR_PLATFORM}")
        else()
            set(DETECTED_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
        endif()

        foreach(mapping IN LISTS KNOWN_ARCH_MAPPINGS)
            string(REPLACE ":" ";" mapping_pair ${mapping})
            list(GET mapping_pair 0 pattern)
            list(GET mapping_pair 1 arch)

            if(DETECTED_ARCH MATCHES "^${pattern}$")
                set(SYSTEM_ARCH "${arch}")
                break()
            endif()
        endforeach()

        if(APPLE AND CMAKE_OSX_ARCHITECTURES)
            list(GET CMAKE_OSX_ARCHITECTURES 0 mac_arch)
            if(mac_arch STREQUAL "arm64")
                set(SYSTEM_ARCH "arm64")
            elseif(mac_arch STREQUAL "x86_64")
                set(SYSTEM_ARCH "x64")
            endif()
        endif()

        if(NOT SYSTEM_ARCH)
            message(FATAL_ERROR "Unable to automatically detect system architecture! Please manually specify -DCMAKE_GENERATOR_PLATFORM=xxx")
        endif()
    endmacro()

    if(NOT CMAKE_GENERATOR_PLATFORM)
        detect_system_architecture()
        set(CMAKE_GENERATOR_PLATFORM "${SYSTEM_ARCH}" CACHE STRING "target platform" FORCE)
        message(STATUS "Automatically detect system architecture: ${SYSTEM_ARCH}")
    endif()

    string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" PYTHON_PLATFORM_TMP)
    if(PYTHON_PLATFORM_TMP STREQUAL "x64")
        set(PYTHON_PLATFORM "amd64")
    else()
        set(PYTHON_PLATFORM "${PYTHON_PLATFORM_TMP}")
    endif()

    set(SUPPORTED_PLATFORMS amd64 x86 arm32 arm64)
    if(NOT PYTHON_PLATFORM IN_LIST SUPPORTED_PLATFORMS)
        message(FATAL_ERROR "Unsupported platform: ${PYTHON_PLATFORM}\nValid platforms: ${SUPPORTED_PLATFORMS}")
    endif()
endif()

if(WIN32)
    set(PYTHON_VER 314)
else()
    set(PYTHON_VER 3.14)
endif()

add_subdirectory(third-party)

include_directories(${PROJECT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/third-party/cpython/PCbuild/${PYTHON_PLATFORM})
include_directories(${PROJECT_SOURCE_DIR}/third-party/cpython/Include)


set(COMPILER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/prototype/compiler.cc)
set(VM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/prototype/vm.c)

add_executable(aq_compiler ${COMPILER_SOURCES})
add_executable(aq_vm ${VM_SOURCES})

if(WIN32)
    add_custom_command(TARGET aq_vm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_SOURCE_DIR}/third-party/cpython/PCbuild/${PYTHON_PLATFORM}/python${PYTHON_VER}.dll
            $<TARGET_FILE_DIR:aq_vm>
    )
else()
    target_link_libraries(aq_vm PRIVATE
        cpython::static
        ${PYTHON_SYSTEM_LIBS}
    )

    target_include_directories(aq_vm PRIVATE ${PYTHON_INCLUDE_DIRS})
endif()