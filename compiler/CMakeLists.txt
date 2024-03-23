# Copyright 2024 AQ authors, All Rights Reserved.
# This program is licensed under the AQ License. You can find the AQ license in
# the root directory.

cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/compiler.cc)


add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(token)
add_subdirectory(ast)
add_subdirectory(pair)
add_subdirectory(linked_list)
add_subdirectory(dyn_array)
add_subdirectory(hash_map)

add_library(AqCompiler STATIC ${SOURCES})

target_link_libraries(AqCompiler PRIVATE AqCompilerLexer)
target_link_libraries(AqCompiler PRIVATE AqCompilerParser)
target_link_libraries(AqCompiler PRIVATE AqCompilerToken)
target_link_libraries(AqCompiler PRIVATE AqCompilerAst)
target_link_libraries(AqCompiler PRIVATE AqCompilerPair)
target_link_libraries(AqCompiler PRIVATE AqCompilerLinkedList)
target_link_libraries(AqCompiler PRIVATE AqCompilerDynArray)
target_link_libraries(AqCompiler PRIVATE AqCompilerHashMap)
target_link_libraries(AqCompiler PRIVATE AqDebugger)