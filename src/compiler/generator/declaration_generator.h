// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_GENERATOR_DECLARATION_GENERATOR_H_
#define AQ_COMPILER_GENERATOR_DECLARATION_GENERATOR_H_

#include <unordered_map>

#include "compiler/ast/ast.h"
#include "compiler/generator/bytecode.h"
#include "compiler/generator/generator.h"
#include "compiler/generator/memory.h"

namespace Aq {
namespace Compiler {
namespace Generator {
// Handles the import statement.
void HandleImport(Generator& generator, Ast::Import* statement);

// Handles the function declaration.
void HandleFunctionDeclaration(Generator& generator,
                               Ast::FunctionDeclaration* declaration);

// Handles the class function declaration.
void HandleClassFunctionDeclaration(Generator& generator,
                                    Ast::FunctionDeclaration* declaration);

// Handles the class constructor.
void HandleClassConstructor(Generator& generator,
                            Ast::FunctionDeclaration* declaration);

// Handles the class declaration.
void HandleClassDeclaration(Generator& generator, Ast::Class* declaration);

// Handles the variable declaration.
std::size_t HandleVariableDeclaration(Generator& generator,
                                      Ast::Variable* declaration,
                                      std::vector<Bytecode>& code);

// Handles the static declaration.
std::size_t HandleGlobalVariableDeclaration(Generator& generator,
                                            Ast::Variable* declaration,
                                            std::vector<Bytecode>& code);

// Handles the static variable declaration.
std::size_t HandleStaticVariableDeclaration(Generator& generator,
                                            Ast::Variable* declaration);

// Handles the variable declaration.
std::size_t HandleClassVariableDeclaration(Generator& generator,
                                           Ast::Variable* declaration);

// Handles the array declaration.
std::size_t HandleArrayDeclaration(Generator& generator,
                                   Ast::ArrayDeclaration* declaration,
                                   std::vector<Bytecode>& code);

// Handles the global variable declaration.
std::size_t HandleGlobalArrayDeclaration(Generator& generator,
                                         Ast::ArrayDeclaration* declaration,
                                         std::vector<Bytecode>& code);

// Handles the static array declaration.
std::size_t HandleStaticArrayDeclaration(Generator& generator,
                                         Ast::ArrayDeclaration* declaration);

// Handles the class array declaration.
std::size_t HandleClassArrayDeclaration(Generator& generator,
                                        Ast::ArrayDeclaration* declaration);

// Handles the class in variable declaration.
void GenerateBytecode(std::string import_location);

// Gets the function name with scope.
std::string GetFunctionNameWithScope(Generator& generator,
                                     Ast::FunctionDeclaration* declaration);

// Handles the function arguments.
void HandleFunctionArguments(Generator& generator,
                             Ast::FunctionDeclaration* declaration,
                             std::vector<std::size_t>& parameters_index,
                             std::vector<Bytecode>& code);

// Handles the return statement.
void HandleReturnInHandlingFunction(Generator& generator,
                                    std::vector<Bytecode>& code);

// Handles the goto statement in handling function.
void HandleGotoInHandlingFunction(Generator& generator,
                                  std::size_t current_scope,
                                  std::vector<Bytecode>& code);

// Handles the return statement in handling function.
void AddFunctionIntoList(Generator& generator,
                         Ast::FunctionDeclaration* declaration,
                         std::vector<std::size_t>& parameters_index,
                         std::vector<Bytecode>& code);

// Handles the class function declaration.
void AddClassFunctionIntoList(Generator& generator,
                              Ast::FunctionDeclaration* declaration,
                              std::vector<std::size_t>& parameters_index,
                              std::vector<Bytecode>& code);

// Handles the class constructor function declaration.
void AddClassConstructorFunctionIntoList(
    Generator& generator, Ast::FunctionDeclaration* declaration,
    std::vector<std::size_t>& parameters_index, std::vector<Bytecode>& code);

// Handles the return statement in handling function.
void HandleReturnVariableInHandlingFunction(
    Generator& generator, Ast::FunctionDeclaration* declaration,
    std::string scope_name, std::vector<std::size_t>& parameters_index);

// Handles the factory function in handling constructor.
std::vector<std::size_t> HandleFactoryFunctionInHandlingConstructor(
    Generator& generator, Ast::FunctionDeclaration* declaration);

// Handles the constructor function in handling constructor.
void HandleConstructorFunctionInHandlingConstructor(
    Generator& generator, Ast::FunctionDeclaration* declaration,
    std::vector<std::size_t>& parameters_index);

// Handles the class statement.
void HandleSubClassesInHandlingClass(Generator& generator,
                                     Ast::Class* declaration);

// Handles the static members in handling class.
void HandleStaticMembersInHandlingClass(Generator& generator,
                                        Ast::Class* declaration);

// Handles the class members in handling class.
void HandleClassMembersInHandlingClass(Generator& generator,
                                       Ast::Class* declaration);

// Handles the methods in handling class.
void HandleMethodsInHandlingClass(Generator& generator,
                                  Ast::Class* declaration);

// Handles the class in handling variable.
void AddVoidConstructorInHandlingClass(Generator& generator,
                                       Ast::Class* declaration);

// Handles the void factory function in handling class.
std::vector<std::size_t> HandleVoidFactoryFunctionInHandlingClass(
    Generator& generator, Ast::Class* declaration);

// Handles the void constructor function in handling class.
void HandleVoidConstructorFunctionInHandlingClass(
    Generator& generator, Ast::Class* declaration,
    std::vector<std::size_t>& parameters_index);

// Handles the class in handling variable.
void HandleClassInHandlingVariable(Generator& generator,
                                   Ast::Variable* declaration,
                                   std::size_t variable_index,
                                   std::vector<Bytecode>& code);

// Handles the class in handling variable.
std::string GetClassNameString(Generator& generator, Ast::ClassType* type);
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif