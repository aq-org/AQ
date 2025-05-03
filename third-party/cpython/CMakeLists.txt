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
    ExternalProject_Add(cpython_dynamic
        SOURCE_DIR      ${CPYTHON_SOURCE_DIR}
        BINARY_DIR      ${CPYTHON_BUILD_DIR}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND
            # COMMAND ${CMAKE_COMMAND} -E chdir ${CPYTHON_BUILD_DIR}
            ${PROJECT_SOURCE_DIR}/third-party/cpython/PCbuild/build.bat -p x64 -c Release
        INSTALL_COMMAND ""
        BUILD_BYPRODUCTS
            ${CPYTHON_BUILD_DIR}/amd64/python${PYTHON_VER}.lib
            ${CPYTHON_BUILD_DIR}/amd64/python${PYTHON_VER}.dll
    )

    add_library(libpython SHARED IMPORTED GLOBAL)
    add_dependencies(libpython cpython_build)


    set_target_properties(libpython PROPERTIES
        IMPORTED_IMPLIB ${CPYTHON_BINARY_DIR}/python${PYTHON_VER}.lib
        IMPORTED_LOCATION ${CPYTHON_BINARY_DIR}/python${PYTHON_VER}.dll
        INTERFACE_INCLUDE_DIRECTORIES ${CPYTHON_SOURCE_DIR}/Include
    )

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
        BUILD_COMMAND     $(MAKE) -j8 VERBOSE=1
        INSTALL_COMMAND   $(MAKE) install
        BUILD_BYPRODUCTS  ${INSTALL_DIR}/lib/${PYTHON_LIB_NAME}
    )
    add_library(libpython STATIC IMPORTED GLOBAL)
    add_dependencies(libpython cpython_build)
endif()

if(WIN32)
else()
    file(MAKE_DIRECTORY ${CPYTHON_INCLUDE_DIR})
    set_target_properties(libpython PROPERTIES
        IMPORTED_LOCATION ${INSTALL_DIR}/lib/${PYTHON_LIB_NAME}
        INTERFACE_INCLUDE_DIRECTORIES ${CPYTHON_INCLUDE_DIR}
    )
endif()

add_library(cpython::static ALIAS libpython)
set(PYTHON_INCLUDE_DIRS ${CPYTHON_INCLUDE_DIR} PARENT_SCOPE)