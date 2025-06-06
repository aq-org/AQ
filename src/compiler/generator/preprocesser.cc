// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/generator/preprocesser.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "compiler/ast/ast.h"
#include "compiler/generator/class.h"
#include "compiler/logging/logging.h"

namespace Aq {
namespace Compiler {
namespace Generator {
void PreProcessDeclaration(
    Ast::Compound* statements, std::vector<std::string>& scopes,
    std::unordered_set<std::string>& function_declarations,
    std::unordered_map<std::string, std::size_t>& variable_declaration_map,
    std::unordered_map<std::string, Generator::Class*>& class_declaration_map,
    Memory& global_memory) {
  if (statements == nullptr) INTERNAL_ERROR("stmt is nullptr.");

  std::vector<Ast::Statement*> statements_buffer;

  for (std::size_t i = 0; i < statements->GetStatements().size(); i++) {
    if (Ast::IsOfType<Ast::Class>(statements->GetStatements()[i])) {
      PreProcessClassDeclaration(
          Ast::Cast<Ast::Class>(statements->GetStatements()[i]), scopes,
          function_declarations, class_declaration_map, global_memory);
      statements_buffer.push_back(statements->GetStatements()[i]);
    } else if (Ast::IsOfType<Ast::FunctionDeclaration>(
                   statements->GetStatements()[i])) {
      PreProcessFunctionDeclaration(
          Ast::Cast<Ast::FunctionDeclaration>(statements->GetStatements()[i]),
          scopes, function_declarations);
      statements_buffer.push_back(statements->GetStatements()[i]);
    } else if (Ast::IsOfType<Ast::Import>(statements->GetStatements()[i])) {
      PreProcessImport(Ast::Cast<Ast::Import>(statements->GetStatements()[i]),
                       variable_declaration_map, global_memory);
    }
  }

  for (std::size_t i = 0; i < statements_buffer.size(); i++) {
    switch (statements_buffer[i]->GetStatementType()) {
      case Ast::Statement::StatementType::kClass:
        PreProcessStaticDeclaration(Ast::Cast<Ast::Class>(statements_buffer[i]),
                                    scopes, function_declarations,
                                    class_declaration_map, global_memory);
        break;
      default:
        INTERNAL_ERROR("Unexpected statement type.");
        break;
    }
  }
}

void PreProcessFunctionDeclaration(
    Ast::FunctionDeclaration* statement, std::vector<std::string>& scopes,
    std::unordered_set<std::string>& function_declarations) {
  std::string function_name = static_cast<std::string>(
      *statement->GetFunctionStatement()->GetFunctionNameExpression());
  // The function name with the scopes.
  std::string full_name = scopes.back() + "." + function_name;

  function_declarations.insert(full_name);
}

void PreProcessClassDeclaration(
    Ast::Class* statement, std::vector<std::string>& scopes,
    std::unordered_set<std::string>& function_declarations,
    std::unordered_map<std::string, Generator::Class*>& class_declaration_map,
    Memory& global_memory) {
  std::string class_name = static_cast<std::string>(statement->GetClassName());
  std::string full_name = scopes.back() + "." + class_name;
  scopes.push_back(full_name);

  Generator::Class* current_class = new Generator::Class();
  current_class->SetName(full_name);
  current_class->SetClass(statement);

  current_class->GetMemory().SetCode(&current_class->GetCode());
  current_class->GetMemory().SetGlobalMemory(&global_memory);

  class_declaration_map[full_name] = current_class;

  for (std::size_t i = 0; i < statement->GetSubClasses().size(); i++) {
    if (Ast::IsOfType<Ast::Class>(statement->GetSubClasses()[i])) {
      PreProcessClassDeclaration(
          Ast::Cast<Ast::Class>(statement->GetSubClasses()[i]), scopes,
          function_declarations, class_declaration_map, global_memory);
    } else {
      INTERNAL_ERROR("Unexpected statement type.");
    }
  }

  for (std::size_t i = 0; i < statement->GetStaticMembers().size(); i++) {
    Ast::Declaration* declaration =
        statement->GetStaticMembers()[i]->GetStaticDeclaration();
    if (Ast::IsOfType<Ast::FunctionDeclaration>(declaration)) {
      PreProcessFunctionDeclaration(
          Ast::Cast<Ast::FunctionDeclaration>(declaration), scopes,
          function_declarations);
    }
    // Other declarations are handled by the PreProcessStaticDeclaration() in
    // PreProcessDeclaration().
  }

  function_declarations.insert(scopes.back());
  scopes.pop_back();
}

void PreProcessStaticDeclaration(
    Ast::Class* statement, std::vector<std::string>& scopes,
    std::unordered_set<std::string>& function_declarations,
    std::unordered_map<std::string, Generator::Class*>& class_declaration_map,
    Memory& global_memory) {
  std::string class_name = static_cast<std::string>(statement->GetClassName());
  std::string full_name = scopes.back() + "." + class_name;
  scopes.push_back(full_name);

  if (class_declaration_map.find(class_name) == class_declaration_map.end())
    INTERNAL_ERROR("Not found class decl.");
  Generator::Class* current_class = class_declaration_map[class_name];

  for (std::size_t i = 0; i < statement->GetSubClasses().size(); i++) {
    if (Ast::IsOfType<Ast::Class>(statement->GetSubClasses()[i])) {
      PreProcessStaticDeclaration(
          Ast::Cast<Ast::Class>(statement->GetSubClasses()[i]), scopes,
          function_declarations, class_declaration_map, global_memory);
    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }

  for (std::size_t i = 0; i < statement->GetStaticMembers().size(); i++) {
    Ast::Declaration* declaration = statement->GetStaticMembers()[i];
    if (Ast::IsOfType<Ast::Declaration>(declaration)) {
      HandleVariableDeclaration(Ast::Cast<Ast::Variable>(declaration),
                                current_class->GetCode());
    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(declaration)) {
      HandleArrayDeclaration(Ast::Cast<Ast::Array>(declaration),
                             current_class->GetCode());
    }
  }

  Ast::FunctionDeclaration* function_declaration =
      new Ast::FunctionDeclaration(nullptr, nullptr, nullptr);
  function_declarations.insert(scopes.back());
  scopes.pop_back();
}

void PreProcessImport(
    Ast::Import* statement,
    std::unordered_map<std::string, std::size_t>& variable_declaration_map,
    Memory& global_memory) {
  if (statement == nullptr) INTERNAL_ERROR("stmt is nullptr.");

  if (statement->IsFromImport()) {
    INTERNAL_ERROR("Unsupported import type now.");
  } else {
    std::string import_location = statement->GetImportLocation();
    std::string name = statement->GetName();

    std::size_t array_index = global_memory.Add(1);
    variable_declaration_map.emplace(
        "#" + static_cast<std::string>(name),
        std::pair<Ast::Variable*, std::size_t>(nullptr, array_index));
  }
}
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq