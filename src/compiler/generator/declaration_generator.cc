// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/generator/declaration_generator.h"

#include <unordered_map>

#include "compiler/ast/ast.h"
#include "compiler/compiler.h"
#include "compiler/generator/bytecode.h"
#include "compiler/generator/memory.h"
#include "compiler/logging/logging.h"
#include "compiler/parser/parser.h"

namespace Aq {
namespace Compiler {
namespace Generator {
void HandleImport(Generator& generator, Ast::Import* statement) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // TODO: Handles from import.
  if (statement->IsFromImport()) INTERNAL_ERROR("Unsupported import type now.");

  // Gets the reference of context.
  auto& main_class = generator.main_class;
  auto& memory = generator.global_memory;
  auto& variables = generator.context.variables;
  auto& init_code = generator.init_code;

  // Gets the information from the import statement.
  std::string location = statement->GetImportLocation();
  std::string name = statement->GetName();

  // Generates bytecode if the bytecode isn't generated yet.
  if (imports_map.find(location) == imports_map.end()) {
    GenerateBytecode(location);
  }

  // Checks if the import is already imported.
  if (imports_map.find(name) != imports_map.end())
    LOGGING_ERROR("Has same name bytecode file.");
  imports_map[name] = imports_map[location];

  // Gets index from import preprocessing.
  std::size_t index = variables["#" + name];

  // Adds the import bytecode into the main class memory and variables.
  main_class.GetVariables()[name] = main_class.GetMemory().Add(name);

  // Loads and initializes the import bytecode.
  init_code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 2,
                               memory.AddString(name)));
  init_code.push_back(
      Bytecode(_AQVM_OPERATOR_NEW, 3, index, memory.AddUint64t(0),
               memory.AddString("~" + location + "bc~.!__start")));
}

void HandleFunctionDeclaration(Generator& generator,
                               Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  generator.context.function_context = new FunctionContext();

  auto& scopes = generator.context.scopes;
  auto& memory = generator.global_memory;
  auto& function_list = generator.functions;
  auto& goto_map = generator.context.function_context->goto_map;
  auto& exit_index = generator.context.function_context->exit_index;
  auto& label_map = generator.context.function_context->label_map;
  auto& variables = generator.context.variables;

  Ast::Function* statement = declaration->GetFunctionStatement();
  auto arguments = statement->GetParameters();
  std::vector<Bytecode> code;

  // Handles the function name with scopes.
  std::string scope_name = GetFunctionNameWithScope(generator, declaration);
  scopes.push_back(scope_name);
  std::size_t current_scope = scopes.size() - 1;

  // Handles function statement without function body.
  if (declaration->GetFunctionBody() == nullptr) {
    LOGGING_WARNING(
        "The function statement without function body has been deprecated.");
    scopes.pop_back();
    return;
  }

  std::vector<std::size_t> arguments_index;

  HandleReturnVariableInHandlingFunction(generator, declaration, scope_name,
                                         arguments_index);

  HandleFunctionArguments(generator, declaration, arguments_index, code);

  HandleStatement(generator, declaration->GetFunctionBody(), code);

  HandleReturnInHandlingFunction(generator, code);

  HandleGotoInHandlingFunction(generator, current_scope, code);

  AddFunctionIntoList(generator, declaration, arguments_index, code);

  scopes.pop_back();
  delete generator.context.function_context;
  generator.context.function_context = nullptr;
}

void HandleClassFunctionDeclaration(Generator& generator,
                                    Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  generator.context.function_context = new FunctionContext();

  auto& current_class = generator.context.current_class;
  auto& goto_map = generator.context.function_context->goto_map;
  auto& exit_index = generator.context.function_context->exit_index;
  auto& label_map = generator.context.function_context->label_map;
  auto& scopes = generator.context.scopes;
  auto& memory = generator.global_memory;
  auto& variables = generator.context.variables;

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();
  auto arguments = statement->GetParameters();
  std::vector<Bytecode> code;

  // Handles the class constructor if this function is a constructor function.
  if (std::string(current_class->GetClassDeclaration()->GetClassName()) ==
      name) {
    HandleClassConstructor(generator, declaration);
    return;
  }

  // Handles the function name with scopes.
  std::string scope_name = GetFunctionNameWithScope(generator, declaration);
  scopes.push_back(scope_name);
  std::size_t current_scope = scopes.size() - 1;

  current_class->GetFunctions().insert(name);

  if (declaration->GetFunctionBody() == nullptr) {
    LOGGING_WARNING(
        "The function statement without function body has been deprecated.");
    scopes.pop_back();
    return;
  }

  std::vector<std::size_t> arguments_index;

  HandleReturnVariableInHandlingFunction(generator, declaration, scope_name,
                                         arguments_index);

  HandleFunctionArguments(generator, declaration, arguments_index, code);

  HandleClassStatement(generator, declaration->GetFunctionBody(), code);

  HandleReturnInHandlingFunction(generator, code);

  HandleGotoInHandlingFunction(generator, current_scope, code);

  AddClassFunctionIntoList(generator, declaration, arguments_index, code);

  scopes.pop_back();
  delete generator.context.function_context;
  generator.context.function_context = nullptr;
}

void HandleClassConstructor(Generator& generator,
                            Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  generator.context.function_context = new FunctionContext();

  auto& current_class = generator.context.current_class;
  auto& goto_map = generator.context.function_context->goto_map;
  auto& exit_index = generator.context.function_context->exit_index;
  auto& label_map = generator.context.function_context->label_map;
  auto& scopes = generator.context.scopes;
  auto& memory = generator.global_memory;
  auto& variables = generator.context.variables;

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();
  auto arguments = statement->GetParameters();
  std::vector<Bytecode> code;

  // Handles the class constructor if this function is a constructor function.
  if (std::string(current_class->GetClassDeclaration()->GetClassName()) ==
      name) {
    HandleClassConstructor(generator, declaration);
    return;
  }

  // Handles the function name with scopes.
  std::string scope_name = GetFunctionNameWithScope(generator, declaration);
  scopes.push_back(scope_name);
  std::size_t current_scope = scopes.size() - 1;

  current_class->GetFunctions().insert(name);

  if (declaration->GetFunctionBody() == nullptr) {
    LOGGING_WARNING(
        "The function statement without function body has been deprecated.");
    scopes.pop_back();
    return;
  }

  std::vector<std::size_t> arguments_index;

  std::size_t return_value_index = memory.Add(1);

  HandleFunctionArguments(generator, declaration, arguments_index, code);

  code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, return_value_index,
                          memory.AddUint64t(0), memory.AddString(name)));

  current_class->GetFunctions().insert("@constructor");

  std::vector<std::size_t> invoke_class_args;
  invoke_class_args.push_back(return_value_index);
  invoke_class_args.push_back(memory.AddString("@constructor"));
  invoke_class_args.push_back(arguments_index.size());
  invoke_class_args.push_back(memory.Add(1));
  invoke_class_args.insert(invoke_class_args.end(), arguments_index.begin() + 1,
                           arguments_index.end());
  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_class_args));

  AddClassFunctionIntoList(generator, declaration, arguments_index, code);

  scopes.pop_back();
  code.clear();

  scopes.push_back(scope_name);
  current_scope = scopes.size() - 1;

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  for (std::size_t i = 0; i < current_class->GetCode().size(); i++) {
    code.push_back(current_class->GetCode()[i]);
  }

  HandleClassStmt(declaration->GetFunctionBody(), code);

  HandleReturnInHandlingFunction(generator, code);

  HandleGotoInHandlingFunction(generator, current_scope, code);

  AddClassFunctionIntoList(generator, declaration, arguments_index, code);

  scopes.pop_back();
  delete generator.context.function_context;
  generator.context.function_context = nullptr;
}

void HandleClassDeclaration(Ast::Class* declaration) {}
std::size_t HandleVariableDeclaration(Ast::Variable* declaration,
                                      std::vector<Bytecode>& code) {}
std::size_t HandleGlobalVariableDeclaration(Ast::Variable* declaration,
                                            std::vector<Bytecode>& code) {}
std::size_t HandleStaticVariableDeclaration(Ast::Variable* declaration,
                                            std::vector<Bytecode>& code) {}
std::size_t HandleClassVariableDeclaration(
    ClassMemory& memory,
    std::unordered_map<std::string, std::size_t>& variable_declaration_map,
    Ast::Variable* declaration, std::vector<Bytecode>& code) {}
std::size_t HandleArrayDeclaration(Ast::ArrayDeclaration* declaration,
                                   std::vector<Bytecode>& code) {}
std::size_t HandleGlobalArrayDeclaration(Ast::ArrayDeclaration* declaration,
                                         std::vector<Bytecode>& code) {}
std::size_t HandleStaticArrayDeclaration(Ast::ArrayDeclaration* declaration,
                                         std::vector<Bytecode>& code) {}
std::size_t HandleClassArrayDeclaration(
    ClassMemory& memory,
    std::unordered_map<std::string, std::size_t>& variable_declaration_map,
    Ast::ArrayDeclaration* declaration, std::vector<Bytecode>& code) {}

void GenerateBytecode(std::string import_location) {
  std::vector<char> code;
  Aq::Compiler::ReadCodeFromFile(import_location.c_str(), code);

  std::vector<Aq::Compiler::Token> token;
  Aq::Compiler::LexCode(code, token);

  Aq::Compiler::Ast::Compound* ast = Aq::Compiler::Parser::Parse(token);
  if (ast == nullptr) Aq::Compiler::LOGGING_ERROR("ast is nullptr.");

  Aq::Compiler::Generator::Generator generator;
  import_location = import_location + "bc";
  generator.Generate(ast, import_location.c_str());

  Aq::Compiler::LOGGING_INFO("Generate Bytecode SUCCESS!");
}

std::string GetFunctionNameWithScope(Generator& generator,
                                     Ast::FunctionDeclaration* declaration) {
  auto& scopes = generator.context.scopes;

  // Gets the function statement and its parameters.
  Ast::Function* statement = declaration->GetFunctionStatement();
  std::vector<Ast::Expression*> arguments = statement->GetParameters();

  // Gets the function name with scopes.
  std::string scope_name = scopes.back() + "." + statement->GetFunctionName();

  // Adds the function arguments type into the scope name.
  for (std::size_t i = 0; i < arguments.size(); i++) {
    // Adds the argument type separator to the scope name.
    if (i == 0) {
      scope_name += "@";
    } else {
      scope_name += ",";
    }

    // Adds the argument type to the scope name.
    if (Ast::IsOfType<Ast::Variable>(arguments[i])) {
      auto declaration = Ast::Cast<Ast::Variable>(arguments[i]);
      scope_name += std::string(*declaration->GetVariableType());

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(arguments[i])) {
      auto declaration = Ast::Cast<Ast::ArrayDeclaration>(arguments[i]);
      scope_name += std::string(*declaration->GetVariableType());

    } else {
      LOGGING_ERROR(
          "Function arguments is not a variable or array declaration.");
    }

    // Handles variadic arguments.
    if (i == arguments.size() - 1 && statement->IsVariadic()) {
      scope_name += ",...";
    }
  }

  // Handles variadic arguments if the function is variadic and arguments size
  // is 0.
  if (arguments.size() == 0 && statement->IsVariadic()) {
    scope_name += "@...";
  }

  return scope_name;
}

void HandleFunctionArguments(Generator& generator,
                             Ast::FunctionDeclaration* declaration,
                             std::vector<std::size_t>& arguments_index,
                             std::vector<Bytecode>& code) {
  auto& scopes = generator.context.scopes;
  auto& memory = generator.global_memory;
  auto& variables = generator.context.variables;

  // Gets the function statement and its parameters.
  Ast::Function* statement = declaration->GetFunctionStatement();
  auto arguments = statement->GetParameters();

  // Handles functions that only contain variable parameters.
  if (arguments.size() == 0 && statement->IsVariadic()) {
    std::size_t index = memory.Add(1);

    // Adds index into |arguments_index| and |variables|.
    arguments_index.push_back(index);
    variables[scopes.back() + "#args"] = index;
    return;
  }

  // Handles function arguments.
  for (std::size_t i = 0; i < arguments.size(); i++) {
    if (Ast::IsOfType<Ast::Variable>(arguments[i])) {
      // Gets function declaration and name.
      auto declaration = Ast::Cast<Ast::Variable>(arguments[i]);
      std::string name = scopes.back() + "#" + declaration->GetVariableName();

      std::size_t index = HandleVariableDeclaration(declaration, code);

      // Adds index into |arguments_index| and |variables|.
      arguments_index.push_back(index);
      variables[name] = index;

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(arguments[i])) {
      // Gets function declaration and name.
      auto declaration = Ast::Cast<Ast::ArrayDeclaration>(arguments[i]);
      std::string name = scopes.back() + "#" + declaration->GetVariableName();

      std::size_t index = HandleArrayDeclaration(declaration, code);

      // Adds index into |arguments_index| and |variables|.
      arguments_index.push_back(index);
      variables[name] = index;

    } else {
      INTERNAL_ERROR(
          "Function arguments is not a variable or array declaration.");
    }

    // Handles variable parameters if have.
    if (i == arguments.size() - 1 && statement->IsVariadic()) {
      std::size_t index = memory.Add(1);

      // Adds index into |arguments_index| and |variables|.
      arguments_index.push_back(index);
      variables[scopes.back() + "#args"] = index;
    }
  }
}

void HandleReturnInHandlingFunction(Generator& generator,
                                    std::vector<Bytecode>& code) {
  auto& exit_index = generator.context.function_context->exit_index;
  auto& memory = generator.global_memory;

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t return_location = code.size();
  for (std::size_t i = 0; i < exit_index.size(); i++) {
    code[exit_index[i]].SetArgs(1, memory.AddUint64t(return_location));
  }
}

void HandleGotoInHandlingFunction(Generator& generator,
                                  std::size_t current_scope,
                                  std::vector<Bytecode>& code) {
  auto& goto_map = generator.context.function_context->goto_map;
  auto& memory = generator.global_memory;
  auto& scopes = generator.context.scopes;
  auto& label_map = generator.context.function_context->label_map;

  while (goto_map.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = scopes.size() - 1; i >= current_scope; i--) {
      auto iterator = label_map.find(scopes[i] + "$" + goto_map.back().first);
      if (iterator != label_map.end()) {
        goto_location = iterator->second;
        break;
      }
      if (i == current_scope) LOGGING_ERROR("Label not found.");
    }

    code[goto_map.back().second].SetArgs(1, memory.AddUint64t(goto_location));
    goto_map.pop_back();
  }
}

void AddFunctionIntoList(Generator& generator,
                         Ast::FunctionDeclaration* declaration,
                         std::vector<std::size_t>& arguments_index,
                         std::vector<Bytecode>& code) {
  auto& function_list = generator.functions;

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();

  Function function(name, arguments_index, code);
  if (statement->IsVariadic()) function.EnableVariadic();
  function_list.push_back(function);
}

void AddClassFunctionIntoList(Generator& generator,
                              Ast::FunctionDeclaration* declaration,
                              std::vector<std::size_t>& arguments_index,
                              std::vector<Bytecode>& code) {
  auto& function_list = generator.context.current_class->GetFunctionList();

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();

  Function function(name, arguments_index, code);
  if (statement->IsVariadic()) function.EnableVariadic();
  function_list.push_back(function);
}

void HandleReturnVariableInHandlingFunction(
    Generator& generator, Ast::FunctionDeclaration* declaration,
    std::string scope_name, std::vector<std::size_t>& arguments_index) {
  auto& variables = generator.context.variables;
  auto& memory = generator.global_memory;

  std::vector<uint8_t> vm_type = declaration->GetReturnType()->GetVmType();
  variables[scope_name + "#!return"] = memory.AddWithType(vm_type);
  variables[scope_name + "#!return_reference"] = memory.Add(1);
  arguments_index.push_back(variables[scope_name + "#!return_reference"]);
}
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq