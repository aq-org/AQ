// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/generator/declaration_generator.h"

#include <unordered_map>
#include <vector>

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

  Ast::Function* statement = declaration->GetFunctionStatement();
  auto arguments = statement->GetParameters();
  std::vector<Bytecode> code;

  // Handles the function name with scopes.
  std::string scope_name = GetFunctionNameWithScope(generator, declaration);
  scopes.push_back(scope_name);
  std::size_t current_scope = scopes.size() - 1;

  // DEPRECATED: Keep this implementation for adaptation to previous versions.
  // In principle, it will no longer be used.
  if (declaration->GetFunctionBody() == nullptr) {
    LOGGING_WARNING(
        "The function statement without function body has been deprecated.");
    scopes.pop_back();
    return;
  }

  // Handles function arguments and return value.
  std::vector<std::size_t> arguments_index;
  HandleReturnVariableInHandlingFunction(generator, declaration, scope_name,
                                         arguments_index);
  HandleFunctionArguments(generator, declaration, arguments_index, code);

  // Handles function body.
  HandleStatement(generator, declaration->GetFunctionBody(), code);

  // Handles jump statements.
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
  auto& label_map = generator.context.function_context->label_map;
  auto& scopes = generator.context.scopes;

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();
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

  // Adds the function into class functions map.
  current_class->GetFunctions().insert(name);

  // DEPRECATED: Keep this implementation for adaptation to previous versions.
  // In principle, it will no longer be used.
  if (declaration->GetFunctionBody() == nullptr) {
    LOGGING_WARNING(
        "The function statement without function body has been deprecated.");
    scopes.pop_back();
    return;
  }

  // Handles function arguments and return value.
  std::vector<std::size_t> arguments_index;
  HandleReturnVariableInHandlingFunction(generator, declaration, scope_name,
                                         arguments_index);
  HandleFunctionArguments(generator, declaration, arguments_index, code);

  // Handles function body.
  HandleClassStatement(generator, declaration->GetFunctionBody(), code);

  // Handles jump statements.
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

  auto arguments_index =
      HandleFactoryFunctionInHandlingConstructor(generator, declaration);
  HandleConstructorFunctionInHandlingConstructor(generator, declaration,
                                                 arguments_index);
}

void HandleClassDeclaration(Generator& generator, Ast::Class* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  auto& scopes = generator.context.scopes;
  auto& classes = generator.context.classes;
  auto& classes_list = generator.classes;
  auto& current_class = generator.context.current_class;
  auto& functions = generator.context.functions;

  std::string class_name =
      scopes.back() + "." + std::string(declaration->GetClassName());

  scopes.push_back(class_name);

  // Check if there are any errors in the preprocessing.
  if (classes.find(class_name) == classes.end())
    INTERNAL_ERROR("Not found class declaration.");
  current_class = classes[class_name];

  // Adds the special variable into class memory.
  current_class->GetMemory().Add("@name");
  current_class->GetMemory().Add("@size");

  HandleSubClassesInHandlingClass(generator, declaration);

  HandleStaticMembersInHandlingClass(generator, declaration);

  HandleClassMembersInHandlingClass(generator, declaration);

  HandleMethodsInHandlingClass(generator, declaration);

  if (functions.find(class_name) == functions.end())
    AddVoidConstructorInHandlingClass(generator, declaration);

  scopes.pop_back();
  current_class = nullptr;
  classes_list.push_back(*current_class);
}

std::size_t HandleVariableDeclaration(Generator& generator,
                                      Ast::Variable* declaration,
                                      std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  auto& memory = generator.global_memory;
  auto& scopes = generator.context.scopes;
  auto& functions = generator.context.functions;
  auto& global_code = generator.global_code;
  auto& variables = generator.context.variables;

  std::vector<uint8_t> vm_type = declaration->GetVariableType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;

  auto category = declaration->GetVariableType()->GetTypeCategory();

  // Deletes const flag if the variable type contain const.
  if (category == Ast::Type::TypeCategory::kConst) {
    vm_type.erase(vm_type.begin());
  }

  bool is_has_value = declaration->GetVariableValue()[0] != nullptr;
  if (!is_has_value) {
    std::size_t variable_index = memory.AddWithType(vm_type);

    if (category == Ast::Type::TypeCategory::kClass) {
      std::string func_name = std::string(*declaration->GetVariableType());
      for (int64_t i = scopes.size() - 1; i >= -1; i--) {
        auto iterator = functions.find(func_name);
        if (i != -1) iterator = functions.find(scopes[i] + "." + func_name);
        if (iterator != functions.end()) {
          func_name = func_name;
          if (i != -1) func_name = scopes[i] + "." + func_name;
          break;
        }
        if (i == -1) LOGGING_ERROR("Function not found.");
      }

      std::size_t reference_index = memory.Add(1);

      global_code.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, reference_index, variable_index));

      global_code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, reference_index,
                                     memory.AddByte(0),
                                     memory.AddString(func_name)));

      // Classes without initialization requires default initialization.
      global_code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4,
                                     reference_index,
                                     memory.AddString("@constructor"), 1, 0));
    }

    variables.emplace(scopes.back() + "#" + declaration->GetVariableName(),
                      variable_index);
    return variable_index;
  } else {
    std::size_t value_index =
        HandleExpr(declaration->GetVariableValue()[0], code);
    std::size_t var_index = memory.AddWithType(vm_type);
    if (category == Ast::Type::TypeCategory::kClass) {
      std::string func_name = (std::string)*declaration->GetVariableType();
      for (int64_t i = scopes.size() - 1; i >= -1; i--) {
        auto iterator = functions.find(func_name);
        if (i != -1) iterator = functions.find(scopes[i] + "." + func_name);
        if (iterator != functions.end()) {
          func_name = func_name;
          if (i != -1) func_name = scopes[i] + "." + func_name;
          break;
        }
        if (i == -1) LOGGING_ERROR("Function not found.");
      }

      std::size_t var_index_reference = memory.Add(1);

      global_code.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference, var_index));

      global_code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, var_index_reference,
                                     memory.AddByte(0),
                                     memory.AddString(func_name)));

      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    }
    if (category == Ast::Type::TypeCategory::kReference) {
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.erase(value_ptr.begin());
      value_ptr.insert(value_ptr.begin(), 0x06);
      code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, var_index, value_index));
      variables.emplace(scopes.back() + "#" + declaration->GetVariableName(),
                        var_index);
      return var_index;
    }
    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    if (category == Ast::Type::TypeCategory::kConst) {
      std::size_t const_var_index = memory.AddWithType(return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CONST, 2, const_var_index, var_index));
      variables.emplace(scopes.back() + "#" + declaration->GetVariableName(),
                        const_var_index);
      return const_var_index;
    }
    variables.emplace(scopes.back() + "#" + declaration->GetVariableName(),
                      var_index);
    return var_index;
  }
}
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

std::vector<std::size_t> HandleFactoryFunctionInHandlingConstructor(
    Generator& generator, Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  generator.context.function_context = new FunctionContext();

  auto& scopes = generator.context.scopes;
  auto& memory = generator.global_memory;
  auto& functions = generator.context.functions;

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();
  auto arguments = statement->GetParameters();
  std::vector<Bytecode> code;

  functions.insert(name);

  // Handles the return value and arguments.
  std::vector<std::size_t> arguments_index;
  std::size_t return_index = memory.Add(1);
  arguments_index.push_back(return_index);
  HandleFunctionArguments(generator, declaration, arguments_index, code);

  // Builds the main part of the factory function.
  code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, return_index,
                          memory.AddUint64t(0), memory.AddString(name)));
  std::vector<std::size_t> method_arguments = arguments_index;
  method_arguments.insert(
      method_arguments.begin(),
      {return_index, memory.AddString("@constructor"), arguments_index.size()});
  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, method_arguments));

  AddFunctionIntoList(generator, declaration, arguments_index, code);

  scopes.pop_back();
  delete generator.context.function_context;
  generator.context.function_context = nullptr;
  return arguments_index;
}

void HandleConstructorFunctionInHandlingConstructor(
    Generator& generator, Ast::FunctionDeclaration* declaration,
    std::vector<std::size_t>& arguments_index) {
  generator.context.function_context = new FunctionContext();

  auto& scopes = generator.context.scopes;
  auto& memory = generator.global_memory;
  auto& functions = generator.context.functions;
  auto& current_class = generator.context.current_class;

  std::vector<Bytecode> code;

  // Handles the function name with scopes.
  std::string scope_name = GetFunctionNameWithScope(generator, declaration);
  scopes.push_back(scope_name);
  std::size_t current_scope = scopes.size() - 1;

  current_class->GetFunctions().insert("@constructor");

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  // Handles the class init code.
  for (std::size_t i = 0; i < current_class->GetCode().size(); i++) {
    code.push_back(current_class->GetCode()[i]);
  }

  // DEPRECATED: Keep this implementation for adaptation to previous versions.
  // In principle, it will no longer be used.
  if (declaration->GetFunctionBody() == nullptr) {
    LOGGING_WARNING(
        "The function statement without function body has been deprecated.");
    scopes.pop_back();
    return;
  }

  HandleClassStatement(declaration->GetFunctionBody(), code);

  HandleReturnInHandlingFunction(generator, code);

  HandleGotoInHandlingFunction(generator, current_scope, code);

  AddClassFunctionIntoList(generator, declaration, arguments_index, code);

  scopes.pop_back();
  delete generator.context.function_context;
  generator.context.function_context = nullptr;
}

void HandleSubClassesInHandlingClass(Generator& generator,
                                     Ast::Class* declaration) {
  auto& scopes = generator.context.scopes;
  auto& classes = generator.context.classes;

  for (std::size_t i = 0; i < declaration->GetSubClasses().size(); i++) {
    if (Ast::IsOfType<Ast::Class>(declaration->GetSubClasses()[i])) {
      // Handles the sub class declaration.
      auto sub_class = Ast::Cast<Ast::Class>(declaration->GetSubClasses()[i]);
      HandleClassDeclaration(generator, sub_class);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }

  // Restores class name.
  auto& current_class = generator.context.current_class;
  std::string class_name =
      scopes.back() + "." + std::string(declaration->GetClassName());
  current_class = classes[class_name];
}

void HandleStaticMembersInHandlingClass(Generator& generator,
                                        Ast::Class* declaration) {
  for (std::size_t i = 0; i < declaration->GetStaticMembers().size(); i++) {
    auto member = declaration->GetStaticMembers()[i]->GetStaticDeclaration();
    if (Ast::IsOfType<Ast::Variable>(member)) {
      // Handles the static variable declaration.
      auto variable = Ast::Cast<Ast::Variable>(member);
      HandleStaticVariableDeclaration(generator, variable);

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(member)) {
      // Handles the static array declaration.
      auto array = Ast::Cast<Ast::ArrayDeclaration>(member);
      HandleStaticArrayDeclaration(generator, array);

    } else if (Ast::IsOfType<Ast::FunctionDeclaration>(member)) {
      // Handles the static function declaration.
      auto function = Ast::Cast<Ast::FunctionDeclaration>(member);
      HandleFunctionDeclaration(generator, function);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }
}

void HandleClassMembersInHandlingClass(Generator& generator,
                                       Ast::Class* declaration) {
  auto& current_class = generator.context.current_class;

  auto& memory = current_class->GetMemory();
  auto& variables = current_class->GetVariables();
  auto& code = current_class->GetCode();

  for (std::size_t i = 0; i < declaration->GetMembers().size(); i++) {
    auto member = dynamic_cast<Ast::Declaration*>(declaration->GetMembers()[i]);

    if (Ast::IsOfType<Ast::Variable>(member)) {
      // Handles the variable.
      auto variable = Ast::Cast<Ast::Variable>(member);
      HandleClassVariableDeclaration(generator, memory, variables, variable,
                                     code);

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(member)) {
      // Handles the array declaration.
      auto array = Ast::Cast<Ast::ArrayDeclaration>(member);
      HandleClassArrayDeclaration(generator, memory, variables, array, code);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }
}

void HandleMethodsInHandlingClass(Generator& generator,
                                  Ast::Class* declaration) {
  for (std::size_t i = 0; i < declaration->GetMethods().size(); i++) {
    if (Ast::IsOfType<Ast::FunctionDeclaration>(declaration->GetMethods()[i])) {
      // Handles the class function declaration.
      auto function =
          Ast::Cast<Ast::FunctionDeclaration>(declaration->GetMethods()[i]);
      HandleClassFunctionDeclaration(generator, function);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }
}

void AddVoidConstructorInHandlingClass(Generator& generator,
                                       Ast::Class* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  auto arguments_index =
      HandleVoidFactoryFunctionInHandlingClass(generator, declaration);

  HandleVoidConstructorFunctionInHandlingClass(generator, declaration,
                                               arguments_index);
}
std::vector<std::size_t> HandleVoidFactoryFunctionInHandlingClass(
    Generator& generator, Ast::Class* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  generator.context.function_context = new FunctionContext();

  auto& scopes = generator.context.scopes;
  auto& functions = generator.context.functions;
  auto& current_class = generator.context.current_class;
  auto& goto_map = generator.context.function_context->goto_map;
  auto& variables = generator.context.variables;
  auto& memory = generator.global_memory;
  auto& function_list = generator.functions;
  auto& exit_index = generator.context.function_context->exit_index;

  std::string name = scopes.back();
  std::string class_name = declaration->GetClassName();

  // Handles the arguments.
  std::vector<std::size_t> arguments_index;
  std::size_t return_index = memory.Add(1);
  arguments_index.push_back(return_index);

  functions.insert(name);

  // Builds the main part of the factory function.
  std::vector<Bytecode> code;
  code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, return_index,
                          memory.AddUint64t(0), memory.AddString(class_name)));
  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, return_index,
                          memory.AddString("@constructor"), 1, return_index));

  Function factory(name, arguments_index, code);
  function_list.push_back(factory);

  delete generator.context.function_context;
  generator.context.function_context = nullptr;
  return arguments_index;
}
void HandleVoidConstructorFunctionInHandlingClass(
    Generator& generator, Ast::Class* declaration,
    std::vector<std::size_t>& arguments_index) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  auto& current_class = generator.context.current_class;

  current_class->GetFunctions().insert("@constructor");

  std::vector<Bytecode> code;
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  Function constructor("@constructor", arguments_index, code);
  current_class->GetFunctionList().push_back(constructor);

  delete generator.context.function_context;
  generator.context.function_context = nullptr;
}
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq