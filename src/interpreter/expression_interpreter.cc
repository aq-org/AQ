// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/expression_interpreter.h"

#include "ast/ast.h"
#include "interpreter/declaration_interpreter.h"
#include "interpreter/interpreter.h"
#include "interpreter/operator.h"
#include "interpreter/statement_interpreter.h"
#include "logging/logging.h"

namespace Aq {
namespace Interpreter {
std::size_t HandleExpression(Interpreter& interpreter,
                             Ast::Expression* expression,
                             std::vector<Bytecode>& code,
                             std::size_t result_index) {
  if (expression == nullptr) INTERNAL_ERROR("expression is nullptr.");

  if (Ast::IsOfType<Ast::Unary>(expression) ||
      Ast::IsOfType<Ast::Array>(expression)) {
    return HandleUnaryExpression(interpreter, Ast::Cast<Ast::Unary>(expression),
                                 code, result_index);
  } else if (Ast::IsOfType<Ast::Binary>(expression)) {
    return HandleBinaryExpression(
        interpreter, Ast::Cast<Ast::Binary>(expression), code, result_index);
  }

  return GetIndex(interpreter, expression, code);
}

std::size_t HandleUnaryExpression(Interpreter& interpreter,
                                  Ast::Unary* expression,
                                  std::vector<Bytecode>& code,
                                  std::size_t result_index) {
  if (expression == nullptr) INTERNAL_ERROR("expression is nullptr.");

  // Gets the reference of context.
  auto global_memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;

  std::size_t sub_expression = HandleExpression(
      interpreter, expression->GetExpression(), code, result_index);
  std::size_t new_index =
      result_index == 0 ? global_memory->Add(1) : result_index;

  switch (expression->GetOperator()) {
    case Ast::Unary::Operator::kPostInc: {  // ++ (postfix)
      code.push_back(
          Bytecode{_AQVM_OPERATOR_EQUAL, {new_index, sub_expression}});
      if (global_memory->GetMemory()[sub_expression].type == 0x02) {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_ADDI,
            {sub_expression, sub_expression, global_memory->AddLong(1)}});
      } else if (global_memory->GetMemory()[sub_expression].type == 0x03) {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_ADDF,
            {sub_expression, sub_expression, global_memory->AddDouble(1.0)}});
      } else {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_ADD,
            {sub_expression, sub_expression, AddConstInt8t(interpreter, 1)}});
      }
      return new_index;
    }

    case Ast::Unary::Operator::kPostDec: {  // -- (postfix)
      code.push_back(
          Bytecode{_AQVM_OPERATOR_EQUAL, {new_index, sub_expression}});
      if (global_memory->GetMemory()[sub_expression].type == 0x02) {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_SUBI,
            {sub_expression, sub_expression, global_memory->AddLong(1)}});
      } else if (global_memory->GetMemory()[sub_expression].type == 0x03) {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_SUBF,
            {sub_expression, sub_expression, global_memory->AddDouble(1.0)}});
      } else {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_SUB,
            {sub_expression, sub_expression, AddConstInt8t(interpreter, 1)}});
      }
      return new_index;
    }

    case Ast::Unary::Operator::kPreInc:  // ++ (prefix)
      if (global_memory->GetMemory()[sub_expression].type == 0x02) {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_ADDI,
            {sub_expression, sub_expression, global_memory->AddLong(1)}});
      } else if (global_memory->GetMemory()[sub_expression].type == 0x03) {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_ADDF,
            {sub_expression, sub_expression, global_memory->AddDouble(1.0)}});
      } else {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_ADD,
            {sub_expression, sub_expression, AddConstInt8t(interpreter, 1)}});
      }
      return sub_expression;

    case Ast::Unary::Operator::kPreDec:  // -- (prefix)
      if (global_memory->GetMemory()[sub_expression].type == 0x02) {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_SUBI,
            {sub_expression, sub_expression, global_memory->AddLong(1)}});
      } else if (global_memory->GetMemory()[sub_expression].type == 0x03) {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_SUBF,
            {sub_expression, sub_expression, global_memory->AddDouble(1.0)}});
      } else {
        code.push_back(Bytecode{
            _AQVM_OPERATOR_SUB,
            {sub_expression, sub_expression, AddConstInt8t(interpreter, 1)}});
      }
      return sub_expression;

    case Ast::Unary::Operator::kPlus:  // + (unary plus)
      // Unary plus does not change the value, so we can just return the
      // sub_expression index.
      return sub_expression;

    case Ast::Unary::Operator::kMinus:
    case Ast::Unary::Operator::kNot: {  // - (unary minus) or ! (logical NOT)

      code.push_back(Bytecode{_AQVM_OPERATOR_NEG, {new_index, sub_expression}});
      return new_index;
    }

    case Ast::Unary::Operator::ARRAY: {  // []
      std::size_t offset = HandleExpression(
          interpreter, Ast::Cast<Ast::Array>(expression)->GetIndexExpression(),
          code, 0);

      code.push_back(
          Bytecode{_AQVM_OPERATOR_ARRAY, {new_index, sub_expression, offset}});
      return new_index;
    }

    case Ast::Unary::Operator::kBitwiseNot:  // ~ (bitwise NOT)
    // TODO: Implement bitwise NOT operator.
    default:
      LOGGING_WARNING("Unexpected unary operator is not implemented yet.");
      return sub_expression;
  }
}

std::size_t HandleBinaryExpression(Interpreter& interpreter,
                                   Ast::Binary* expression,
                                   std::vector<Bytecode>& code,
                                   std::size_t result_index) {
  if (expression == nullptr) INTERNAL_ERROR("expression is nullptr.");

  // Gets the reference of context.
  auto global_memory = interpreter.global_memory;

  // Gets the reference of expressions.
  Ast::Expression* right_expression = expression->GetRightExpression();
  Ast::Expression* left_expression = expression->GetLeftExpression();

  // Gets the left and right expression indexes.
  std::size_t left = 0;
  std::size_t right = 0;
  if (expression->GetOperator() != Ast::Binary::Operator::kMember &&
      expression->GetOperator() != Ast::Binary::Operator::kAssign) {
    right = HandleExpression(interpreter, right_expression, code, 0);
    left = HandleExpression(interpreter, left_expression, code, 0);
  }

  uint8_t type = global_memory->GetMemory()[left].type >
                         global_memory->GetMemory()[right].type
                     ? global_memory->GetMemory()[left].type
                     : global_memory->GetMemory()[right].type;

  if (expression->GetOperator() == Ast::Binary::Operator::kLT ||
      expression->GetOperator() == Ast::Binary::Operator::kGT ||
      expression->GetOperator() == Ast::Binary::Operator::kLE ||
      expression->GetOperator() == Ast::Binary::Operator::kGE ||
      expression->GetOperator() == Ast::Binary::Operator::kEQ ||
      expression->GetOperator() == Ast::Binary::Operator::kNE)
    type = 0;

  std::size_t result =
      result_index == 0 ? global_memory->AddWithType(type) : result_index;
  switch (expression->GetOperator()) {
    case Ast::Binary::Operator::kAdd:  // +
      if (global_memory->GetMemory()[result].type == 0x02 &&
          global_memory->GetMemory()[left].type == 0x02 &&
          global_memory->GetMemory()[right].type == 0x02) {
        code.push_back(Bytecode{_AQVM_OPERATOR_ADDI, {result, left, right}});
      } else if (global_memory->GetMemory()[result].type == 0x03 &&
                 global_memory->GetMemory()[left].type == 0x03 &&
                 global_memory->GetMemory()[right].type == 0x03) {
        code.push_back(Bytecode{_AQVM_OPERATOR_ADDF, {result, left, right}});
      } else {
        code.push_back(Bytecode{_AQVM_OPERATOR_ADD, {result, left, right}});
      }

      return result;

    case Ast::Binary::Operator::kSub:  // -
      if (global_memory->GetMemory()[result].type == 0x02 &&
          global_memory->GetMemory()[left].type == 0x02 &&
          global_memory->GetMemory()[right].type == 0x02) {
        code.push_back(Bytecode{_AQVM_OPERATOR_SUBI, {result, left, right}});
      } else if (global_memory->GetMemory()[result].type == 0x03 &&
                 global_memory->GetMemory()[left].type == 0x03 &&
                 global_memory->GetMemory()[right].type == 0x03) {
        code.push_back(Bytecode{_AQVM_OPERATOR_SUBF, {result, left, right}});
      } else {
        code.push_back(Bytecode{_AQVM_OPERATOR_SUB, {result, left, right}});
      }
      return result;

    case Ast::Binary::Operator::kMul:  // *
      if (global_memory->GetMemory()[result].type == 0x02 &&
          global_memory->GetMemory()[left].type == 0x02 &&
          global_memory->GetMemory()[right].type == 0x02) {
        code.push_back(Bytecode{_AQVM_OPERATOR_MULI, {result, left, right}});
      } else if (global_memory->GetMemory()[result].type == 0x03 &&
                 global_memory->GetMemory()[left].type == 0x03 &&
                 global_memory->GetMemory()[right].type == 0x03) {
        code.push_back(Bytecode{_AQVM_OPERATOR_MULF, {result, left, right}});
      } else {
        code.push_back(Bytecode{_AQVM_OPERATOR_MUL, {result, left, right}});
      }
      return result;

    case Ast::Binary::Operator::kDiv:  // /
      if (global_memory->GetMemory()[result].type == 0x02 &&
          global_memory->GetMemory()[left].type == 0x02 &&
          global_memory->GetMemory()[right].type == 0x02) {
        code.push_back(Bytecode{_AQVM_OPERATOR_DIVI, {result, left, right}});
      } else if (global_memory->GetMemory()[result].type == 0x03 &&
                 global_memory->GetMemory()[left].type == 0x03 &&
                 global_memory->GetMemory()[right].type == 0x03) {
        code.push_back(Bytecode{_AQVM_OPERATOR_DIVF, {result, left, right}});
      } else {
        code.push_back(Bytecode{_AQVM_OPERATOR_DIV, {result, left, right}});
      }
      return result;

    case Ast::Binary::Operator::kRem:  // %
      if (global_memory->GetMemory()[result].type == 0x02 &&
          global_memory->GetMemory()[left].type == 0x02 &&
          global_memory->GetMemory()[right].type == 0x02) {
        code.push_back(Bytecode{_AQVM_OPERATOR_REMI, {result, left, right}});
      } else {
        code.push_back(Bytecode{_AQVM_OPERATOR_REM, {result, left, right}});
      }
      return result;

    case Ast::Binary::Operator::kAnd:  // &
      code.push_back(Bytecode{_AQVM_OPERATOR_AND, {result, left, right}});
      return result;

    case Ast::Binary::Operator::kOr:  // |
      code.push_back(Bytecode{_AQVM_OPERATOR_OR, {result, left, right}});
      return result;

    case Ast::Binary::Operator::kXor:  // ^
      code.push_back(Bytecode{_AQVM_OPERATOR_XOR, {result, left, right}});
      return result;

    case Ast::Binary::Operator::kShl:  // <<
      code.push_back(Bytecode{_AQVM_OPERATOR_SHL, {result, left, right}});
      return result;

    case Ast::Binary::Operator::kShr:  // >>
      code.push_back(Bytecode{_AQVM_OPERATOR_SHR, {result, left, right}});
      return result;

    case Ast::Binary::Operator::kLT:  // <
      code.push_back(Bytecode{_AQVM_OPERATOR_CMP,
                              {result, (std::size_t)0x04, left, right}});
      return result;

    case Ast::Binary::Operator::kGT:  // >
      code.push_back(Bytecode{_AQVM_OPERATOR_CMP,
                              {result, (std::size_t)0x02, left, right}});
      return result;

    case Ast::Binary::Operator::kLE:  // <=
      code.push_back(Bytecode{_AQVM_OPERATOR_CMP,
                              {result, (std::size_t)0x05, left, right}});
      return result;

    case Ast::Binary::Operator::kGE:  // >=
      code.push_back(Bytecode{_AQVM_OPERATOR_CMP,
                              {result, (std::size_t)0x03, left, right}});
      return result;

    case Ast::Binary::Operator::kEQ:  // ==
      code.push_back(Bytecode{_AQVM_OPERATOR_CMP,
                              {result, (std::size_t)0x00, left, right}});
      return result;

    case Ast::Binary::Operator::kNE:  // !=
      code.push_back(Bytecode{_AQVM_OPERATOR_CMP,
                              {result, (std::size_t)0x01, left, right}});
      return result;

    case Ast::Binary::Operator::kLAnd:  // &&
      code.push_back(Bytecode{_AQVM_OPERATOR_AND, {result, left, right}});
      return result;

    case Ast::Binary::Operator::kLOr:  // ||
      code.push_back(Bytecode{_AQVM_OPERATOR_OR, {result, left, right}});
      return result;

    case Ast::Binary::Operator::kAssign:  // =
      left = HandleExpression(interpreter, left_expression, code, result);
      right = HandleExpression(interpreter, right_expression, code, left);
      if (right != left)
        code.push_back(Bytecode{_AQVM_OPERATOR_EQUAL, {left, right}});
      return left;

    case Ast::Binary::Operator::kAddAssign:  // +=
      code.push_back(Bytecode{_AQVM_OPERATOR_ADD, {left, left, right}});
      return left;

    case Ast::Binary::Operator::kSubAssign:  // -=
      code.push_back(Bytecode{_AQVM_OPERATOR_SUB, {left, left, right}});
      return left;

    case Ast::Binary::Operator::kMulAssign:  // *=
      code.push_back(Bytecode{_AQVM_OPERATOR_MUL, {left, left, right}});
      return left;

    case Ast::Binary::Operator::kDivAssign:  // /=
      code.push_back(Bytecode{_AQVM_OPERATOR_DIV, {left, left, right}});
      return left;

    case Ast::Binary::Operator::kRemAssign:  // %=
      code.push_back(Bytecode{_AQVM_OPERATOR_REM, {left, left, right}});
      return left;

    case Ast::Binary::Operator::kAndAssign:  // &=
      code.push_back(Bytecode{_AQVM_OPERATOR_AND, {left, left, right}});
      return left;

    case Ast::Binary::Operator::kOrAssign:  // |=
      code.push_back(Bytecode{_AQVM_OPERATOR_OR, {left, left, right}});
      return left;

    case Ast::Binary::Operator::kXorAssign:  // ^=
      code.push_back(Bytecode{_AQVM_OPERATOR_XOR, {left, left, right}});
      return left;

    case Ast::Binary::Operator::kShlAssign:  // <<=
      code.push_back(Bytecode{_AQVM_OPERATOR_SHL, {left, left, right}});
      return left;

    case Ast::Binary::Operator::kShrAssign:  // >>=
      code.push_back(Bytecode{_AQVM_OPERATOR_SHR, {left, left, right}});
      return left;

    case Ast::Binary::Operator::kMember: {
      return HandlePeriodExpression(interpreter, expression, code,
                                    result_index);
      break;
    }

    default:
      LOGGING_ERROR("Unexpected binary operator.");
      break;
  }

  LOGGING_ERROR("Unexpected binary operator.");
  return 0;
}

std::size_t HandlePeriodExpression(Interpreter& interpreter,
                                   Ast::Binary* expression,
                                   std::vector<Bytecode>& code,
                                   std::size_t result_index) {
  if (expression->GetOperator() != Ast::Binary::Operator::kMember)
    INTERNAL_ERROR("The expression isn't a period expression.");

  // Gets the reference of context.
  auto global_memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;
  auto& functions = interpreter.functions;
  auto& variables = interpreter.context.variables;

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
          std::size_t name_index = global_memory->AddString(full_name);
          std::size_t return_value_index =
              HandleFunctionReturnValue(interpreter, code);

          // Handles the function arguments.
          auto arguments = right_expression->GetParameters();
          std::size_t arguments_size = arguments.size();

          // Handles the invocation of the function.
          std::vector<std::size_t> invoke_arguments = {2, name_index,
                                                       return_value_index};
          for (std::size_t i = 0; i < arguments_size; i++)
            invoke_arguments.push_back(
                HandleExpression(interpreter, arguments[i], code, 0));

          code.push_back(Bytecode{_AQVM_OPERATOR_INVOKE_METHOD,
                                  std::move(invoke_arguments)});
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
      std::size_t class_index = HandleExpression(
          interpreter, expression->GetLeftExpression(), code, 0);
      std::size_t function_name_index = global_memory->AddString(
          Ast::Cast<Ast::Function>(right_expression)->GetFunctionName());

      // Handles the function return value.
      std::size_t return_value_index =
          HandleFunctionReturnValue(interpreter, code);

      // Handles the function arguments.
      auto arguments =
          Ast::Cast<Ast::Function>(right_expression)->GetParameters();
      std::size_t arguments_size = arguments.size();

      // Handles the invocation of the function.
      std::vector<std::size_t> invoke_arguments{
          class_index, function_name_index, return_value_index};
      for (std::size_t i = 0; i < arguments_size; i++)
        invoke_arguments.push_back(
            HandleExpression(interpreter, arguments[i], code, 0));

      code.push_back(
          Bytecode{_AQVM_OPERATOR_INVOKE_METHOD, std::move(invoke_arguments)});
      return return_value_index;
    }

    case Ast::Statement::StatementType::kIdentifier: {
      std::size_t return_value_index = global_memory->Add(1);

      // Handles the class and variable name.
      std::size_t class_index = HandleExpression(
          interpreter, expression->GetLeftExpression(), code, 0);
      std::size_t variable_name_index = global_memory->AddString(std::string(
          *Ast::Cast<Ast::Identifier>(expression->GetRightExpression())));

      code.push_back(
          Bytecode{_AQVM_OPERATOR_LOAD_MEMBER,
                   {return_value_index, class_index, variable_name_index}});
      return return_value_index;
    }

    default:
      LOGGING_ERROR("Unsupported expression type.");
      break;
  }
  return 0;
}

std::size_t HandleFunctionInvoke(Interpreter& interpreter,
                                 Ast::Function* function,
                                 std::vector<Bytecode>& code,
                                 std::size_t result_index) {
  if (function == nullptr) INTERNAL_ERROR("function is nullptr.");

  // Gets the reference of context.
  auto global_memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;
  auto& functions = interpreter.functions;
  auto& classes = interpreter.classes;

  std::string function_name = function->GetFunctionName();
  auto arguments = function->GetParameters();

  // Check if this is a class instantiation (constructor call).
  // Try to find the class name with and without scopes.
  std::string class_name_to_check;
  bool is_class_instantiation = false;
  
  for (int64_t i = scopes.size() - 1; i >= -1; i--) {
    std::string scoped_name = function_name;
    if (i != -1) scoped_name = scopes[i] + "." + function_name;
    
    // Check if this name matches a class.
    if (classes.find(scoped_name) != classes.end()) {
      is_class_instantiation = true;
      class_name_to_check = scoped_name;
      break;
    }
  }

  // If this is a class instantiation, handle it as such.
  if (is_class_instantiation) {
    // Handles the return value (the new class instance).
    std::size_t return_value_index = HandleFunctionReturnValue(interpreter, code);

    // Create a new instance of the class using NEW operator.
    code.push_back(
        Bytecode{_AQVM_OPERATOR_NEW,
                 {return_value_index, global_memory->AddByte(0),
                  global_memory->AddString(class_name_to_check)}});

    // Call the constructor with the provided arguments.
    // Constructors are stored with the special name "@constructor".
    std::vector<std::size_t> constructor_arguments{
        return_value_index, global_memory->AddString("@constructor"),
        global_memory->Add(1)};
    for (std::size_t i = 0; i < arguments.size(); i++)
      constructor_arguments.push_back(
          HandleExpression(interpreter, arguments[i], code, 0));

    code.push_back(Bytecode{_AQVM_OPERATOR_INVOKE_METHOD,
                            std::move(constructor_arguments)});

    return return_value_index;
  }

  // Otherwise, handle it as a regular function call.
  // First, check if function_name is a variable containing a function reference
  auto& variables = interpreter.context.variables;
  std::size_t function_ref_index = 0;
  bool is_function_variable = false;

  // Check if the function name is actually a variable holding a function reference
  for (int64_t i = scopes.size() - 1; i >= -1; i--) {
    std::string var_name = function_name;
    if (i != -1) var_name = scopes[i] + "#" + function_name;

    auto var_iterator = variables.find(var_name);
    if (var_iterator != variables.end()) {
      function_ref_index = var_iterator->second;
      // Check if this variable contains a string (function reference)
      if (global_memory->GetMemory()[function_ref_index].type == 0x05) {
        is_function_variable = true;
        // Get the actual function name from the variable
        function_name = *global_memory->GetMemory()[function_ref_index].data.string_data;
        break;
      }
    }
  }

  // If not a function variable, look for the function directly
  if (!is_function_variable) {
    bool found = false;
    for (int64_t i = scopes.size() - 1; i >= -1; i--) {
      std::string lookup_name = function_name;
      if (i != -1) lookup_name = scopes[i] + "." + function_name;
      
      auto iterator = functions.find(lookup_name);

      if (iterator != functions.end()) {
        function_name = lookup_name;
        found = true;
        break;
      }
    }
    
    if (!found) {
      LOGGING_ERROR("Function '" + function_name + "' not found.");
    }
  }

  // Handles the function return value.
  std::size_t return_value_index = HandleFunctionReturnValue(interpreter, code);

  // Handles the arguments of the functions.
  std::vector<std::size_t> vm_arguments{
      2, global_memory->AddString(function_name), return_value_index};
  for (std::size_t i = 0; i < arguments.size(); i++)
    vm_arguments.push_back(
        HandleExpression(interpreter, arguments[i], code, 0));

  code.push_back(
      Bytecode{_AQVM_OPERATOR_INVOKE_METHOD, std::move(vm_arguments)});

  return return_value_index;
}

std::size_t AddConstInt8t(Interpreter& interpreter, int8_t value) {
  return interpreter.global_memory->AddByte(value);
}

std::size_t HandleFunctionReturnValue(Interpreter& interpreter,
                                      std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto global_memory = interpreter.global_memory;

  std::size_t return_value_index = global_memory->Add(1);

  return return_value_index;
}

std::size_t GetIndex(Interpreter& interpreter, Ast::Expression* expression,
                     std::vector<Bytecode>& code) {
  if (expression == nullptr) INTERNAL_ERROR("expression is nullptr.");

  // Gets the reference of context.
  auto global_memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;
  auto& variables = interpreter.context.variables;
  auto& current_scope = interpreter.context.function_context->current_scope;
  auto& current_class = interpreter.context.current_class;

  if (current_class != nullptr) {
    std::size_t index = GetClassIndex(interpreter, expression, code);
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
      }
      LOGGING_ERROR("Identifier not found.");
      break;
    }

    case Ast::Statement::StatementType::kValue: {
      std::size_t vm_type = Ast::Cast<Ast::Value>(expression)->GetVmType();
      switch (vm_type) {
        case 0x01: {
          int8_t value = Ast::Cast<Ast::Value>(expression)->GetByteValue();
          return global_memory->AddByte(value);
          break;
        }

        case 0x02: {
          int64_t value = Ast::Cast<Ast::Value>(expression)->GetLongValue();
          return global_memory->AddLong(value);
        }

        case 0x03: {
          double value = Ast::Cast<Ast::Value>(expression)->GetDoubleValue();
          return global_memory->AddDouble(value);
        }

        case 0x04: {
          uint64_t value = Ast::Cast<Ast::Value>(expression)->GetUInt64Value();
          return global_memory->AddUint64t(value);
        }

        case 0x05: {
          std::string value =
              Ast::Cast<Ast::Value>(expression)->GetStringValue();
          std::size_t str_index = global_memory->AddString(value);
          return str_index;
        }

        default:
          LOGGING_ERROR("Unexpected value vm type.");
          break;
      }
    }

    case Ast::Statement::StatementType::kFunction:
      return HandleFunctionInvoke(
          interpreter, Ast::Cast<Ast::Function>(expression), code, 0);

    case Ast::Statement::StatementType::kLambda:
      return HandleLambdaExpression(
          interpreter, Ast::Cast<Ast::Lambda>(expression), code);

    case Ast::Statement::StatementType::kArrayDeclaration:
      return HandleArrayDeclaration(
          interpreter, Ast::Cast<Ast::ArrayDeclaration>(expression), code);

    default:
      LOGGING_ERROR("Unexpected expression type.");
  }

  return 0;
}

std::size_t GetClassIndex(Interpreter& interpreter, Ast::Expression* expression,
                          std::vector<Bytecode>& code) {
  if (expression == nullptr) INTERNAL_ERROR("expression is nullptr.");

  // Gets the reference of context.
  auto global_memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;
  auto& variables = interpreter.context.variables;
  auto& current_scope = interpreter.context.function_context->current_scope;
  auto& current_class = interpreter.context.current_class;

  switch (expression->GetStatementType()) {
    case Ast::Statement::StatementType::kIdentifier: {
      std::string variable_name =
          std::string(*Ast::Cast<Ast::Identifier>(expression));

      Object temp;

      // Check if the current class has the variable.
      if (current_class != nullptr && current_class->GetName() != ".!__start" &&
          current_class->GetVariable(variable_name, temp)) {
        // Gets the reference of the variable index.
        std::size_t return_index = global_memory->Add(1);
        code.push_back(Bytecode{
            _AQVM_OPERATOR_LOAD_MEMBER,
            {return_index, 0, global_memory->AddString(variable_name)}});
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
          return global_memory->AddByte(value);
          break;
        }

        case 0x02: {
          int64_t value = Ast::Cast<Ast::Value>(expression)->GetLongValue();
          return global_memory->AddLong(value);
        }

        case 0x03: {
          double value = Ast::Cast<Ast::Value>(expression)->GetDoubleValue();
          return global_memory->AddDouble(value);
        }

        case 0x04: {
          uint64_t value = Ast::Cast<Ast::Value>(expression)->GetUInt64Value();
          return global_memory->AddUint64t(value);
        }

        case 0x05: {
          std::string value =
              Ast::Cast<Ast::Value>(expression)->GetStringValue();
          std::size_t str_index = global_memory->AddString(value);
          return str_index;
        }

        default:
          LOGGING_ERROR("Unexpected value vm type.");
          break;
      }
    }

    case Ast::Statement::StatementType::kFunction:
      return HandleFunctionInvoke(
          interpreter, Ast::Cast<Ast::Function>(expression), code, 0);

    default:
      return 0;
  }

  return 0;
}

std::size_t HandleLambdaExpression(Interpreter& interpreter,
                                   Ast::Lambda* lambda,
                                   std::vector<Bytecode>& code) {
  if (lambda == nullptr) INTERNAL_ERROR("lambda is nullptr.");

  // Gets the reference of context.
  auto global_memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;
  auto& variables = interpreter.context.variables;

  // Generate a unique name for the lambda function
  static std::size_t lambda_counter = 0;
  std::string lambda_name = "__lambda_" + std::to_string(lambda_counter++);

  // Save current function context
  FunctionContext* saved_context = interpreter.context.function_context;
  FunctionContext new_context;
  interpreter.context.function_context = &new_context;

  // Create a new scope for the lambda
  std::string lambda_scope = lambda_name;
  scopes.push_back(lambda_scope);

  // Parse lambda parameters
  std::vector<std::size_t> parameters_index;
  std::vector<Bytecode> lambda_code;

  for (auto param : lambda->GetParameters()) {
    Ast::Type* param_type = param->GetVariableType();
    std::string param_name = param->GetVariableName();
    
    // Add parameter to variables
    std::size_t param_index = global_memory->Add(param_type->GetVmType());
    variables[lambda_scope + "#" + param_name] = param_index;
    parameters_index.push_back(param_index);
  }

  // Process lambda body
  for (auto statement : lambda->GetBody()->GetStatements()) {
    HandleStatement(interpreter, statement, lambda_code);
  }

  // Handle return if no explicit return
  HandleReturnInHandlingFunction(interpreter, lambda_code);
  HandleGotoInHandlingFunction(interpreter, new_context.current_scope,
                               lambda_code);

  // Create function object
  Function lambda_func(lambda_name, parameters_index, lambda_code);
  if (lambda->IsVariadic()) {
    lambda_func.EnableVariadic();
  }

  // Add lambda function to interpreter
  interpreter.functions[lambda_name].push_back(lambda_func);

  // Restore context
  scopes.pop_back();
  interpreter.context.function_context = saved_context;

  // Store function reference in memory (as a function pointer)
  // For now, we'll store the function name as a string
  std::size_t func_index = global_memory->AddString(lambda_name);
  
  return func_index;
}
}  // namespace Interpreter
}  // namespace Aq