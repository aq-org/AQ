// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/preprocesser.h"

#include <string>
#include <vector>

#include "ast/ast.h"
#include "interpreter/class.h"
#include "interpreter/declaration_interpreter.h"
#include "interpreter/interpreter.h"
#include "logging/logging.h"

namespace Aq {
namespace Interpreter {
void PreProcessDeclaration(Interpreter& interpreter, Ast::Compound* statements) {
  if (statements == nullptr) INTERNAL_ERROR("stmt is nullptr.");

  std::vector<Ast::Statement*> statements_buffer;

  for (std::size_t i = 0; i < statements->GetStatements().size(); i++) {
    Ast::Statement* statement = statements->GetStatements()[i];

    if (Ast::IsOfType<Ast::Class>(statement)) {
      //PreProcessClassDeclaration(interpreter, Ast::Cast<Ast::Class>(statement));
      statements_buffer.push_back(statement);

    } else if (Ast::IsOfType<Ast::FunctionDeclaration>(statement)) {
      PreProcessFunctionDeclaration(
          interpreter, Ast::Cast<Ast::FunctionDeclaration>(statement));

    } else if (Ast::IsOfType<Ast::Import>(statement)) {
      PreProcessImport(interpreter, Ast::Cast<Ast::Import>(statement));
    }
  }

  for (std::size_t i = 0; i < statements_buffer.size(); i++) {
    switch (statements_buffer[i]->GetStatementType()) {
      case Ast::Statement::StatementType::kClass:
        PreProcessClassDeclaration(interpreter,
                                   Ast::Cast<Ast::Class>(statements_buffer[i]));
        break;

      default:
        INTERNAL_ERROR("Unexpected statement type.");
        break;
    }
  }
}

void PreProcessFunctionDeclaration(Interpreter& interpreter,
                                   Ast::FunctionDeclaration* statement) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // Gets the reference of the context.
  auto& scopes = interpreter.context.scopes;
  auto& functions = interpreter.context.functions;

  std::string function_name =
      statement->GetFunctionStatement()->GetFunctionName();

  // The function name with the scopes.
  std::string full_name = scopes.back() + "." + function_name;

  functions.insert(full_name);
}

void PreProcessClassDeclaration(Interpreter& interpreter, Ast::Class* statement) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // Gets the reference of the context.
  auto& scopes = interpreter.context.scopes;
  auto& classes = interpreter.context.classes;
  auto& global_memory = interpreter.global_memory;
  auto& functions = interpreter.context.functions;

  std::string class_name = static_cast<std::string>(statement->GetClassName());
  std::string full_name = scopes.back() + "." + class_name;
  scopes.push_back(full_name);

  Class* current_class = new Class();
  current_class->SetName(full_name);
  current_class->SetClass(statement);

  current_class->GetMemory().SetCode(&current_class->GetCode());
  current_class->GetMemory().SetGlobalMemory(&global_memory);

  classes[full_name] = current_class;

  for (std::size_t i = 0; i < statement->GetSubClasses().size(); i++) {
    if (Ast::IsOfType<Ast::Class>(statement->GetSubClasses()[i])) {
      PreProcessClassDeclaration(
          interpreter, Ast::Cast<Ast::Class>(statement->GetSubClasses()[i]));

    } else {
      INTERNAL_ERROR("Unexpected statement type.");
    }
  }

  for (std::size_t i = 0; i < statement->GetStaticMembers().size(); i++) {
    Ast::Declaration* declaration =
        statement->GetStaticMembers()[i]->GetStaticDeclaration();

    if (Ast::IsOfType<Ast::FunctionDeclaration>(declaration)) {
      PreProcessFunctionDeclaration(
          interpreter, Ast::Cast<Ast::FunctionDeclaration>(declaration));
    }

    // Other declarations are handled by the PreProcessStaticDeclaration() in
    // PreProcessDeclaration().
  }

  functions.insert(scopes.back());
  scopes.pop_back();
}

void PreProcessStaticDeclaration(Interpreter& interpreter, Ast::Class* statement) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // Gets the reference of the context.
  auto& scopes = interpreter.context.scopes;
  auto& classes = interpreter.context.classes;
  auto& global_memory = interpreter.global_memory;
  auto& functions = interpreter.context.functions;
  auto& code = interpreter.global_code;

  std::string class_name = static_cast<std::string>(statement->GetClassName());
  std::string full_name = scopes.back() + "." + class_name;
  scopes.push_back(full_name);

  if (classes.find(class_name) == classes.end())
    INTERNAL_ERROR("Not found class declaration.");
  Class* current_class = classes[class_name];

  for (std::size_t i = 0; i < statement->GetSubClasses().size(); i++) {
    if (Ast::IsOfType<Ast::Class>(statement->GetSubClasses()[i])) {
      PreProcessStaticDeclaration(
          interpreter, Ast::Cast<Ast::Class>(statement->GetSubClasses()[i]));

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }

  for (std::size_t i = 0; i < statement->GetStaticMembers().size(); i++) {
    Ast::Declaration* declaration = statement->GetStaticMembers()[i];
    if (Ast::IsOfType<Ast::Declaration>(declaration)) {
      HandleVariableDeclaration(interpreter,
                                Ast::Cast<Ast::Variable>(declaration), code);

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(declaration)) {
      HandleArrayDeclaration(
          interpreter, Ast::Cast<Ast::ArrayDeclaration>(declaration), code);
    }
  }

  functions.insert(scopes.back());
  scopes.pop_back();
}

void PreProcessImport(Interpreter& interpreter, Ast::Import* statement) {
  if (statement == nullptr) INTERNAL_ERROR("stmt is nullptr.");

  // Gets the reference of the context.
  auto& global_memory = interpreter.global_memory;
  auto& variables = interpreter.context.variables;

  if (statement->IsFromImport()) {
    INTERNAL_ERROR("Unsupported import type now.");
  } else {
    std::string import_location = statement->GetImportLocation();
    std::string name = statement->GetName();

    std::size_t array_index = global_memory.Add(1);
    variables.emplace("#" + static_cast<std::string>(name), array_index);
  }
}
}  // namespace Interpreter
}  // namespace Aq