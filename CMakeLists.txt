# Copyright 2025 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)
project(AQ C CXX)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)

set(COMPILER_SOURCES ${PROJECT_SOURCE_DIR}/src/compiler/compiler.cc 
${PROJECT_SOURCE_DIR}/src/compiler/ast/ast.cc 
${PROJECT_SOURCE_DIR}/src/compiler/ast/type.cc 
${PROJECT_SOURCE_DIR}/src/compiler/lexer/lexer.cc 
${PROJECT_SOURCE_DIR}/src/compiler/parser/parser.cc 
${PROJECT_SOURCE_DIR}/src/compiler/parser/declaration_parser.cc 
${PROJECT_SOURCE_DIR}/src/compiler/parser/expression_parser.cc 
${PROJECT_SOURCE_DIR}/src/compiler/token/token.cc 
${PROJECT_SOURCE_DIR}/src/compiler/token/tokenmap.cc 
${PROJECT_SOURCE_DIR}/src/compiler/generator/generator.cc
${PROJECT_SOURCE_DIR}/src/compiler/generator/builtin.cc
${PROJECT_SOURCE_DIR}/src/compiler/generator/declaration_generator.cc
${PROJECT_SOURCE_DIR}/src/compiler/generator/expression_generator.cc
${PROJECT_SOURCE_DIR}/src/compiler/generator/goto_generator.cc
${PROJECT_SOURCE_DIR}/src/compiler/generator/memory.cc
${PROJECT_SOURCE_DIR}/src/compiler/generator/preprocesser.cc
${PROJECT_SOURCE_DIR}/src/compiler/generator/statement_generator.cc)

set(VM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/prototype/vm.c)

add_executable(aq_compiler ${COMPILER_SOURCES})
add_executable(aq_vm ${VM_SOURCES})

if(UNIX)
TARGET_LINK_LIBRARIES(aq_vm m)
TARGET_LINK_LIBRARIES(aq_vm ncurses)
endif()