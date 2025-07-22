// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_GENERATOR_DECLARATION_GENERATOR_H_
#define AQ_GENERATOR_DECLARATION_GENERATOR_H_

#include "ast/ast.h"
#include "interpreter/bytecode.h"
#include "interpreter/interpreter.h"

namespace Aq {
namespace Interpreter {
// Handles the import statement.
void HandleImport(Interpreter& interpreter, Ast::Import* statement);

// Handles the function declaration.
void HandleFunctionDeclaration(Interpreter& interpreter,
                               Ast::FunctionDeclaration* declaration);

// Handles the class function declaration.
void HandleClassFunctionDeclaration(Interpreter& interpreter,
                                    Ast::FunctionDeclaration* declaration);

// Handles the class constructor.
void HandleClassConstructor(Interpreter& interpreter,
                            Ast::FunctionDeclaration* declaration);

// Handles the class declaration.
void HandleClassDeclaration(Interpreter& interpreter, Ast::Class* declaration);

// Handles the variable declaration.
std::size_t HandleVariableDeclaration(Interpreter& interpreter,
                                      Ast::Variable* declaration,
                                      std::vector<Bytecode>& code);

// Handles the static declaration.
std::size_t HandleGlobalVariableDeclaration(Interpreter& interpreter,
                                            Ast::Variable* declaration,
                                            std::vector<Bytecode>& code);

// Handles the static variable declaration.
std::size_t HandleStaticVariableDeclaration(Interpreter& interpreter,
                                            Ast::Variable* declaration);

// Handles the variable declaration.
std::size_t HandleClassVariableDeclaration(Interpreter& interpreter,
                                           Ast::Variable* declaration);

// Handles the array declaration.
std::size_t HandleArrayDeclaration(Interpreter& interpreter,
                                   Ast::ArrayDeclaration* declaration,
                                   std::vector<Bytecode>& code);

// Handles the global variable declaration.
std::size_t HandleGlobalArrayDeclaration(Interpreter& interpreter,
                                         Ast::ArrayDeclaration* declaration,
                                         std::vector<Bytecode>& code);

// Handles the static array declaration.
std::size_t HandleStaticArrayDeclaration(Interpreter& interpreter,
                                         Ast::ArrayDeclaration* declaration);

// Handles the class array declaration.
std::size_t HandleClassArrayDeclaration(Interpreter& interpreter,
                                        Ast::ArrayDeclaration* declaration);

// Handles the class in variable declaration.
void GenerateBytecode(std::string import_location);

// Gets the function name with scope.
std::string GetFunctionNameWithScope(Interpreter& interpreter,
                                     Ast::FunctionDeclaration* declaration);

// Handles the function arguments.
void HandleFunctionArguments(Interpreter& interpreter,
                             Ast::FunctionDeclaration* declaration,
                             std::vector<std::size_t>& parameters_index,
                             std::vector<Bytecode>& code);

// Handles the return statement.
void HandleReturnInHandlingFunction(Interpreter& interpreter,
                                    std::vector<Bytecode>& code);

// Handles the goto statement in handling function.
void HandleGotoInHandlingFunction(Interpreter& interpreter,
                                  std::size_t current_scope,
                                  std::vector<Bytecode>& code);

// Handles the return statement in handling function.
void AddFunctionIntoList(Interpreter& interpreter,
                         Ast::FunctionDeclaration* declaration,
                         std::vector<std::size_t>& parameters_index,
                         std::vector<Bytecode>& code);

// Handles the class function declaration.
void AddClassFunctionIntoList(Interpreter& interpreter,
                              Ast::FunctionDeclaration* declaration,
                              std::vector<std::size_t>& parameters_index,
                              std::vector<Bytecode>& code);

// Handles the class constructor function declaration.
void AddClassConstructorFunctionIntoList(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration,
    std::vector<std::size_t>& parameters_index, std::vector<Bytecode>& code);

// Handles the return statement in handling function.
void HandleReturnVariableInHandlingFunction(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration,
    std::string scope_name, std::vector<std::size_t>& parameters_index);

// Handles the factory function in handling constructor.
std::vector<std::size_t> HandleFactoryFunctionInHandlingConstructor(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration);

// Handles the constructor function in handling constructor.
void HandleConstructorFunctionInHandlingConstructor(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration,
    std::vector<std::size_t>& parameters_index);

// Handles the class statement.
void HandleSubClassesInHandlingClass(Interpreter& interpreter,
                                     Ast::Class* declaration);

// Handles the static members in handling class.
void HandleStaticMembersInHandlingClass(Interpreter& interpreter,
                                        Ast::Class* declaration);

// Handles the class members in handling class.
void HandleClassMembersInHandlingClass(Interpreter& interpreter,
                                       Ast::Class* declaration);

// Handles the methods in handling class.
void HandleMethodsInHandlingClass(Interpreter& interpreter,
                                  Ast::Class* declaration);

// Handles the class in handling variable.
void AddVoidConstructorInHandlingClass(Interpreter& interpreter,
                                       Ast::Class* declaration);

// Handles the void factory function in handling class.
std::vector<std::size_t> HandleVoidFactoryFunctionInHandlingClass(
    Interpreter& interpreter, Ast::Class* declaration);

// Handles the void constructor function in handling class.
void HandleVoidConstructorFunctionInHandlingClass(
    Interpreter& interpreter, Ast::Class* declaration,
    std::vector<std::size_t>& parameters_index);

// Handles the class in handling variable.
void HandleClassInHandlingVariable(Interpreter& interpreter,
                                   Ast::Variable* declaration,
                                   std::size_t variable_index,
                                   std::vector<Bytecode>& code);

// Handles the class in handling variable.
std::string GetClassNameString(Interpreter& interpreter, Ast::ClassType* type);
}  // namespace Interpreter
}  // namespace Aq

#endif