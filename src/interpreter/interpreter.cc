// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/interpreter.h"

#include <cstdint>

#include "ast/ast.h"
#include "interpreter/declaration_interpreter.h"
#include "interpreter/goto_interpreter.h"
#include "interpreter/operator.h"
#include "interpreter/preprocesser.h"
#include "interpreter/statement_interpreter.h"
#include "logging/logging.h"
#include "memory.h"

namespace Aq {
namespace Interpreter {
void Interpreter::Generate(Ast::Compound* statement) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // Adds the function context for the main function.
  auto function_context = new FunctionContext();
  context.function_context = function_context;

  // Main program return value and its reference.
  global_memory->Add(1);
  global_memory->Add(1);

  // Bytecode Running class.
  std::vector<uint8_t> bytecodeclass_vm_type{0x09};
  global_memory->AddWithType(bytecodeclass_vm_type);
  global_code.push_back(
      Bytecode(_AQVM_OPERATOR_EQUAL, 2, 0, global_memory->AddString("(void)")));
  global_code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, 1, 0));

  // Sets the scope information.
  context.scopes.push_back("");
  context.function_context->current_scope = context.scopes.size() - 1;

  // Sets the initialize function.
  global_code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, 2,
                                 global_memory->AddString(".!__init"), 1,
                                 global_memory->Add(1)));

  // Sets the current class.
  Class* start_class = &this->main_class;
  context.current_class = start_class;
  start_class->SetName(".__start");
  start_class->GetMembers()->AddString("@name", ".__start");

  // Preprocesses the declaration statements.
  PreProcessDeclaration(*this, statement);

  std::vector<Ast::Statement*> statements;
  std::vector<Ast::Import*> imports;

  // Initialize the statements that need to be preprocessed for the parts that
  // have not been preprocessed in the preprocessor.
  for (std::size_t i = 0; i < statement->GetStatements().size(); i++) {
    Ast::Statement* sub_statement = statement->GetStatements()[i];
    switch (sub_statement->GetStatementType()) {
      case Ast::Statement::StatementType::kFunctionDeclaration:
        HandleFunctionDeclaration(
            *this, Ast::Cast<Ast::FunctionDeclaration>(sub_statement));
        break;

      case Ast::Statement::StatementType::kVariable:
        context.variables[context.scopes.back() + "#" +
                          Ast::Cast<Ast::Variable>(sub_statement)
                              ->GetVariableName()] =
            HandleGlobalVariableDeclaration(
                *this, Ast::Cast<Ast::Variable>(sub_statement), init_code);
        break;

      case Ast::Statement::StatementType::kArrayDeclaration:
        context.variables[context.scopes.back() + "#" +
                          Ast::Cast<Ast::ArrayDeclaration>(sub_statement)
                              ->GetVariableName()] =
            HandleGlobalArrayDeclaration(
                *this, Ast::Cast<Ast::ArrayDeclaration>(sub_statement),
                init_code);
        break;

      case Ast::Statement::StatementType::kGoto:
        statements.push_back(sub_statement);
        break;

      case Ast::Statement::StatementType::kImport:
        imports.push_back(Ast::Cast<Ast::Import>(sub_statement));
        break;

      default:
        statements.push_back(sub_statement);
    }
  }

  // Restores function context null caused by possible function generation.
  context.function_context = function_context;
  context.current_class = start_class;

  for (std::size_t i = 0; i < imports.size(); i++) {
    HandleImport(*this, imports[i]);
  }

  for (std::size_t i = 0; i < statements.size(); i++) {
    switch (statements[i]->GetStatementType()) {
      case Ast::Statement::StatementType::kGoto:
        HandleGoto(*this, Ast::Cast<Ast::Goto>(statements[i]), global_code);
        break;

      default:
        HandleStatement(*this, statements[i], global_code);
    }
  }

  // Restores function context null caused by possible function generation.
  context.function_context = function_context;
  context.current_class = start_class;

  while (context.function_context->goto_map.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = context.scopes.size() - 1;
         i >= context.function_context->current_scope; i--) {
      // Searches for the label in the current scope.
      auto iterator = context.function_context->label_map.find(
          context.scopes[i] + "$" +
          context.function_context->goto_map.back().first);

      // If the label is found in the current scope, it will be replaced with
      // the address of the label.
      if (iterator != context.function_context->label_map.end()) {
        goto_location = iterator->second;
        break;
      }

      if (i == 0) LOGGING_ERROR("Label not found.");
    }

    global_code[context.function_context->goto_map.back().second].SetArgs(
        1, global_memory->AddUint64t(goto_location));
    context.function_context->goto_map.pop_back();
  }

  context.scopes.clear();
  context.function_context->current_scope = 0;

  // Generates the bytecode for the main function.
  std::vector<std::size_t> constructor_args;
  std::vector<Bytecode> start_code;
  constructor_args.push_back(global_memory->Add(1));
  std::size_t start_function_name = global_memory->AddString(".!__start");

  // Adds the start function name into the constructor arguments. And makes the
  // invoke for the start function.
  std::vector<std::size_t> invoke_start_arguments = {2, start_function_name, 1,
                                                     1};
  start_code.push_back(
      Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_start_arguments));

  // Makes the constructor function for the start function.
  Function constructor_func("@constructor", constructor_args, start_code);
  functions["@constructor"] = constructor_func;

  // Adds the start function into the global memory.
  std::vector<std::size_t> arguments;
  arguments.push_back(1);

  // Adds the main function invoke into the global code.
  std::size_t main_func = global_memory->AddString(".main");
  std::vector<std::size_t> invoke_main_arguments = {2, main_func, 1, 1};
  global_code.push_back(
      Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_main_arguments));
  Function start_func(".!__start", arguments, global_code);
  functions[".!__start"] = start_func;

  // Checks if the break statement is used outside of loops or switches.
  if (context.function_context->loop_break_index.size() != 0)
    LOGGING_ERROR("Break cannot be used outside of loops and switches.");

  // Generates the bytecode for the initialization function.
  std::vector<std::size_t> memory_init_args;
  memory_init_args.push_back(global_memory->Add(1));
  Function memory_init_func(".!__init", memory_init_args, init_code);
  functions[".!__init"] = memory_init_func;

  Run();
}

void Interpreter::Run() { is_run = true; }

}  // namespace Interpreter
}  // namespace Aq