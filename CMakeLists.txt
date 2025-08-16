# Copyright 2025 AQ author, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)
project(AQ CXX)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")



include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)

set(SOURCES ${PROJECT_SOURCE_DIR}/src/aq.cc 
${PROJECT_SOURCE_DIR}/src/ast/ast.cc 
${PROJECT_SOURCE_DIR}/src/ast/type.cc 
${PROJECT_SOURCE_DIR}/src/lexer/lexer.cc 
${PROJECT_SOURCE_DIR}/src/parser/parser.cc 
${PROJECT_SOURCE_DIR}/src/parser/declaration_parser.cc 
${PROJECT_SOURCE_DIR}/src/parser/expression_parser.cc 
${PROJECT_SOURCE_DIR}/src/token/token.cc 
${PROJECT_SOURCE_DIR}/src/token/tokenmap.cc 
${PROJECT_SOURCE_DIR}/src/interpreter/interpreter.cc
${PROJECT_SOURCE_DIR}/src/interpreter/operator.cc
${PROJECT_SOURCE_DIR}/src/interpreter/builtin.cc
${PROJECT_SOURCE_DIR}/src/interpreter/declaration_interpreter.cc
${PROJECT_SOURCE_DIR}/src/interpreter/expression_interpreter.cc
${PROJECT_SOURCE_DIR}/src/interpreter/goto_interpreter.cc
${PROJECT_SOURCE_DIR}/src/interpreter/memory.cc
${PROJECT_SOURCE_DIR}/src/interpreter/preprocesser.cc
${PROJECT_SOURCE_DIR}/src/interpreter/statement_interpreter.cc)

add_executable(aq ${SOURCES})