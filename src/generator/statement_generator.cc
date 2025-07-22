// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "generator/statement_generator.h"

#include "ast/ast.h"
#include "generator/declaration_generator.h"
#include "generator/expression_generator.h"
#include "generator/generator.h"
#include "generator/goto_generator.h"
#include "logging/logging.h"

namespace Aq {
namespace Generator {
void HandleStatement(Generator& generator, Ast::Statement* statement,
                     std::vector<Bytecode>& code) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  LOGGING_INFO("Handling statement: " +
               std::to_string(static_cast<int>(statement->GetStatementType())));

  switch (statement->GetStatementType()) {
    case Ast::Statement::StatementType::kImport:
      LOGGING_WARNING("Unexpected import statement in the code.");
      break;

    case Ast::Statement::StatementType::kBreak:
      HandleBreakStatement(generator, code);
      break;

    case Ast::Statement::StatementType::kCompound:
      LOGGING_INFO("Handling compound statement.");
      HandleCompoundStatement(generator, Ast::Cast<Ast::Compound>(statement),
                              code);
      break;

    case Ast::Statement::StatementType::kExpression:
      HandleExpression(generator, Ast::Cast<Ast::Expression>(statement), code);
      break;

    case Ast::Statement::StatementType::kUnary:
      HandleUnaryExpression(generator, Ast::Cast<Ast::Unary>(statement), code);
      break;

    case Ast::Statement::StatementType::kBinary:
      HandleBinaryExpression(generator, Ast::Cast<Ast::Binary>(statement),
                             code);
      break;

    case Ast::Statement::StatementType::kIf:
      HandleIfStatement(generator, Ast::Cast<Ast::If>(statement), code);
      break;

    case Ast::Statement::StatementType::kWhile:
      HandleWhileStatement(generator, Ast::Cast<Ast::While>(statement), code);
      break;

    case Ast::Statement::StatementType::kDowhile:
      HandleDowhileStatement(generator, Ast::Cast<Ast::DoWhile>(statement),
                             code);
      break;

    case Ast::Statement::StatementType::kFor:
      HandleForStatement(generator, Ast::Cast<Ast::For>(statement), code);
      break;

    case Ast::Statement::StatementType::kFunctionDeclaration:
      HandleFunctionDeclaration(generator,
                                Ast::Cast<Ast::FunctionDeclaration>(statement));
      break;

    case Ast::Statement::StatementType::kVariable:
      HandleVariableDeclaration(generator, Ast::Cast<Ast::Variable>(statement),
                                code);
      break;

    case Ast::Statement::StatementType::kArrayDeclaration:
      HandleArrayDeclaration(generator,
                             Ast::Cast<Ast::ArrayDeclaration>(statement), code);
      break;

    case Ast::Statement::StatementType::kClass:
      HandleClassDeclaration(generator, Ast::Cast<Ast::Class>(statement));
      break;

    case Ast::Statement::StatementType::kFunction:
      HandleFunctionInvoke(generator, Ast::Cast<Ast::Function>(statement),
                           code);
      break;

    case Ast::Statement::StatementType::kReturn:
      HandleReturnStatement(generator, Ast::Cast<Ast::Return>(statement), code);
      break;

    case Ast::Statement::StatementType::kLabel:
      HandleLabel(generator, Ast::Cast<Ast::Label>(statement), code);
      break;

    case Ast::Statement::StatementType::kGoto:
      HandleGoto(generator, Ast::Cast<Ast::Goto>(statement), code);
      break;

    case Ast::Statement::StatementType::kStatement:
    default:
      // LOGGING_INFO("Unexpected statement type in the code." +
      //     std::to_string(static_cast<int>(statement->GetStatementType())));
      LOGGING_WARNING("Unexpected statement type in the code.");
      break;
  }
}

void HandleBreakStatement(Generator& generator, std::vector<Bytecode>& code) {
  std::size_t index = 0;
  code.push_back(
      Bytecode(_AQVM_OPERATOR_GOTO, 1,
               generator.global_memory.AddUint64tWithoutValue(index)));

  generator.context.function_context->loop_break_index.push_back(index);
}

void HandleClassStatement(Generator& generator, Ast::Statement* statement,
                          std::vector<Bytecode>& code) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  switch (statement->GetStatementType()) {
    case Ast::Statement::StatementType::kCompound:
      HandleCompoundStatement(generator, Ast::Cast<Ast::Compound>(statement),
                              code);
      break;

    case Ast::Statement::StatementType::kExpression:
      HandleExpression(generator, Ast::Cast<Ast::Expression>(statement), code);
      break;

    case Ast::Statement::StatementType::kUnary:
      HandleUnaryExpression(generator, Ast::Cast<Ast::Unary>(statement), code);
      break;

    case Ast::Statement::StatementType::kBinary:
      HandleBinaryExpression(generator, Ast::Cast<Ast::Binary>(statement),
                             code);
      break;

    case Ast::Statement::StatementType::kIf:
      HandleIfStatement(generator, Ast::Cast<Ast::If>(statement), code);
      break;

    case Ast::Statement::StatementType::kWhile:
      HandleWhileStatement(generator, Ast::Cast<Ast::While>(statement), code);
      break;

    case Ast::Statement::StatementType::kDowhile:
      HandleDowhileStatement(generator, Ast::Cast<Ast::DoWhile>(statement),
                             code);
      break;

    case Ast::Statement::StatementType::kFor:
      HandleForStatement(generator, Ast::Cast<Ast::For>(statement), code);
      break;

    case Ast::Statement::StatementType::kFunctionDeclaration:
      HandleFunctionDeclaration(generator,
                                Ast::Cast<Ast::FunctionDeclaration>(statement));
      break;

    case Ast::Statement::StatementType::kVariable:
      HandleVariableDeclaration(generator, Ast::Cast<Ast::Variable>(statement),
                                code);
      break;

    case Ast::Statement::StatementType::kArrayDeclaration:
      HandleArrayDeclaration(generator,
                             Ast::Cast<Ast::ArrayDeclaration>(statement), code);
      break;

    case Ast::Statement::StatementType::kClass:
      HandleClassDeclaration(generator, Ast::Cast<Ast::Class>(statement));
      break;

    case Ast::Statement::StatementType::kFunction:
      HandleFunctionInvoke(generator, Ast::Cast<Ast::Function>(statement),
                           code);
      break;

    case Ast::Statement::StatementType::kReturn:
      HandleReturnStatement(generator, Ast::Cast<Ast::Return>(statement), code);
      break;

    case Ast::Statement::StatementType::kLabel:
      HandleLabel(generator, Ast::Cast<Ast::Label>(statement), code);
      break;

    case Ast::Statement::StatementType::kGoto:
      HandleGoto(generator, Ast::Cast<Ast::Goto>(statement), code);
      break;

    case Ast::Statement::StatementType::kStatement:
    default:
      // LOGGING_INFO("Unexpected statement type in the code."+
      //  std::to_string(static_cast<int>(statement->GetStatementType())));
      LOGGING_WARNING("Unexpected statement type in the code.");
      break;
  }
}

void HandleReturnStatement(Generator& generator, Ast::Return* statement,
                           std::vector<Bytecode>& code) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // Gets the reference of context.
  auto& exit_index = generator.context.function_context->exit_index;
  auto& variables = generator.context.variables;
  auto& scopes = generator.context.scopes;
  auto& current_scope = generator.context.function_context->current_scope;

  if (statement->GetExpression() == nullptr) {
    // Void return.
    exit_index.push_back(code.size());
    code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));

  } else {
    // Gets return value.
    std::size_t return_value =
        HandleExpression(generator, statement->GetExpression(), code);

    // Gets the reference of the return variables.
    bool is_find = false;
    auto return_reference_iterator = variables.find("#!return_reference");
    for (int64_t i = scopes.size() - 1; i >= current_scope; i--) {
      return_reference_iterator = variables.find(
          scopes[i] + "#" + static_cast<std::string>("!return_reference"));
      if (return_reference_iterator != variables.end()) {
        is_find = true;
        break;
      }
    }
    if (!is_find) LOGGING_ERROR("Not found identifier define.");

    // Sets the return value to the return reference.
    std::size_t return_reference_index = return_reference_iterator->second;

    // Sets the return value to the return reference.
    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, return_reference_index,
                            return_value));
    exit_index.push_back(code.size());
    code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
  }
}

void HandleCompoundStatement(Generator& generator, Ast::Compound* statement,
                             std::vector<Bytecode>& code) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  LOGGING_INFO("message");

  for (std::size_t i = 0; i < statement->GetStatements().size(); i++) {
    LOGGING_INFO("Handling compound statement: " + std::to_string(i) + "/" +
                 std::to_string(statement->GetStatements().size()));
    HandleStatement(generator, statement->GetStatements()[i], code);
  }
}

void HandleIfStatement(Generator& generator, Ast::If* statement,
                       std::vector<Bytecode>& code) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // Gets the reference of context.
  auto& scopes = generator.context.scopes;
  auto& undefined_count = generator.context.undefined_count;
  auto& global_memory = generator.global_memory;

  std::size_t condition_index =
      HandleExpression(generator, statement->GetConditionExpression(), code);

  // Handles the if operator and its statements.
  std::size_t if_operator_index = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));

  // Handles the true branch of the if statement.
  std::size_t true_operator_location = code.size();
  scopes.push_back(scopes.back() + "@@" + std::to_string(++undefined_count));
  HandleStatement(generator, statement->GetIfBody(), code);
  scopes.pop_back();

  // Handles the goto operator for the true branch.
  std::size_t goto_operator_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));

  // Handles the else operator and its statements if have.
  std::size_t false_operator_location = code.size();
  if (statement->GetElseBody() != nullptr) {
    scopes.push_back(scopes.back() + "@@" + std::to_string(++undefined_count));
    HandleStatement(generator, statement->GetElseBody(), code);
    scopes.pop_back();
  }

  // Handles the exit branch of the if statement.
  // If there is no else body, the exit branch is the goto operator location.
  std::size_t exit_branch = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  // Sets the arguments for the if and goto operators.
  std::vector<std::size_t> if_arguments{condition_index, true_operator_location,
                                        false_operator_location};
  std::vector<std::size_t> goto_arguments{
      global_memory.AddUint64t(exit_branch)};

  // Updates the if and goto operators with the arguments.
  code[if_operator_index].SetArgs(if_arguments);
  code[goto_operator_location].SetArgs(goto_arguments);
}

void HandleWhileStatement(Generator& generator, Ast::While* statement,
                          std::vector<Bytecode>& code) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // Gets the reference of context.
  auto& scopes = generator.context.scopes;
  auto& undefined_count = generator.context.undefined_count;
  auto& global_memory = generator.global_memory;
  auto& loop_break_index = generator.context.function_context->loop_break_index;

  // Adds -1 into loop_break_index as the start flag for the loop.
  loop_break_index.push_back(-1);

  // Initializes the loop with a NOP operator and sets the start location.
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t start_location = code.size();

  // Handles the condition expression of the while statement.
  std::size_t condition_index =
      HandleExpression(generator, statement->GetConditionExpression(), code);

  // Handles the if operator for the condition and its body.
  std::size_t if_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));
  std::size_t body_location = code.size();

  // Handles the true branch of the while statement.
  scopes.push_back(scopes.back() + "@@" + std::to_string(++undefined_count));
  HandleStatement(generator, statement->GetWhileBody(), code);
  scopes.pop_back();

  // Handles the goto operator to jump back to the start of the loop.
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                          global_memory.AddUint64t(start_location)));

  // Handles the exit branch of the while statement.
  std::size_t exit_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  // Sets the arguments for the if and goto operators.
  std::vector<std::size_t> if_arguments{condition_index, body_location,
                                        exit_location};
  code[if_location].SetArgs(if_arguments);

  // Sets the exit location for the loop break statements.
  while (loop_break_index.back() != -1) {
    global_memory.SetUint64tValue(loop_break_index.back(), exit_location);
    loop_break_index.pop_back();
  }

  loop_break_index.pop_back();
}

void HandleDowhileStatement(Generator& generator, Ast::DoWhile* statement,
                            std::vector<Bytecode>& code) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // Gets the reference of context.
  auto& scopes = generator.context.scopes;
  auto& undefined_count = generator.context.undefined_count;
  auto& global_memory = generator.global_memory;
  auto& loop_break_index = generator.context.function_context->loop_break_index;

  // Adds -1 into loop_break_index as the start flag for the loop.
  loop_break_index.push_back(-1);

  // Initializes the loop with a NOP operator and sets the start location.
  // Because do while will loop at least once, handle the body directly.
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t body_location = code.size();

  // Handles the body of the do-while statement.
  scopes.push_back(scopes.back() + "@@" + std::to_string(++undefined_count));
  HandleStatement(generator, statement->GetDoWhileBody(), code);
  scopes.pop_back();

  // Handles the condition expression of the do-while statement.
  std::size_t condition_index =
      HandleExpression(generator, statement->GetConditionExpression(), code);
  std::size_t if_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));

  // Handles the exit branch of the do-while statement.
  std::size_t exit_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  // Sets the arguments for the if and goto operators.
  std::vector<std::size_t> if_args{condition_index, body_location,
                                   exit_location};
  code[if_location].SetArgs(if_args);

  // Sets the exit location for the loop break statements.
  while (loop_break_index.back() != -1) {
    global_memory.SetUint64tValue(loop_break_index.back(), exit_location);
    loop_break_index.pop_back();
  }

  loop_break_index.pop_back();
}

void HandleForStatement(Generator& generator, Ast::For* statement,
                        std::vector<Bytecode>& code) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // Gets the reference of context.
  auto& scopes = generator.context.scopes;
  auto& undefined_count = generator.context.undefined_count;
  auto& global_memory = generator.global_memory;
  auto& loop_break_index = generator.context.function_context->loop_break_index;

  // Adds -1 into loop_break_index as the start flag for the loop.
  loop_break_index.push_back(-1);

  // LOGGING_INFO("1");
  //  Initializes the loop with the start expression.
  scopes.push_back(scopes.back() + "@@" + std::to_string(++undefined_count));
  HandleStatement(generator, statement->GetStartExpression(), code);

  // Adds a NOP operator to mark the start of the loop.
  // This is necessary to ensure the loop can be run correctly.
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t start_location = code.size();

  // LOGGING_INFO("1");

  // Handles the condition expression of the for statement.
  Ast::Expression* ex = statement->GetConditionExpression();
  if (ex == nullptr) LOGGING_ERROR("Condition expression is nullptr.");
  // LOGGING_INFO("0X010110100110010101");
  std::size_t condition_index = HandleExpression(generator, ex, code);
  // LOGGING_INFO("0X010110100110010101");
  // LOGGING_INFO("1");
  //  Handles the judgment of the for statement.
  std::size_t if_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));

  // Handles the body branch and post-expression of the for statement.
  std::size_t body_location = code.size();
  HandleStatement(generator, statement->GetForBody(), code);
  if (!statement->GetEndExpression())
    LOGGING_ERROR("End expression is nullptr.");
  HandleExpression(generator, statement->GetEndExpression(), code);
  scopes.pop_back();

  // LOGGING_INFO("1");
  //  Makes the for statement loop automatically.
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                          global_memory.AddUint64t(start_location)));

  // Handles the exit branch of the for statement.
  std::size_t exit_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  // Sets the arguments for the if and goto operators.
  std::vector<std::size_t> if_args{condition_index, body_location,
                                   exit_location};
  code[if_location].SetArgs(if_args);

  // Sets the exit location for the loop break statements.
  while (loop_break_index.back() != -1) {
    global_memory.SetUint64tValue(loop_break_index.back(), exit_location);
    loop_break_index.pop_back();
  }

  loop_break_index.pop_back();
}
}  // namespace Generator
}  // namespace Aq