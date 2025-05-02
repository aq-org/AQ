# Copyright 2025 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include(ExternalProject)
include(GNUInstallDirs)

if(WIN32)
    set(PYTHON_VER 314)
    set(PYTHON_LIB_NAME python${PYTHON_VER}.lib)
    set(INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/install)
else()
    set(PYTHON_VER 3.14)
    set(PYTHON_LIB_NAME libpython${PYTHON_VER}.a)
    set(INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/install)
endif()

set(CPYTHON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CPYTHON_INCLUDE_DIR ${INSTALL_DIR}/include/python${PYTHON_VER})

if(WIN32)
    if(NOT DEFINED CMAKE_VS_MSBUILD_COMMAND)
        get_filename_component(VS_DIR [HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\VisualStudio\\SxS\\VS7;Latest] ABSOLUTE)
        
        find_program(VSWHERE_EXE vswhere
            PATHS "$ENV{ProgramFiles(x86)}/Microsoft Visual Studio/Installer"
            NO_DEFAULT_PATH)
        
        if(VSWHERE_EXE)
            execute_process(
                COMMAND "${VSWHERE_EXE}" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
                OUTPUT_VARIABLE VS_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        endif()
        
        set(MSBUILD_PATHS
            "${VS_DIR}/MSBuild/Current/Bin"
            "${VS_DIR}/MSBuild/15.0/Bin"
            "$ENV{MSBUILD_DIR}/Bin"
            "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/MSBuild/Current/Bin"
            "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/MSBuild/Current/Bin"
        )
    else()
        set(MSBUILD_PATHS ${CMAKE_VS_MSBUILD_COMMAND})
    endif()

    find_program(MSBUILD_EXE
        NAMES MSBuild.exe msbuild
        PATHS ${MSBUILD_PATHS}
        DOC "MSBuild executable path"
    )

    if(NOT MSBUILD_EXE)
        message(FATAL_ERROR "MSBuild not found. Solutions:\n"
            "1. Install Visual Studio Build Tools\n"
            "2. Set MSBUILD_DIR environment variable\n"
            "3. Use Developer Command Prompt")
    endif()
else()
    set(CPYTHON_STATIC_FLAGS
        --enable-static
        --disable-shared
        --prefix=${INSTALL_DIR}
        --with-ensurepip=no
        CFLAGS=-fPIC
    )

    ExternalProject_Add(cpython_build
        SOURCE_DIR    ${CPYTHON_SOURCE_DIR}
        BINARY_DIR    ${CMAKE_CURRENT_BINARY_DIR}/build
        INSTALL_DIR   ${INSTALL_DIR}
        
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CPYTHON_STATIC_FLAGS}
        BUILD_COMMAND     $(MAKE) -j
        INSTALL_COMMAND   $(MAKE) install
        BUILD_BYPRODUCTS  ${INSTALL_DIR}/lib/${PYTHON_LIB_NAME}
    )
endif()

add_library(libpython STATIC IMPORTED GLOBAL)
add_dependencies(libpython cpython_build)

if(WIN32)
    set_target_properties(libpython PROPERTIES
        IMPORTED_LOCATION ${INSTALL_DIR}/lib/${PYTHON_LIB_NAME}
        INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include/python${PYTHON_VER}
    )
else()
    file(MAKE_DIRECTORY ${CPYTHON_INCLUDE_DIR})
    set_target_properties(libpython PROPERTIES
        IMPORTED_LOCATION ${INSTALL_DIR}/lib/${PYTHON_LIB_NAME}
        INTERFACE_INCLUDE_DIRECTORIES ${CPYTHON_INCLUDE_DIR}
    )
endif()

add_library(cpython::static ALIAS libpython)
set(PYTHON_INCLUDE_DIRS ${CPYTHON_INCLUDE_DIR} PARENT_SCOPE)