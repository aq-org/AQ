// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "generator/expression_generator.h"

#include "ast/ast.h"
#include "generator/generator.h"
#include "generator/operator.h"
#include "logging/logging.h"

namespace Aq {
namespace Generator {
std::size_t HandleExpression(Generator& generator, Ast::Expression* expression,
                             std::vector<Bytecode>& code) {
  if (expression == nullptr) INTERNAL_ERROR("expression is nullptr.");

  if (Ast::IsOfType<Ast::Unary>(expression) ||
      Ast::IsOfType<Ast::Array>(expression)) {
    return HandleUnaryExpression(generator, Ast::Cast<Ast::Unary>(expression),
                                 code);
  } else if (Ast::IsOfType<Ast::Binary>(expression)) {
    return HandleBinaryExpression(generator, Ast::Cast<Ast::Binary>(expression),
                                  code);
  }

  return GetIndex(generator, expression, code);
}

std::size_t HandleUnaryExpression(Generator& generator, Ast::Unary* expression,
                                  std::vector<Bytecode>& code) {
  if (expression == nullptr) INTERNAL_ERROR("expression is nullptr.");

  // Gets the reference of context.
  auto& global_memory = generator.global_memory;
  auto& scopes = generator.context.scopes;

  std::size_t sub_expression =
      HandleExpression(generator, expression->GetExpression(), code);
  std::size_t new_index = global_memory.Add(1);

  switch (expression->GetOperator()) {
    case Ast::Unary::Operator::kPostInc: {  // ++ (postfix)
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, new_index, sub_expression));
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, sub_expression,
                              sub_expression, AddConstInt8t(generator, 1)));
      return new_index;
    }

    case Ast::Unary::Operator::kPostDec: {  // -- (postfix)
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, new_index, sub_expression));
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, sub_expression,
                              sub_expression, AddConstInt8t(generator, 1)));
      return new_index;
    }

    case Ast::Unary::Operator::kPreInc:  // ++ (prefix)
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, sub_expression,
                              sub_expression, AddConstInt8t(generator, 1)));
      return sub_expression;

    case Ast::Unary::Operator::kPreDec:  // -- (prefix)
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, sub_expression,
                              sub_expression, AddConstInt8t(generator, 1)));
      return sub_expression;

    case Ast::Unary::Operator::kPlus:  // + (unary plus)
      // Unary plus does not change the value, so we can just return the
      // sub_expression index.
      return sub_expression;

    case Ast::Unary::Operator::kMinus:
    case Ast::Unary::Operator::kNot: {  // - (unary minus) or ! (logical NOT)
      code.push_back(
          Bytecode(_AQVM_OPERATOR_NEG, 2, new_index, sub_expression));
      return new_index;
    }

    case Ast::Unary::Operator::ARRAY: {  // []
      std::size_t offset = HandleExpression(
          generator, Ast::Cast<Ast::Array>(expression)->GetIndexExpression(),
          code);

      code.push_back(
          Bytecode(_AQVM_OPERATOR_ARRAY, 3, new_index, sub_expression, offset));
      return new_index;
    }

    case Ast::Unary::Operator::kBitwiseNot:  // ~ (bitwise NOT)
    // TODO: Implement bitwise NOT operator.
    default:
      LOGGING_WARNING("Unexpected unary operator is not implemented yet.");
      return sub_expression;
  }
}

std::size_t HandleBinaryExpression(Generator& generator,
                                   Ast::Binary* expression,
                                   std::vector<Bytecode>& code) {
  if (expression == nullptr) INTERNAL_ERROR("expression is nullptr.");

  // Gets the reference of context.
  auto& global_memory = generator.global_memory;

  // Gets the reference of expressions.
  Ast::Expression* right_expression = expression->GetRightExpression();
  Ast::Expression* left_expression = expression->GetLeftExpression();

  // Gets the left and right expression indexes.
  std::size_t left = 0;
  std::size_t right = 0;
  if (expression->GetOperator() != Ast::Binary::Operator::kMember) {
    // LOGGING_INFO("message");
    right = HandleExpression(generator, right_expression, code);
    // LOGGING_INFO("message");
    left = HandleExpression(generator, left_expression, code);
    // LOGGING_INFO("message");
  }

  std::size_t result = global_memory.Add(1);
  switch (expression->GetOperator()) {
    case Ast::Binary::Operator::kAdd:  // +
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kSub:  // -
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kMul:  // *
      code.push_back(Bytecode(_AQVM_OPERATOR_MUL, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kDiv:  // /
      code.push_back(Bytecode(_AQVM_OPERATOR_DIV, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kRem:  // %
      code.push_back(Bytecode(_AQVM_OPERATOR_REM, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kAnd:  // &
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kOr:  // |
      code.push_back(Bytecode(_AQVM_OPERATOR_OR, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kXor:  // ^
      code.push_back(Bytecode(_AQVM_OPERATOR_XOR, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kShl:  // <<
      code.push_back(Bytecode(_AQVM_OPERATOR_SHL, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kShr:  // >>
      code.push_back(Bytecode(_AQVM_OPERATOR_SHR, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kLT:  // <
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x04,
                              left, right));
      return result;

    case Ast::Binary::Operator::kGT:  // >
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x02,
                              left, right));
      return result;

    case Ast::Binary::Operator::kLE:  // <=
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x05,
                              left, right));
      return result;

    case Ast::Binary::Operator::kGE:  // >=
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x03,
                              left, right));
      return result;

    case Ast::Binary::Operator::kEQ:  // ==
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x00,
                              left, right));
      return result;

    case Ast::Binary::Operator::kNE:  // !=
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x01,
                              left, right));
      return result;

    case Ast::Binary::Operator::kLAnd:  // &&
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kLOr:  // ||
      code.push_back(Bytecode(_AQVM_OPERATOR_OR, 3, result, left, right));
      return result;

    case Ast::Binary::Operator::kAssign:  // =
      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, left, right));
      return left;

    case Ast::Binary::Operator::kAddAssign:  // +=
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, left, left, right));
      return left;

    case Ast::Binary::Operator::kSubAssign:  // -=
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, left, left, right));
      return left;

    case Ast::Binary::Operator::kMulAssign:  // *=
      code.push_back(Bytecode(_AQVM_OPERATOR_MUL, 3, left, left, right));
      return left;

    case Ast::Binary::Operator::kDivAssign:  // /=
      code.push_back(Bytecode(_AQVM_OPERATOR_DIV, 3, left, left, right));
      return left;

    case Ast::Binary::Operator::kRemAssign:  // %=
      code.push_back(Bytecode(_AQVM_OPERATOR_REM, 3, left, left, right));
      return left;

    case Ast::Binary::Operator::kAndAssign:  // &=
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, left, left, right));
      return left;

    case Ast::Binary::Operator::kOrAssign:  // |=
      code.push_back(Bytecode(_AQVM_OPERATOR_OR, 3, left, left, right));
      return left;

    case Ast::Binary::Operator::kXorAssign:  // ^=
      code.push_back(Bytecode(_AQVM_OPERATOR_XOR, 3, left, left, right));
      return left;

    case Ast::Binary::Operator::kShlAssign:  // <<=
      code.push_back(Bytecode(_AQVM_OPERATOR_SHL, 3, left, left, right));
      return left;

    case Ast::Binary::Operator::kShrAssign:  // >>=
      code.push_back(Bytecode(_AQVM_OPERATOR_SHR, 3, left, left, right));
      return left;

    case Ast::Binary::Operator::kMember: {
      return HandlePeriodExpression(generator, expression, code);
      break;
    }

    default:
      LOGGING_ERROR("Unexpected binary operator.");
      break;
  }

  LOGGING_ERROR("Unexpected binary operator.");
  return 0;
}

std::size_t HandlePeriodExpression(Generator& generator,
                                   Ast::Binary* expression,
                                   std::vector<Bytecode>& code) {
  if (expression->GetOperator() != Ast::Binary::Operator::kMember)
    INTERNAL_ERROR("The expression isn't a period expression.");

  LOGGING_INFO("Period expression: ");

  // Gets the reference of context.
  auto& global_memory = generator.global_memory;
  auto& scopes = generator.context.scopes;
  auto& functions = generator.context.functions;
  auto& variables = generator.context.variables;

  Ast::Expression* handle_expr = expression;
  std::vector<Ast::Expression*> expressions;

  while (handle_expr != nullptr) {
    if (Ast::IsOfType<Ast::Binary>(handle_expr)) {
      // The expression isn't a period expression.
      if (Ast::Cast<Ast::Binary>(handle_expr)->GetOperator() !=
          Ast::Binary::Operator::kMember)
        break;

      // Adds the expression with name to the expressions.
      expressions.insert(
          expressions.begin(),
          Ast::Cast<Ast::Binary>(handle_expr)->GetRightExpression());

      // Gets the next (left) expression.
      handle_expr = Ast::Cast<Ast::Binary>(handle_expr)->GetLeftExpression();
    } else {
      // Adds the last expression with name to the expressions.
      expressions.insert(expressions.begin(), handle_expr);
      handle_expr = nullptr;
    }
  }

  // is_end equals true, indicating that the front-end of the expression
  // (excluding the rightmost expression) is entirely composed of identifiers
  // and does not contain any member functions.
  bool is_end = true;
  std::string full_name;
  // expressions.size() - 1 is used to remove the rightmost expression.
  for (std::size_t i = 0; i < expressions.size() - 1; i++) {
    // Special structures containing member functions or other non identifiers.
    if (!Ast::IsOfType<Ast::Identifier>(expressions[i])) {
      is_end = false;
      break;
    }

    full_name += std::string(*Ast::Cast<Ast::Identifier>(expressions[i])) + ".";
  }

  // Resolve the complete identifier expression and try to obtain as many
  // recognizable variable names as possible.
  // If the conditions are met, return early.
  if (is_end) {
    if (Ast::IsOfType<Ast::Function>(expressions.back())) {
      Ast::Function* right_expression =
          Ast::Cast<Ast::Function>(expressions.back());
      full_name += right_expression->GetFunctionName();

      for (int64_t k = scopes.size() - 1; k >= 0; k--) {
        auto iterator = functions.find(scopes[k] + "." + full_name);

        // The function has been found. If not found, it means that the middle
        // part of the expression is not entirely scope, but contains member
        // functions of the class.
        if (iterator != functions.end()) {
          full_name = scopes[k] + "." + full_name;

          // Adds the function name, return value and its reference into the
          // global memory.
          std::size_t name_index = global_memory.AddString(full_name);
          std::size_t return_value_index =
              HandleFunctionReturnValue(generator, code);

          // Handles the function arguments.
          auto arguments = right_expression->GetParameters();
          std::size_t arguments_size = arguments.size();

          // Handles the invocation of the function.
          std::vector<std::size_t> invoke_arguments = {
              2, name_index, arguments_size + 1, return_value_index};
          for (std::size_t i = 0; i < arguments_size; i++)
            invoke_arguments.push_back(
                HandleExpression(generator, arguments[i], code));

          code.push_back(
              Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_arguments));
          return return_value_index;
        }
      }

    } else if (Ast::Cast<Ast::Identifier>(expressions.back())) {
      // The last expression is an identifier, so we try to find the variable
      // in the scopes. If not found, it means that the middle part of the
      // expression is not entirely scope, but contains member variables of the
      // class.
      full_name += std::string(*Ast::Cast<Ast::Identifier>(expressions.back()));
      for (int64_t i = scopes.size() - 1; i >= 0; i--) {
        auto iterator = variables.find(scopes[i] + "." + full_name);
        if (iterator != variables.end()) return iterator->second;
      }

    } else {
      LOGGING_ERROR("Unsupported statement type.");
    }
  }

  // Handles non all scope (including variables or member functions of a class)
  // expressions.
  Ast::Expression* right_expression = expression->GetRightExpression();
  switch (right_expression->GetStatementType()) {
    // Handles the case where the right expression is a function.
    // This is a function call, so we need to handle it specially.
    case Ast::Statement::StatementType::kFunction: {
      // Handles the class and function name.
      std::size_t class_index =
          HandleExpression(generator, expression->GetLeftExpression(), code);
      std::size_t function_name_index = global_memory.AddString(
          Ast::Cast<Ast::Function>(right_expression)->GetFunctionName());

      // Handles the function return value.
      std::size_t return_value_index =
          HandleFunctionReturnValue(generator, code);

      // Handles the function arguments.
      auto arguments =
          Ast::Cast<Ast::Function>(right_expression)->GetParameters();
      std::size_t arguments_size = arguments.size();

      // Handles the invocation of the function.
      std::vector<std::size_t> invoke_arguments{
          class_index, function_name_index, arguments_size + 1,
          return_value_index};
      for (std::size_t i = 0; i < arguments_size; i++)
        invoke_arguments.push_back(
            HandleExpression(generator, arguments[i], code));

      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_arguments));
      break;
    }

    case Ast::Statement::StatementType::kIdentifier: {
      std::size_t return_value_index = global_memory.Add(1);

      // Handles the class and variable name.
      std::size_t class_index =
          HandleExpression(generator, expression->GetLeftExpression(), code);
      std::size_t variable_name_index = global_memory.AddString(std::string(
          *Ast::Cast<Ast::Identifier>(expression->GetRightExpression())));

      code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, return_value_index,
                              class_index, variable_name_index));
      return return_value_index;
    }

    default:
      LOGGING_ERROR("Unsupported expression type.");
      break;
  }
  return 0;
}

std::size_t HandleFunctionInvoke(Generator& generator, Ast::Function* function,
                                 std::vector<Bytecode>& code) {
  if (function == nullptr) INTERNAL_ERROR("function is nullptr.");

  // Gets the reference of context.
  auto& global_memory = generator.global_memory;
  auto& scopes = generator.context.scopes;
  auto& functions = generator.context.functions;

  std::string function_name = function->GetFunctionName();
  auto arguments = function->GetParameters();

  for (int64_t i = scopes.size() - 1; i >= -1; i--) {
    auto iterator = functions.find(function_name);

    // Use the technique of reprocessing names to prevent scope overflow.
    if (i != -1) iterator = functions.find(scopes[i] + "." + function_name);

    if (iterator != functions.end()) {
      // Use the technique of reprocessing names to prevent scope overflow.
      if (i != -1) function_name = scopes[i] + "." + function_name;
      break;
    }

    if (i == -1) LOGGING_ERROR("Function not found.");
  }

  // Handles the function return value.
  std::size_t return_value_index = HandleFunctionReturnValue(generator, code);

  // Handles the arguments of the functions.
  std::vector<std::size_t> vm_arguments{
      2, global_memory.AddString(function_name), arguments.size() + 1,
      return_value_index};
  for (std::size_t i = 0; i < arguments.size(); i++)
    vm_arguments.push_back(HandleExpression(generator, arguments[i], code));

  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, vm_arguments));

  return return_value_index;
}

std::size_t AddConstInt8t(Generator& generator, int8_t value) {
  return generator.global_memory.AddByte(value);
}

std::size_t HandleFunctionReturnValue(Generator& generator,
                                      std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& global_memory = generator.global_memory;

  std::size_t return_value_index = global_memory.Add(1);
  std::size_t return_value_reference_index = global_memory.Add(1);
  code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, return_value_reference_index,
                          return_value_index));

  return return_value_reference_index;
}

std::size_t GetIndex(Generator& generator, Ast::Expression* expression,
                     std::vector<Bytecode>& code) {
  if (expression == nullptr) INTERNAL_ERROR("expression is nullptr.");

  // Gets the reference of context.
  auto& global_memory = generator.global_memory;
  auto& scopes = generator.context.scopes;
  auto& variables = generator.context.variables;
  auto& current_scope = generator.context.function_context->current_scope;
  auto& current_class = generator.context.current_class;

  if (current_class != nullptr) {
    std::size_t index = GetClassIndex(generator, expression, code);
    if (index != 0) return index;
  }

  switch (expression->GetStatementType()) {
    case Ast::Statement::StatementType::kIdentifier: {
      LOGGING_INFO(std::string(*Ast::Cast<Ast::Identifier>(expression)));
      for (int64_t i = scopes.size() - 1; i >= -1; i--) {
        auto iterator = variables.find(
            std::string(*Ast::Cast<Ast::Identifier>(expression)));

        // Use the technique of reprocessing names to prevent scope overflow.
        if (i != -1)
          iterator = variables.find(
              scopes[i] + "#" +
              std::string(*Ast::Cast<Ast::Identifier>(expression)));

        if (iterator != variables.end()) {
          // Use the technique of reprocessing names to prevent scope overflow.
          return iterator->second;
        }

        LOGGING_ERROR("Identifier not found.");
        break;
      }
    }

    case Ast::Statement::StatementType::kValue: {
      std::size_t vm_type = Ast::Cast<Ast::Value>(expression)->GetVmType();
      switch (vm_type) {
        case 0x01: {
          int8_t value = Ast::Cast<Ast::Value>(expression)->GetByteValue();
          return global_memory.AddByte(value);
          break;
        }

        case 0x02: {
          int64_t value = Ast::Cast<Ast::Value>(expression)->GetLongValue();
          return global_memory.AddLong(value);
        }

        case 0x03: {
          double value = Ast::Cast<Ast::Value>(expression)->GetDoubleValue();
          return global_memory.AddDouble(value);
        }

        case 0x04: {
          uint64_t value = Ast::Cast<Ast::Value>(expression)->GetUInt64Value();
          return global_memory.AddUint64t(value);
        }

        case 0x05: {
          std::string value =
              Ast::Cast<Ast::Value>(expression)->GetStringValue();
          std::size_t str_index = global_memory.AddString(value);
          return str_index;
        }

        default:
          LOGGING_ERROR("Unexpected value vm type.");
          break;
      }
    }

    case Ast::Statement::StatementType::kFunction:
      return HandleFunctionInvoke(generator,
                                  Ast::Cast<Ast::Function>(expression), code);

    default:
      LOGGING_ERROR("Unexpected expression type.");
  }

  return 0;
}

std::size_t GetClassIndex(Generator& generator, Ast::Expression* expression,
                          std::vector<Bytecode>& code) {
  if (expression == nullptr) INTERNAL_ERROR("expression is nullptr.");

  // Gets the reference of context.
  auto& global_memory = generator.global_memory;
  auto& scopes = generator.context.scopes;
  auto& variables = generator.context.variables;
  auto& current_scope = generator.context.function_context->current_scope;
  auto& current_class = generator.context.current_class;

  switch (expression->GetStatementType()) {
    case Ast::Statement::StatementType::kIdentifier: {
      std::size_t index = 0;

      std::string variable_name =
          std::string(*Ast::Cast<Ast::Identifier>(expression));

      // Check if the current class has the variable.
      if (current_class != nullptr &&
          current_class->GetVariable(variable_name, index)) {
        // Gets the reference of the variable index.
        std::size_t return_index = global_memory.Add(1);
        code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, return_index, 0,
                                global_memory.AddString(variable_name)));
        return return_index;
      }

      for (int64_t i = scopes.size() - 1; i >= 0; i--) {
        auto iterator = variables.find(scopes[i] + "#" + variable_name);

        // If the variable is found in the current scope, return its index.
        if (iterator != variables.end()) {
          return iterator->second;
        }
      }

      // If the variable is not found in any scope, return 0 and try to get
      // the index from the global memory by GetIndex().
      return 0;
    }

    case Ast::Statement::StatementType::kValue: {
      std::size_t vm_type = Ast::Cast<Ast::Value>(expression)->GetVmType();
      switch (vm_type) {
        case 0x01: {
          int8_t value = Ast::Cast<Ast::Value>(expression)->GetByteValue();
          return global_memory.AddByte(value);
          break;
        }

        case 0x02: {
          int64_t value = Ast::Cast<Ast::Value>(expression)->GetLongValue();
          return global_memory.AddLong(value);
        }

        case 0x03: {
          double value = Ast::Cast<Ast::Value>(expression)->GetDoubleValue();
          return global_memory.AddDouble(value);
        }

        case 0x04: {
          uint64_t value = Ast::Cast<Ast::Value>(expression)->GetUInt64Value();
          return global_memory.AddUint64t(value);
        }

        case 0x05: {
          std::string value =
              Ast::Cast<Ast::Value>(expression)->GetStringValue();
          std::size_t str_index = global_memory.AddString(value);
          return str_index;
        }

        default:
          LOGGING_ERROR("Unexpected value vm type.");
          break;
      }
    }

    case Ast::Statement::StatementType::kFunction:
      return HandleFunctionInvoke(generator,
                                  Ast::Cast<Ast::Function>(expression), code);

    default:
      return 0;
  }

  return 0;
}
}  // namespace Generator
}  // namespace Aq