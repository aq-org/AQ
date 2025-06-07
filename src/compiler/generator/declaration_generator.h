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
std::unordered_map<std::string, Generator*> imports_map;

// Handles the import statement.
void HandleImport(Generator& generator, Ast::Import* statement);

// Handles the function declaration.
void HandleFunctionDeclaration(Generator& generator,
                               Ast::FunctionDeclaration* declaration);

void HandleClassFunctionDeclaration(Generator& generator,
                                    Ast::FunctionDeclaration* declaration);
void HandleClassConstructor(Generator& generator,
                            Ast::FunctionDeclaration* declaration);
void HandleClassDeclaration(Generator& generator, Ast::Class* declaration);
std::size_t HandleVariableDeclaration(Generator& generator,
                                      Ast::Variable* declaration);
std::size_t HandleGlobalVariableDeclaration(Generator& generator,
                                            Ast::Variable* declaration);
std::size_t HandleStaticVariableDeclaration(Generator& generator,
                                            Ast::Variable* declaration);
std::size_t HandleClassVariableDeclaration(Generator& generator,
                                           Ast::Variable* declaration);
std::size_t HandleArrayDeclaration(Generator& generator,
                                   Ast::ArrayDeclaration* declaration);
std::size_t HandleGlobalArrayDeclaration(Generator& generator,
                                         Ast::ArrayDeclaration* declaration);
std::size_t HandleStaticArrayDeclaration(Generator& generator,
                                         Ast::ArrayDeclaration* declaration);
std::size_t HandleClassArrayDeclaration(Generator& generator,
                                        Ast::ArrayDeclaration* declaration);

void GenerateBytecode(std::string import_location);
std::string GetFunctionNameWithScope(Generator& generator,
                                     Ast::FunctionDeclaration* declaration);
void HandleFunctionArguments(Generator& generator,
                             Ast::FunctionDeclaration* declaration,
                             std::vector<std::size_t>& parameters_index,
                             std::vector<Bytecode>& code);
void HandleReturnInHandlingFunction(Generator& generator,
                                    std::vector<Bytecode>& code);
void HandleGotoInHandlingFunction(Generator& generator,
                                  std::size_t current_scope,
                                  std::vector<Bytecode>& code);
void AddFunctionIntoList(Generator& generator,
                         Ast::FunctionDeclaration* declaration,
                         std::vector<std::size_t>& parameters_index,
                         std::vector<Bytecode>& code);
void AddClassFunctionIntoList(Generator& generator,
                              Ast::FunctionDeclaration* declaration,
                              std::vector<std::size_t>& parameters_index,
                              std::vector<Bytecode>& code);
void HandleReturnVariableInHandlingFunction(
    Generator& generator, Ast::FunctionDeclaration* declaration,
    std::string scope_name, std::vector<std::size_t>& parameters_index);
std::vector<std::size_t> HandleFactoryFunctionInHandlingConstructor(
    Generator& generator, Ast::FunctionDeclaration* declaration);
void HandleConstructorFunctionInHandlingConstructor(
    Generator& generator, Ast::FunctionDeclaration* declaration,
    std::vector<std::size_t>& parameters_index);
void HandleSubClassesInHandlingClass(Generator& generator,
                                     Ast::Class* declaration);
void HandleStaticMembersInHandlingClass(Generator& generator,
                                        Ast::Class* declaration);
void HandleClassMembersInHandlingClass(Generator& generator,
                                       Ast::Class* declaration);
void HandleMethodsInHandlingClass(Generator& generator,
                                  Ast::Class* declaration);
void AddVoidConstructorInHandlingClass(Generator& generator,
                                       Ast::Class* declaration);
std::vector<std::size_t> HandleVoidFactoryFunctionInHandlingClass(
    Generator& generator, Ast::Class* declaration);
void HandleVoidConstructorFunctionInHandlingClass(
    Generator& generator, Ast::Class* declaration,
    std::vector<std::size_t>& parameters_index);
void HandleClassInHandlingVariable(Generator& generator,
                                   Ast::Variable* declaration,
                                   std::size_t variable_index);
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif