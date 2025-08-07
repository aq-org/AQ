// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/declaration_interpreter.h"

#include <unordered_map>
#include <vector>

#include "aq.h"
#include "ast/ast.h"
#include "ast/type.h"
#include "interpreter/bytecode.h"
#include "interpreter/expression_interpreter.h"
#include "interpreter/interpreter.h"
#include "interpreter/memory.h"
#include "interpreter/operator.h"
#include "interpreter/statement_interpreter.h"
#include "logging/logging.h"
#include "operator.h"
#include "parser/parser.h"

namespace Aq {
namespace Interpreter {
std::unordered_map<std::string, Interpreter*> imports_map;

void HandleImport(Interpreter& interpreter, Ast::Import* statement) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // TODO: Handles from import.
  if (statement->IsFromImport()) INTERNAL_ERROR("Unsupported import type now.");

  // Gets the reference of context.
  auto& main_class = interpreter.main_class;
  auto& memory = interpreter.global_memory;
  auto& variables = interpreter.context.variables;
  auto& init_code = interpreter.init_code;

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

  // Loads and initializes the import bytecode.
  init_code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 2,
                               memory->AddString(name)));
  init_code.push_back(
      Bytecode(_AQVM_OPERATOR_NEW, 3, index, memory->AddUint64t(0),
               memory->AddString("~" + location + "bc~.!__start")));
}

void HandleFunctionDeclaration(Interpreter& interpreter,
                               Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  interpreter.context.function_context = new FunctionContext();

  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& current_scope = interpreter.context.function_context->current_scope;

  // Gets the function statement and its parameters.
  Ast::Function* statement = declaration->GetFunctionStatement();
  auto parameters = statement->GetParameters();
  std::vector<Bytecode> code;

  // Handles the function name with scopes.
  std::string scope_name = GetFunctionNameWithScope(interpreter, declaration);
  scopes.push_back(scope_name);
  current_scope = scopes.size() - 1;

  // DEPRECATED: Keep this implementation for adaptation to previous versions.
  // In principle, it will no longer be used.
  if (declaration->GetFunctionBody() == nullptr) {
    LOGGING_WARNING(
        "The function statement without function body has been deprecated.");
    scopes.pop_back();
    return;
  }

  // Handles function parameters and return value.
  std::vector<std::size_t> parameters_index;
  HandleReturnVariableInHandlingFunction(interpreter, declaration, scope_name,
                                         parameters_index);
  HandleFunctionArguments(interpreter, declaration, parameters_index, code);

  LOGGING_INFO("Handling function body: " +
               std::string(statement->GetFunctionName()));

  // Handles function body.
  HandleStatement(interpreter, declaration->GetFunctionBody(), code);

  // Handles jump statements.
  HandleReturnInHandlingFunction(interpreter, code);
  HandleGotoInHandlingFunction(interpreter, current_scope, code);

  AddFunctionIntoList(interpreter, declaration, scope_name, parameters_index,
                      code);

  LOGGING_INFO("Function " + scope_name +
               " ,args size: " + std::to_string(parameters_index.size()));

  LOGGING_INFO(
      "Main function args size: " +
      std::to_string(
          interpreter.functions[".main"].back().GetParameters().size()));

  // Destroys temporary context.
  scopes.pop_back();
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;
}

void HandleClassFunctionDeclaration(Interpreter& interpreter,
                                    Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Creates temporary context.
  interpreter.context.function_context = new FunctionContext();

  // Gets the reference of context.
  auto& current_class = interpreter.context.current_class;
  auto& label_map = interpreter.context.function_context->label_map;
  auto& scopes = interpreter.context.scopes;
  auto& current_scope = interpreter.context.function_context->current_scope;

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();
  std::vector<Bytecode> code;

  // Handles the class constructor if this function is a constructor function.
  if (std::string(current_class->GetClassDeclaration()->GetClassName()) ==
      name) {
    // LOGGING_INFO("Handling class constructor AAAAAAAAAAAAAAAAAAAAAAAAAAAA: "
    // +
    //   std::string(current_class->GetClassDeclaration()->GetClassName()));
    HandleClassConstructor(interpreter, declaration);
    return;
  } else {
    // LOGGING_INFO("Handling class function: " + name);
    // LOGGING_INFO("Class: " +
    //     std::string(current_class->GetClassDeclaration()->GetClassName()));
  }

  // Handles the function name with scopes.
  std::string scope_name = GetFunctionNameWithScope(interpreter, declaration);
  scopes.push_back(scope_name);
  current_scope = scopes.size() - 1;

  // Records the creation of the function.
  current_class->GetMethods()[name].push_back(Function());

  // DEPRECATED: Keep this implementation for adaptation to previous versions.
  // In principle, it will no longer be used.
  if (declaration->GetFunctionBody() == nullptr) {
    LOGGING_WARNING(
        "The function statement without function body has been deprecated.");
    scopes.pop_back();
    return;
  }

  // Handles function parameters and return value.
  std::vector<std::size_t> parameters_index;
  HandleReturnVariableInHandlingFunction(interpreter, declaration, scope_name,
                                         parameters_index);
  HandleFunctionArguments(interpreter, declaration, parameters_index, code);

  // Handles function body.
  HandleClassStatement(interpreter, declaration->GetFunctionBody(), code);

  // Handles jump statements.
  HandleReturnInHandlingFunction(interpreter, code);
  HandleGotoInHandlingFunction(interpreter, current_scope, code);

  AddClassFunctionIntoList(interpreter, declaration, parameters_index, code);

  LOGGING_INFO("Function " + name +
               " ,args size: " + std::to_string(parameters_index.size()));

  LOGGING_INFO(
      "Main function args size: " +
      std::to_string(
          interpreter.functions[".main"].back().GetParameters().size()));

  // Destroys temporary context.
  scopes.pop_back();
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;
}

void HandleClassConstructor(Interpreter& interpreter,
                            Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  auto parameters_index =
      HandleFactoryFunctionInHandlingConstructor(interpreter, declaration);

  HandleConstructorFunctionInHandlingConstructor(interpreter, declaration,
                                                 parameters_index);
}

void HandleClassDeclaration(Interpreter& interpreter, Ast::Class* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& classes = interpreter.classes;
  auto& current_class = interpreter.context.current_class;
  auto& functions = interpreter.functions;

  std::string class_name =
      scopes.back() + "." + std::string(declaration->GetClassName());

  // Adds the class name into scopes.
  scopes.push_back(class_name);

  // Check if there are any errors in the preprocessing.
  if (classes.find(class_name) == classes.end())
    INTERNAL_ERROR("Not found class declaration.");
  current_class = &classes[class_name];

  // Adds the special variable into class memory.
  current_class->GetMembers()->AddString("@name", class_name);

  HandleSubClassesInHandlingClass(interpreter, declaration);

  // Restores function context null caused by possible sub classes generation.
  current_class = &classes[class_name];

  HandleStaticMembersInHandlingClass(interpreter, declaration);

  HandleClassMembersInHandlingClass(interpreter, declaration);

  HandleMethodsInHandlingClass(interpreter, declaration);

  // Adds the void default constructor if the class doesn't have a constructor.
  if (functions.find(class_name) == functions.end())
    AddVoidConstructorInHandlingClass(interpreter, declaration);

  // Destroys temporary context.
  if (scopes.size() <= 0) {
    LOGGING_WARNING(
        "Scopes size is 0, this may cause unexpected behavior. "
        "Please check the class declaration.");
  } else {
    scopes.pop_back();
  }
  current_class = nullptr;
}

std::size_t HandleVariableDeclaration(Interpreter& interpreter,
                                      Ast::Variable* declaration,
                                      std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;
  auto& variables = interpreter.context.variables;

  // For non const types, |return_type| is equivalent to |vm_type|, but for
  // const types, |vm_type| is the internal variable type excluding the const
  // wrapper, and |return_type| is the final returned variable type.
  std::vector<uint8_t> vm_type = declaration->GetVariableType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;

  auto category = declaration->GetVariableType()->GetTypeCategory();

  // Deletes const flag if the variable type contain const.
  // if (category == Ast::Type::TypeCategory::kConst)
  //   vm_type.erase(vm_type.begin());

  std::string variable_name =
      scopes.back() + "#" + declaration->GetVariableName();

  std::size_t variable_index = 0;
  if (category == Ast::Type::TypeCategory::kConst) {
    variable_index = memory->AddWithType(vm_type, true);
    category = dynamic_cast<Ast::ConstType*>(declaration->GetVariableType())
                   ->GetSubType()
                   ->GetTypeCategory();
  } else {
    variable_index = memory->AddWithType(vm_type);
  }

  // If the variable value isn't nullptr, it means that the variable is
  // initialized.
  if (declaration->GetVariableValue()[0] != nullptr) {
    std::size_t value_index =
        HandleExpression(interpreter, declaration->GetVariableValue()[0], code);

    // If the variable is a reference type, it needs to be handled
    // specially.
    if (category == Ast::Type::TypeCategory::kReference) {
      code.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, variable_index, value_index));
    } else {
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, variable_index, value_index));
    }

  } else if (category == Ast::Type::TypeCategory::kReference) {
    // If the variable is a reference type and not initialized, it will meet
    // undefined behavior.
    LOGGING_WARNING(
        "The reference variable declaration without initialization is "
        "deprecated.");

  } else if (category == Ast::Type::TypeCategory::kClass) {
    HandleClassInHandlingVariable(interpreter, declaration, variable_index,
                                  code);
  }

  variables[variable_name] = variable_index;
  return variable_index;
}

std::size_t HandleGlobalVariableDeclaration(Interpreter& interpreter,
                                            Ast::Variable* declaration,
                                            std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& start_class = interpreter.main_class;
  auto& variables = interpreter.context.variables;
  auto memory = start_class.GetMembers();
  auto& global_memory = interpreter.global_memory;

  // For non const types, |return_type| is equivalent to |vm_type|, but for
  // const types, |vm_type| is the internal variable type excluding the const
  // wrapper, and |return_type| is the final returned variable type.
  std::vector<uint8_t> vm_type = declaration->GetVariableType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;

  auto category = declaration->GetVariableType()->GetTypeCategory();

  // Deletes const flag if the variable type contain const.
  // if (category == Ast::Type::TypeCategory::kConst)
  //   vm_type.erase(vm_type.begin());

  std::string variable_name = declaration->GetVariableName();

  Object temp;
  std::reference_wrapper<Object> object = temp;

  if (category == Ast::Type::TypeCategory::kConst) {
    // If the variable is a const type, it needs to be handled specially.
    memory->AddWithType(variable_name, return_type, true);
    category = dynamic_cast<Ast::ConstType*>(declaration->GetVariableType())
                   ->GetSubType()
                   ->GetTypeCategory();

  } else {
    memory->AddWithType(variable_name, return_type);
  }

  std::size_t reference_index =
      global_memory->AddReference(memory, variable_name);

  // If the variable value isn't nullptr, it means that the variable is
  // initialized.
  if (declaration->GetVariableValue()[0] != nullptr) {
    std::size_t value_index =
        HandleExpression(interpreter, declaration->GetVariableValue()[0], code);

    // If the variable is a reference type, it needs to be handled
    // specially.
    if (category == Ast::Type::TypeCategory::kReference) {
      code.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, reference_index, value_index));
    } else {
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, reference_index, value_index));
    }

  } else if (category == Ast::Type::TypeCategory::kReference) {
    // If the variable is a reference type and not initialized, it will meet
    // undefined behavior.
    LOGGING_WARNING(
        "The reference variable declaration without initialization is "
        "deprecated.");

  } else if (category == Ast::Type::TypeCategory::kClass) {
    HandleClassInHandlingVariable(interpreter, declaration, reference_index,
                                  code);
  }

  variables[variable_name] = reference_index;

  return reference_index;
}

std::size_t HandleStaticVariableDeclaration(Interpreter& interpreter,
                                            Ast::Variable* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& global_memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;
  auto& global_code = interpreter.global_code;
  auto& variables = interpreter.context.variables;

  // For non const types, |return_type| is equivalent to |vm_type|, but for
  // const types, |vm_type| is the internal variable type excluding the const
  // wrapper, and |return_type| is the final returned variable type.
  std::vector<uint8_t> vm_type = declaration->GetVariableType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;

  auto category = declaration->GetVariableType()->GetTypeCategory();

  // Deletes const flag if the variable type contain const.
  // if (category == Ast::Type::TypeCategory::kConst)
  //   vm_type.erase(vm_type.begin());

  std::string variable_name =
      scopes.back() + "." + declaration->GetVariableName();

  std::size_t variable_index = 0;
  if (category == Ast::Type::TypeCategory::kConst) {
    variable_index = global_memory->AddWithType(vm_type, true);
    category = dynamic_cast<Ast::ConstType*>(declaration->GetVariableType())
                   ->GetSubType()
                   ->GetTypeCategory();
  } else {
    variable_index = global_memory->AddWithType(vm_type);
  }

  // If the variable is a class type, it needs to be handled specially.
  if (category == Ast::Type::TypeCategory::kClass)
    HandleClassInHandlingVariable(interpreter, declaration, variable_index,
                                  global_code);

  // If the variable value isn't nullptr, it means that the variable is
  // initialized.
  if (declaration->GetVariableValue()[0] != nullptr) {
    std::size_t value_index = HandleExpression(
        interpreter, declaration->GetVariableValue()[0], global_code);

    // If the variable is a reference type, it needs to be handled
    // specially.
    if (category == Ast::Type::TypeCategory::kReference) {
      global_code.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, variable_index, value_index));
    } else {
      global_code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, variable_index, value_index));
    }

  } else if (category == Ast::Type::TypeCategory::kReference) {
    // If the variable is a reference type and not initialized, it will meet
    // undefined behavior.
    LOGGING_WARNING(
        "The reference variable declaration without initialization is "
        "deprecated.");
  }

  variables[variable_name] = variable_index;
  return variable_index;
}

std::size_t HandleClassVariableDeclaration(Interpreter& interpreter,
                                           Ast::Variable* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& start_class = interpreter.main_class;
  auto& global_memory = interpreter.global_memory;
  auto& current_class = interpreter.context.current_class;
  auto memory = current_class->GetMembers();
  auto& code = current_class->GetCode();

  // For non const types, |return_type| is equivalent to |vm_type|, but for
  // const types, |vm_type| is the internal variable type excluding the const
  // wrapper, and |return_type| is the final returned variable type.
  std::vector<uint8_t> vm_type = declaration->GetVariableType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;

  auto category = declaration->GetVariableType()->GetTypeCategory();

  // Deletes const flag if the variable type contain const.
  // if (category == Ast::Type::TypeCategory::kConst)
  //   vm_type.erase(vm_type.begin());

  std::string variable_name = declaration->GetVariableName();

  Object temp;
  std::reference_wrapper<Object> object = temp;
  if (category == Ast::Type::TypeCategory::kConst) {
    // If the variable is a const type, it needs to be handled specially.
    memory->AddWithType(variable_name, return_type, true);
    category = dynamic_cast<Ast::ConstType*>(declaration->GetVariableType())
                   ->GetSubType()
                   ->GetTypeCategory();

  } else {
    memory->AddWithType(variable_name, return_type);
  }
  std::size_t reference_index = global_memory->Add(1);
  code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, reference_index, 2,
                          global_memory->AddString(variable_name)));

  // If the variable is a class type, it needs to be handled specially.
  if (category == Ast::Type::TypeCategory::kClass)
    HandleClassInHandlingVariable(interpreter, declaration, reference_index,
                                  code);

  // If the variable value isn't nullptr, it means that the variable is
  // initialized.
  if (declaration->GetVariableValue()[0] != nullptr) {
    std::size_t value_index =
        HandleExpression(interpreter, declaration->GetVariableValue()[0], code);

    // If the variable is a reference type, it needs to be handled
    // specially.
    if (category == Ast::Type::TypeCategory::kReference) {
      code.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, reference_index, value_index));
    } else {
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, reference_index, value_index));
    }

  } else if (category == Ast::Type::TypeCategory::kReference) {
    // If the variable is a reference type and not initialized, it will meet
    // undefined behavior.
    LOGGING_WARNING(
        "The reference variable declaration without initialization is "
        "deprecated.");
  }

  return reference_index;
}
std::size_t HandleArrayDeclaration(Interpreter& interpreter,
                                   Ast::ArrayDeclaration* declaration,
                                   std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto global_memory = interpreter.global_memory;
  auto& variables = interpreter.context.variables;
  auto& scopes = interpreter.context.scopes;

  // Handles the array type.
  Ast::ArrayType* array_type =
      dynamic_cast<Ast::ArrayType*>(declaration->GetVariableType());
  if (array_type == nullptr) INTERNAL_ERROR("array_type is nullptr.");
  if (array_type->GetTypeCategory() == Ast::Type::TypeCategory::kConst)
    LOGGING_ERROR("const array not support.");

  std::string variable_name =
      scopes.back() + "#" + declaration->GetVariableName();

  // Adds the array index and the type index.
  std::size_t array_index = global_memory->AddWithType(array_type->GetVmType());
  std::size_t array_type_index = 0;

  // Gets the sub type of the array type and its category.
  Ast::Type* sub_type = array_type->GetSubType();
  auto sub_type_category = sub_type->GetTypeCategory();

  // If the sub type is a class type, it needs to be handled specially.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::string class_name = GetClassNameString(
        interpreter, dynamic_cast<Ast::ClassType*>(sub_type));
    array_type_index = global_memory->AddString(class_name);

  } else {
    // If the vm type of the sub type isn't 0x00 (auto type), it means that
    // the sub type is a primitive type, so we can add it into the global
    // memory.
    if (sub_type->GetVmType()[0] != 0x00)
      array_type_index = global_memory->AddWithType(sub_type->GetVmType());
  }

  // Handles the array creation bytecode.
  // The array is created with the size of 1, and the type of the sub type.
  // This means that regardless of the size of the array definition, it is
  // actually determined based on the actual number of initialization lists
  // given.
  code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                          global_memory->AddByte(1), array_type_index));

  // If the sub type is a class type, it needs to be handled specially. Because
  // the default generated class index is considered an initialized value
  // because it is smaller than the array size, it will not be automatically
  // initialized when the ARRAY operator is called.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::size_t current_index = global_memory->Add(1);
    code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, current_index, 2,
                            global_memory->AddString(variable_name)));
    code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, current_index,
                            global_memory->AddString("@constructor"), 1, 0));
  }

  // Handles the array initialization with the initialization lists.
  if (!declaration->GetVariableValue().empty()) {
    for (std::size_t i = 0; i < declaration->GetVariableValue().size(); i++) {
      // Gets the corresponding array index reference.
      std::size_t current_index = global_memory->Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, current_index, 2,
                              global_memory->AddString(variable_name)));

      // Gets the value of the initialization list and assigns value to
      // corresponding index.
      std::size_t value_index = HandleExpression(
          interpreter, declaration->GetVariableValue()[i], code);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }
  }

  variables[variable_name] = array_index;
  return array_index;
}

std::size_t HandleGlobalArrayDeclaration(Interpreter& interpreter,
                                         Ast::ArrayDeclaration* declaration,
                                         std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& global_memory = interpreter.global_memory;
  auto& variables = interpreter.context.variables;
  auto& start_class = interpreter.main_class;
  auto& scopes = interpreter.context.scopes;
  auto memory = start_class.GetMembers();

  // Handles the array type.
  Ast::ArrayType* array_type =
      dynamic_cast<Ast::ArrayType*>(declaration->GetVariableType());
  if (array_type == nullptr) INTERNAL_ERROR("array_type is nullptr.");
  if (array_type->GetTypeCategory() == Ast::Type::TypeCategory::kConst)
    LOGGING_ERROR("const array not support.");

  std::string variable_name = declaration->GetVariableName();

  // Adds the array index and the type index.
  memory->AddWithType(variable_name, array_type->GetVmType());
  std::size_t array_index = global_memory->AddReference(memory, variable_name);
  std::size_t array_type_index = 0;

  // Gets the sub type of the array type and its category.
  Ast::Type* sub_type = array_type->GetSubType();
  auto sub_type_category = sub_type->GetTypeCategory();

  // If the sub type is a class type, it needs to be handled specially.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::string class_name = GetClassNameString(
        interpreter, dynamic_cast<Ast::ClassType*>(sub_type));
    array_type_index = global_memory->AddString(class_name);

  } else {
    // If the vm type of the sub type isn't 0x00 (auto type), it means that
    // the sub type is a primitive type, so we can add it into the global
    // memory.
    if (sub_type->GetVmType()[0] != 0x00)
      array_type_index = global_memory->AddWithType(sub_type->GetVmType());
  }

  // Handles the array creation bytecode.
  // The array is created with the size of 1, and the type of the sub type.
  // This means that regardless of the size of the array definition, it is
  // actually determined based on the actual number of initialization lists
  // given.
  code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                          global_memory->AddByte(1), array_type_index));

  // If the sub type is a class type, it needs to be handled specially. Because
  // the default generated class index is considered an initialized value
  // because it is smaller than the array size, it will not be automatically
  // initialized when the ARRAY operator is called.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::size_t current_index = global_memory->AddReference(
        std::get<std::shared_ptr<Memory>>(
            global_memory->GetMemory()[array_index].data),
        0);
    code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, current_index,
                            global_memory->AddString("@constructor"), 1, 0));
  }

  // Handles the array initialization with the initialization lists.
  if (!declaration->GetVariableValue().empty()) {
    for (std::size_t i = 0; i < declaration->GetVariableValue().size(); i++) {
      // Gets the corresponding array index reference.
      std::size_t current_index = global_memory->AddReference(
          std::get<std::shared_ptr<Memory>>(
              global_memory->GetMemory()[array_index].data),
          i);

      // Gets the value of the initialization list and assigns value to
      // corresponding index.
      std::size_t value_index = HandleExpression(
          interpreter, declaration->GetVariableValue()[i], code);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }
  }

  variables[variable_name] = array_index;

  return array_index;
}

std::size_t HandleStaticArrayDeclaration(Interpreter& interpreter,
                                         Ast::ArrayDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& global_memory = interpreter.global_memory;
  auto& variables = interpreter.context.variables;
  auto& scopes = interpreter.context.scopes;
  auto& global_code = interpreter.global_code;

  // Handles the array type.
  Ast::ArrayType* array_type =
      dynamic_cast<Ast::ArrayType*>(declaration->GetVariableType());
  if (array_type == nullptr) INTERNAL_ERROR("array_type is nullptr.");
  if (array_type->GetTypeCategory() == Ast::Type::TypeCategory::kConst)
    LOGGING_ERROR("const array not support.");

  std::string variable_name =
      scopes.back() + "." + declaration->GetVariableName();

  // Adds the array index and the type index.
  std::size_t array_index = global_memory->AddWithType(array_type->GetVmType());
  std::size_t array_type_index = 0;

  // Gets the sub type of the array type and its category.
  Ast::Type* sub_type = array_type->GetSubType();
  auto sub_type_category = sub_type->GetTypeCategory();

  // If the sub type is a class type, it needs to be handled specially.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::string class_name = GetClassNameString(
        interpreter, dynamic_cast<Ast::ClassType*>(sub_type));
    array_type_index = global_memory->AddString(class_name);

  } else {
    // If the vm type of the sub type isn't 0x00 (auto type), it means that
    // the sub type is a primitive type, so we can add it into the global
    // memory.
    if (sub_type->GetVmType()[0] != 0x00)
      array_type_index = global_memory->AddWithType(sub_type->GetVmType());
  }

  // Handles the array creation bytecode.
  // The array is created with the size of 1, and the type of the sub type.
  // This means that regardless of the size of the array definition, it is
  // actually determined based on the actual number of initialization lists
  // given.
  global_code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                                 global_memory->AddByte(1), array_type_index));

  // If the sub type is a class type, it needs to be handled specially. Because
  // the default generated class index is considered an initialized value
  // because it is smaller than the array size, it will not be automatically
  // initialized when the ARRAY operator is called.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::size_t current_index = global_memory->AddReference(
        std::get<std::shared_ptr<Memory>>(
            global_memory->GetMemory()[array_index].data),
        0);
    global_code.push_back(
        Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, current_index,
                 global_memory->AddString("@constructor"), 1, 0));
  }

  // Handles the array initialization with the initialization lists.
  if (!declaration->GetVariableValue().empty()) {
    for (std::size_t i = 0; i < declaration->GetVariableValue().size(); i++) {
      // Gets the corresponding array index reference.
      std::size_t current_index = global_memory->AddReference(
          std::get<std::shared_ptr<Memory>>(
              global_memory->GetMemory()[array_index].data),
          i);

      // Gets the value of the initialization list and assigns value to
      // corresponding index.
      std::size_t value_index = HandleExpression(
          interpreter, declaration->GetVariableValue()[i], global_code);
      global_code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }
  }

  variables[variable_name] = array_index;
  return array_index;
}

std::size_t HandleClassArrayDeclaration(Interpreter& interpreter,
                                        Ast::ArrayDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& global_memory = interpreter.global_memory;
  auto& current_class = interpreter.context.current_class;
  auto memory = current_class->GetMembers();
  auto& code = current_class->GetCode();

  // Handles the array type.
  Ast::ArrayType* array_type =
      dynamic_cast<Ast::ArrayType*>(declaration->GetVariableType());
  if (array_type == nullptr) INTERNAL_ERROR("array_type is nullptr.");
  if (array_type->GetTypeCategory() == Ast::Type::TypeCategory::kConst)
    LOGGING_ERROR("const array not support.");

  std::string variable_name = declaration->GetVariableName();

  // Adds the array index and the type index.
  memory->AddWithType(variable_name, array_type->GetVmType());
  std::size_t array_index = global_memory->Add(1);
  code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, array_index, 2,
                          global_memory->AddString(variable_name)));
  std::size_t array_type_index = 0;

  // Gets the sub type of the array type and its category.
  Ast::Type* sub_type = array_type->GetSubType();
  auto sub_type_category = sub_type->GetTypeCategory();

  // If the sub type is a class type, it needs to be handled specially.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::string class_name = GetClassNameString(
        interpreter, dynamic_cast<Ast::ClassType*>(sub_type));
    array_type_index = global_memory->AddString(class_name);

  } else {
    // If the vm type of the sub type isn't 0x00 (auto type), it means that
    // the sub type is a primitive type, so we can add it into the global
    // memory.
    if (sub_type->GetVmType()[0] != 0x00)
      array_type_index = global_memory->AddWithType(sub_type->GetVmType());
  }

  // Handles the array creation bytecode.
  // The array is created with the size of 1, and the type of the sub type.
  // This means that regardless of the size of the array definition, it is
  // actually determined based on the actual number of initialization lists
  // given.
  code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                          global_memory->AddByte(1), array_type_index));

  // If the sub type is a class type, it needs to be handled specially. Because
  // the default generated class index is considered an initialized value
  // because it is smaller than the array size, it will not be automatically
  // initialized when the ARRAY operator is called.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::size_t current_index = global_memory->Add(1);
    code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, current_index, 2,
                            global_memory->AddString(variable_name)));
    code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, current_index,
                            global_memory->AddString("@constructor"), 1, 0));
  }

  // Handles the array initialization with the initialization lists.
  if (!declaration->GetVariableValue().empty()) {
    for (std::size_t i = 0; i < declaration->GetVariableValue().size(); i++) {
      // Gets the corresponding array index reference.
      std::size_t current_index = global_memory->Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, current_index, 2,
                              global_memory->AddString(variable_name)));

      // Gets the value of the initialization list and assigns value to
      // corresponding index.
      std::size_t value_index = HandleExpression(
          interpreter, declaration->GetVariableValue()[i], code);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }
  }

  return array_index;
}

void HandleFunctionArguments(Interpreter& interpreter,
                             Ast::FunctionDeclaration* declaration,
                             std::vector<std::size_t>& parameters_index,
                             std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& memory = interpreter.global_memory;
  auto& variables = interpreter.context.variables;

  // Gets the function statement and its parameters.
  Ast::Function* statement = declaration->GetFunctionStatement();
  auto parameters = statement->GetParameters();

  // Handles functions that only contain variable parameters.
  if (parameters.size() == 0 && statement->IsVariadic()) {
    std::size_t index = memory->Add(1);

    // Adds index into |parameters_index| and |variables|.
    parameters_index.push_back(index);
    variables[scopes.back() + "#args"] = index;
    return;
  }

  // Handles function parameters.
  for (std::size_t i = 0; i < parameters.size(); i++) {
    if (Ast::IsOfType<Ast::Variable>(parameters[i])) {
      // Gets function declaration and name.
      auto declaration = Ast::Cast<Ast::Variable>(parameters[i]);
      std::string name = scopes.back() + "#" + declaration->GetVariableName();

      std::size_t index =
          HandleVariableDeclaration(interpreter, declaration, code);

      // Adds index into |parameters_index| and |variables|.
      parameters_index.push_back(index);
      variables[name] = index;

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(parameters[i])) {
      // Gets function declaration and name.
      auto declaration = Ast::Cast<Ast::ArrayDeclaration>(parameters[i]);
      std::string name = scopes.back() + "#" + declaration->GetVariableName();

      std::size_t index =
          HandleArrayDeclaration(interpreter, declaration, code);

      // Adds index into |parameters_index| and |variables|.
      parameters_index.push_back(index);
      variables[name] = index;

    } else {
      INTERNAL_ERROR(
          "Function parameters is not a variable or array declaration.");
    }

    // Handles variable parameters if have.
    if (i == parameters.size() - 1 && statement->IsVariadic()) {
      std::size_t index = memory->Add(1);

      // Adds index into |parameters_index| and |variables|.
      parameters_index.push_back(index);
      variables[scopes.back() + "#args"] = index;
    }
  }
}

void HandleReturnInHandlingFunction(Interpreter& interpreter,
                                    std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& exit_index = interpreter.context.function_context->exit_index;
  auto& memory = interpreter.global_memory;

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t return_location = code.size();
  for (std::size_t i = 0; i < exit_index.size(); i++) {
    code[exit_index[i]].SetArgs(1, memory->AddUint64t(return_location));
  }
}

void HandleGotoInHandlingFunction(Interpreter& interpreter,
                                  std::size_t current_scope,
                                  std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& goto_map = interpreter.context.function_context->goto_map;
  auto& memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;
  auto& label_map = interpreter.context.function_context->label_map;

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

    code[goto_map.back().second].SetArgs(1, memory->AddUint64t(goto_location));
    goto_map.pop_back();
  }
}

void AddFunctionIntoList(Interpreter& interpreter,
                         Ast::FunctionDeclaration* declaration,
                         std::string name,
                         std::vector<std::size_t> parameters_index,
                         std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& functions = interpreter.functions;

  Ast::Function* statement = declaration->GetFunctionStatement();

  Function function(name, parameters_index, code);
  LOGGING_INFO("Add function args size: " +
               std::to_string(parameters_index.size()));
  if (statement->IsVariadic()) function.EnableVariadic();
  functions[name].push_back(function);
  LOGGING_INFO("Add function: " + name + " into function list.");
}

void AddClassFunctionIntoList(Interpreter& interpreter,
                              Ast::FunctionDeclaration* declaration,
                              std::vector<std::size_t>& parameters_index,
                              std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& methods = interpreter.context.current_class->GetMethods();

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();

  // Adds function into class function list.
  Function function(name, parameters_index, code);
  if (statement->IsVariadic()) function.EnableVariadic();
  methods[name].push_back(function);
}

void AddClassConstructorFunctionIntoList(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration,
    std::vector<std::size_t>& parameters_index, std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& methods = interpreter.context.current_class->GetMethods();

  Ast::Function* statement = declaration->GetFunctionStatement();

  // Adds function into class function list.
  Function function("@constructor", parameters_index, code);
  if (statement->IsVariadic()) function.EnableVariadic();
  methods["@constructor"].push_back(function);
}

void HandleReturnVariableInHandlingFunction(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration,
    std::string scope_name, std::vector<std::size_t>& parameters_index) {
  // Gets the reference of context.
  auto& variables = interpreter.context.variables;
  auto& memory = interpreter.global_memory;

  std::vector<uint8_t> vm_type = declaration->GetReturnType()->GetVmType();
  variables[scope_name + "#!return"] = memory->AddWithType(vm_type);
  variables[scope_name + "#!return_reference"] = memory->Add(1);
  parameters_index.push_back(variables[scope_name + "#!return_reference"]);
}

std::vector<std::size_t> HandleFactoryFunctionInHandlingConstructor(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Creates temporary context.
  interpreter.context.function_context = new FunctionContext();

  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& memory = interpreter.global_memory;
  auto& functions = interpreter.functions;

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();
  auto parameters = statement->GetParameters();
  std::vector<Bytecode> code;

  // name = GetFunctionNameWithScope(interpreter, declaration);
  name = scopes.back();
  scopes.push_back(name);

  // Records the creation of the function.
  functions[name].push_back(Function());
  LOGGING_INFO("Add function: " + name + " into function list.");

  // Handles the return value and parameters.
  std::vector<std::size_t> parameters_index;
  std::size_t return_index = memory->Add(1);
  parameters_index.push_back(return_index);
  HandleFunctionArguments(interpreter, declaration, parameters_index, code);

  // Builds the main part of the factory function.
  code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, return_index,
                          memory->AddUint64t(0), memory->AddString(name)));
  std::vector<std::size_t> method_parameters = parameters_index;
  method_parameters.erase(method_parameters.begin());
  method_parameters.insert(
      method_parameters.begin(),
      {return_index, memory->AddString("@constructor"), memory->Add(1)});
  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, method_parameters));

  AddFunctionIntoList(interpreter, declaration, name, parameters_index, code);
  LOGGING_INFO("Add function: " + name + " into function list.");

  // Destroys temporary context.
  scopes.pop_back();
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;

  return parameters_index;
}

void HandleConstructorFunctionInHandlingConstructor(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration,
    std::vector<std::size_t>& parameters_index) {
  // Creates temporary context.
  interpreter.context.function_context = new FunctionContext();

  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& memory = interpreter.global_memory;
  auto& functions = interpreter.functions;
  auto& current_class = interpreter.context.current_class;
  auto& current_scope = interpreter.context.function_context->current_scope;

  // Creates the bytecode vector.
  std::vector<Bytecode> code;

  // Handles the function name with scopes.
  std::string scope_name = GetFunctionNameWithScope(interpreter, declaration);
  scopes.push_back(scope_name);
  current_scope = scopes.size() - 1;

  // Records the creation of the function.
  current_class->GetMethods()["@constructor"].push_back(Function());

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

  HandleClassStatement(interpreter, declaration->GetFunctionBody(), code);

  HandleReturnInHandlingFunction(interpreter, code);

  HandleGotoInHandlingFunction(interpreter, current_scope, code);

  AddClassConstructorFunctionIntoList(interpreter, declaration,
                                      parameters_index, code);

  // Destroys temporary context.
  scopes.pop_back();
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;
}

void HandleSubClassesInHandlingClass(Interpreter& interpreter,
                                     Ast::Class* declaration) {
  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& classes = interpreter.classes;

  for (std::size_t i = 0; i < declaration->GetSubClasses().size(); i++) {
    if (Ast::IsOfType<Ast::Class>(declaration->GetSubClasses()[i])) {
      // Handles the sub class declaration.
      auto sub_class = Ast::Cast<Ast::Class>(declaration->GetSubClasses()[i]);
      HandleClassDeclaration(interpreter, sub_class);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }

  // Restores class name.
  auto& current_class = interpreter.context.current_class;
  std::string class_name =
      scopes.back() + "." + std::string(declaration->GetClassName());
  current_class = &classes[class_name];
}

void HandleStaticMembersInHandlingClass(Interpreter& interpreter,
                                        Ast::Class* declaration) {
  for (std::size_t i = 0; i < declaration->GetStaticMembers().size(); i++) {
    auto member = declaration->GetStaticMembers()[i]->GetStaticDeclaration();
    if (Ast::IsOfType<Ast::Variable>(member)) {
      // Handles the static variable declaration.
      auto variable = Ast::Cast<Ast::Variable>(member);
      HandleStaticVariableDeclaration(interpreter, variable);

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(member)) {
      // Handles the static array declaration.
      auto array = Ast::Cast<Ast::ArrayDeclaration>(member);
      HandleStaticArrayDeclaration(interpreter, array);

    } else if (Ast::IsOfType<Ast::FunctionDeclaration>(member)) {
      // Handles the static function declaration.
      auto function = Ast::Cast<Ast::FunctionDeclaration>(member);
      HandleFunctionDeclaration(interpreter, function);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }
}

void HandleClassMembersInHandlingClass(Interpreter& interpreter,
                                       Ast::Class* declaration) {
  // Gets the reference of context.
  auto& current_class = interpreter.context.current_class;
  auto memory = current_class->GetMembers();
  auto& code = current_class->GetCode();

  for (std::size_t i = 0; i < declaration->GetMembers().size(); i++) {
    auto member = dynamic_cast<Ast::Declaration*>(declaration->GetMembers()[i]);

    if (Ast::IsOfType<Ast::Variable>(member)) {
      // Handles the variable.
      auto variable = Ast::Cast<Ast::Variable>(member);
      HandleClassVariableDeclaration(interpreter, variable);

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(member)) {
      // Handles the array declaration.
      auto array = Ast::Cast<Ast::ArrayDeclaration>(member);
      HandleClassArrayDeclaration(interpreter, array);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }
}

void HandleMethodsInHandlingClass(Interpreter& interpreter,
                                  Ast::Class* declaration) {
  for (std::size_t i = 0; i < declaration->GetMethods().size(); i++) {
    if (Ast::IsOfType<Ast::FunctionDeclaration>(declaration->GetMethods()[i])) {
      // Handles the class function declaration.
      auto function =
          Ast::Cast<Ast::FunctionDeclaration>(declaration->GetMethods()[i]);
      HandleClassFunctionDeclaration(interpreter, function);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }
}

void AddVoidConstructorInHandlingClass(Interpreter& interpreter,
                                       Ast::Class* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  auto parameters_index =
      HandleVoidFactoryFunctionInHandlingClass(interpreter, declaration);

  HandleVoidConstructorFunctionInHandlingClass(interpreter, declaration,
                                               parameters_index);
}
std::vector<std::size_t> HandleVoidFactoryFunctionInHandlingClass(
    Interpreter& interpreter, Ast::Class* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Creates temporary context.
  interpreter.context.function_context = new FunctionContext();

  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& functions = interpreter.functions;
  auto& current_class = interpreter.context.current_class;
  auto& goto_map = interpreter.context.function_context->goto_map;
  auto& variables = interpreter.context.variables;
  auto& memory = interpreter.global_memory;
  auto& exit_index = interpreter.context.function_context->exit_index;

  // Handles the function name with scopes and the class name.
  std::string name = scopes.back();
  std::string class_name = declaration->GetClassName();

  // Handles the parameters.
  std::vector<std::size_t> parameters_index;
  std::size_t return_index = memory->Add(1);
  parameters_index.push_back(return_index);

  // Records the creation of the function.
  functions[name].push_back(Function());

  // Builds the main part of the factory function.
  std::vector<Bytecode> code;
  code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, return_index,
                          memory->AddUint64t(0),
                          memory->AddString(class_name)));
  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, return_index,
                          memory->AddString("@constructor"), 1, return_index));

  // Adds the function into function list.
  Function factory(name, parameters_index, code);
  functions[name].push_back(factory);

  // Destroys temporary context.
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;
  return parameters_index;
}

void HandleVoidConstructorFunctionInHandlingClass(
    Interpreter& interpreter, Ast::Class* declaration,
    std::vector<std::size_t>& parameters_index) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& current_class = interpreter.context.current_class;

  // Records the creation of the function.
  current_class->GetMethods()["@constructor"].push_back(Function());

  // Handles the class init.
  std::vector<Bytecode> code;
  for (std::size_t i = 0; i < current_class->GetCode().size(); i++) {
    code.push_back(current_class->GetCode()[i]);
  }

  // Adds function into class function list.
  Function constructor("@constructor", parameters_index, code);
  current_class->GetMethods()["@constructor"].push_back(constructor);

  // Destroys temporary context.
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;
}

void HandleClassInHandlingVariable(Interpreter& interpreter,
                                   Ast::Variable* declaration,
                                   std::size_t variable_index,
                                   std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& memory = interpreter.global_memory;
  // auto& global_code = interpreter.global_code;
  auto& scopes = interpreter.context.scopes;
  auto& functions = interpreter.functions;

  // Gets the class name, which is same as the function name.
  std::string name = std::string(
      *dynamic_cast<Ast::ClassType*>(declaration->GetVariableType()));
  for (int64_t i = scopes.size() - 1; i >= -1; i--) {
    // Searching globally first and then conducting a search with scope is to
    // avoid the situation where the scope index is exceeded and -1 occurs.
    auto iterator = functions.find(name);

    // If the search scope is not the global scope, adds the scope name to the
    // class name.
    if (i != -1) iterator = functions.find(scopes[i] + "." + name);
    if (iterator != functions.end()) {
      // Searching globally first and then conducting a search with scope is to
      // avoid the situation where the scope index is exceeded and -1 occurs.
      name = name;

      // If the search scope is not the global scope, adds the scope name to the
      // class name.
      if (i != -1) name = scopes[i] + "." + name;
      break;
    }
    if (i == -1) LOGGING_ERROR("Class not found.");
  }

  // Adds the class into global memory.
  std::size_t reference_index = memory->Add(1);
  code.push_back(
      Bytecode(_AQVM_OPERATOR_REFER, 2, reference_index, variable_index));
  code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, reference_index,
                          memory->AddByte(0), memory->AddString(name)));

  // Classes without initialization requires default initialization.
  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, reference_index,
                          memory->AddString("@constructor"), 1,
                          reference_index));
}

std::string GetClassNameString(Interpreter& interpreter, Ast::ClassType* type) {
  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& functions = interpreter.functions;

  std::string name = type->GetClassName();
  for (int64_t i = scopes.size() - 1; i >= -1; i--) {
    // Searching globally first and then conducting a search with scope is to
    // avoid the situation where the scope index is exceeded and -1 occurs.
    auto iterator = functions.find(name);

    // If the search scope is not the global scope, adds the scope name to the
    // class name.
    if (i != -1) iterator = functions.find(scopes[i] + "." + name);
    if (iterator != functions.end()) {
      // Searching globally first and then conducting a search with scope is to
      // avoid the situation where the scope index is exceeded and -1 occurs.
      name = name;

      // If the search scope is not the global scope, adds the scope name to the
      // class name.
      if (i != -1) name = scopes[i] + "." + name;
      break;
    }
    if (i == -1) LOGGING_ERROR("Class not found.");
  }

  return name;
}

[[deprecated]] void GenerateBytecode(std::string import_location) {
  Interpreter* interpreter = new Interpreter();
  std::vector<char> code;
  Aq::ReadCodeFromFile(import_location.c_str(), code);

  std::vector<Aq::Token> token;
  Aq::LexCode(code, token);

  Aq::Ast::Compound* ast = Aq::Parser::Parse(token);
  if (ast == nullptr) Aq::LOGGING_ERROR("ast is nullptr.");

  interpreter->Generate(ast);

  Aq::LOGGING_INFO("Generate Bytecode SUCCESS!");

  imports_map.insert(std::make_pair(import_location, interpreter));
}

std::string GetFunctionNameWithScope(Interpreter& interpreter,
                                     Ast::FunctionDeclaration* declaration) {
  // Gets the reference of context.

  auto& scopes = interpreter.context.scopes;

  // Gets the function statement and its parameters.
  Ast::Function* statement = declaration->GetFunctionStatement();
  std::vector<Ast::Expression*> parameters = statement->GetParameters();

  // Gets the function name with scopes.
  std::string scope_name = scopes.back() + "." + statement->GetFunctionName();

  // Adds the function parameters type into the scope name.
  for (std::size_t i = 0; i < parameters.size(); i++) {
    // Adds the argument type separator to the scope name.
    if (i == 0) {
      scope_name += "@";
    } else {
      scope_name += ",";
    }

    // Adds the argument type to the scope name.
    if (Ast::IsOfType<Ast::Variable>(parameters[i])) {
      auto declaration = Ast::Cast<Ast::Variable>(parameters[i]);
      scope_name += std::string(*declaration->GetVariableType());

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(parameters[i])) {
      auto declaration = Ast::Cast<Ast::ArrayDeclaration>(parameters[i]);
      scope_name += std::string(*declaration->GetVariableType());

    } else {
      LOGGING_ERROR(
          "Function parameters is not a variable or array declaration.");
    }

    // Handles variadic parameters.
    if (i == parameters.size() - 1 && statement->IsVariadic()) {
      scope_name += ",...";
    }
  }

  // Handles variadic parameters if the function is variadic and parameters size
  // is 0.
  if (parameters.size() == 0 && statement->IsVariadic()) {
    scope_name += "@...";
  }

  return scope_name;
}

}  // namespace Interpreter
}  // namespace Aq