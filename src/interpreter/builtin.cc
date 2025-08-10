// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/builtin.h"

#include <string>

#include "interpreter/interpreter.h"
#include "interpreter/operator.h"

namespace Aq {
namespace Interpreter {
void AddBuiltInFunctionDeclaration(
    Interpreter& interpreter, std::string name,
    std::function<int(std::shared_ptr<Memory>, std::vector<std::size_t>)>
        function) {
  interpreter.functions[name].push_back(Function());
  interpreter.builtin_functions[name] = function;
}

void InitBuiltInFunctionDeclaration(Interpreter& interpreter) {
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_print",
                                __builtin_print);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_vaprint",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_abs", __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_open", __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_close", __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_read", __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_write", __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_input", __builtin_void);

  AddBuiltInFunctionDeclaration(interpreter, "__builtin_GUI_CreateWindow",
                                __builtin_void);

  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_acos",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_asin",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_atan",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_atan2",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_ceil",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_cos",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_cosh",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_exp",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_fabs",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_floor",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_fmod",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_frexp",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_hypot",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_ldexp",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_log",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_log10",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_modf",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_pow",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_sin",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_sinh",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_sqrt",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_tan",
                                __builtin_void);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_tanh",
                                __builtin_void);
}

int __builtin_void(std::shared_ptr<Memory> memory,
                   std::vector<std::size_t> arguments) {
  // This function is intentionally left empty.
  // It can be used to indicate that a function does not return a value.
  LOGGING_WARNING(
      "This function has not yet been implemented. __builtin_void is called, "
      "but it does nothing.");
  return 0;
}

int __builtin_print(std::shared_ptr<Memory> memory,
                    std::vector<std::size_t> arguments) {
  if (arguments.size() < 1) {
    LOGGING_ERROR("Not enough arguments for __builtin_print.");
    return -1;
  }

  std::size_t index = arguments[0];
  auto& return_object = memory->GetOriginData(index);
  arguments.erase(arguments.begin());

  for (const auto& argument : arguments) {
    auto& object = memory->GetOriginData(argument);
    switch (object.type[0]) {
      case 0x01:
        SetLong(return_object, printf("%d", GetByte(object)));
        break;
      case 0x02:
        SetLong(return_object, printf("%lld", GetLong(object)));
        break;
      case 0x03:
        SetLong(return_object, printf("%g", GetDouble(object)));
        break;
      case 0x04:
        SetLong(return_object, printf("%llu", GetUint64(object)));
        break;
      case 0x05:
        SetLong(return_object, printf("%s", GetString(object).c_str()));
        break;
      default:
        LOGGING_ERROR("Unsupported object type in __builtin_print: " +
                      std::to_string(object.type[0]));
        return -1;
    }
  }

  return 0;
}

}  // namespace Interpreter
}  // namespace Aq
