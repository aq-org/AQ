// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/builtin.h"

#include <cmath>
#include <cstdio>
#include <string>

#include "interpreter/interpreter.h"
#include "interpreter/memory.h"

namespace Aq {
namespace Interpreter {
void AddBuiltInFunctionDeclaration(
    Interpreter& interpreter, std::string name,
    std::function<int(Memory*, std::vector<std::size_t>)> function) {
  interpreter.functions[name].push_back(Function());
  interpreter.builtin_functions[name] = function;
}

void InitBuiltInFunctionDeclaration(Interpreter& interpreter) {
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_print",
                                __builtin_print);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_vaprint",
                                __builtin_vaprint);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_abs", __builtin_abs);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_open", __builtin_open);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_close",
                                __builtin_close);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_read", __builtin_read);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_write",
                                __builtin_write);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_input",
                                __builtin_input);

  AddBuiltInFunctionDeclaration(interpreter, "__builtin_GUI_CreateWindow",
                                __builtin_void);

  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_acos",
                                __builtin_math_acos);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_asin",
                                __builtin_math_asin);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_atan",
                                __builtin_math_atan);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_atan2",
                                __builtin_math_atan2);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_ceil",
                                __builtin_math_ceil);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_cos",
                                __builtin_math_cos);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_cosh",
                                __builtin_math_cosh);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_exp",
                                __builtin_math_exp);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_fabs",
                                __builtin_math_fabs);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_floor",
                                __builtin_math_floor);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_fmod",
                                __builtin_math_fmod);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_frexp",
                                __builtin_math_frexp);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_hypot",
                                __builtin_math_hypot);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_ldexp",
                                __builtin_math_ldexp);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_log",
                                __builtin_math_log);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_log10",
                                __builtin_math_log10);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_modf",
                                __builtin_math_modf);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_pow",
                                __builtin_math_pow);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_sin",
                                __builtin_math_sin);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_sinh",
                                __builtin_math_sinh);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_sqrt",
                                __builtin_math_sqrt);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_tan",
                                __builtin_math_tan);
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_tanh",
                                __builtin_math_tanh);
}

int __builtin_void(Memory* memory, std::vector<std::size_t> arguments) {
  // This function is intentionally left empty.
  // It can be used to indicate that a function does not return a value.
  LOGGING_WARNING(
      "This function has not yet been implemented. __builtin_void is called, "
      "but it does nothing.");
  return 0;
}

int __builtin_print(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() < 1) {
    LOGGING_ERROR("Not enough arguments for __builtin_print.");
    return -1;
  }

  auto memory_ptr = memory->GetMemory().data();

  arguments.erase(arguments.begin());

  for (const auto& argument : arguments) {
    auto& object = memory->GetOriginData(argument);
    switch (object.type) {
      case 0x01:
        printf("%d", GetByte(memory_ptr + argument));
        break;
      case 0x02:
        printf("%lld", GetLong(memory_ptr + argument));
        break;
      case 0x03:
        printf("%g", GetDouble(memory_ptr + argument));
        break;
      case 0x04:
        printf("%llu", GetUint64(memory_ptr + argument));
        break;
      case 0x05:
        printf("%s", GetString(memory_ptr + argument).c_str());
        break;
      default:
        LOGGING_ERROR("Unsupported object type in __builtin_print: " +
                      std::to_string(object.type));
        return -1;
    }
  }

  return 0;
}

int __builtin_vaprint(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2)
    LOGGING_ERROR(
        "Invalid number of arguments for __builtin_vaprint. Expected 2, got " +
        std::to_string(arguments.size()));

  auto memory_ptr = memory->GetMemory().data();

  std::size_t value_index = arguments[1];
  auto& value_object = memory->GetOriginData(value_index);
  auto array_ptr = GetArray(memory_ptr + value_index)->GetMemory().data();

  for (std::size_t i = 0;
       i < GetArray(memory_ptr + value_index)->GetMemory().size(); i++) {
    switch (array_ptr[i].type) {
      case 0x01:
        printf("%d", GetByte(array_ptr + i));
        break;
      case 0x02:
        printf("%lld", GetLong(array_ptr + i));
        break;
      case 0x03:
        printf("%g", GetDouble(array_ptr + i));
        break;
      case 0x04:
        printf("%llu", GetUint64(array_ptr + i));
        break;
      case 0x05:
        printf("%s", GetString(array_ptr + i).c_str());
        break;
      default:
        LOGGING_ERROR("Unsupported object type in __builtin_vaprint: " +
                      std::to_string(array_ptr[i].type));
        return -1;
    }
  }

  return 0;
}

int __builtin_abs(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2)
    LOGGING_ERROR(
        "Invalid number of arguments for __builtin_abs. Expected 2, got " +
        std::to_string(arguments.size()));

  auto memory_ptr = memory->GetMemory().data();

  Object* value_object = GetOrigin(memory_ptr + arguments[1]);

  switch (value_object->type) {
    case 0x01:
      SetByte(memory_ptr + arguments[0], abs(value_object->data.byte_data));
      break;
    case 0x02:
      SetLong(memory_ptr + arguments[0], abs(value_object->data.int_data));
      break;
    case 0x03:
      SetDouble(memory_ptr + arguments[0], abs(value_object->data.float_data));
      break;
    case 0x04:
      SetUint64(memory_ptr + arguments[0], value_object->data.uint64t_data);
      break;
    default:
      LOGGING_ERROR("Unsupported data type.");
      break;
  }

  return 0;
}

int __builtin_open(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() < 2 || arguments.size() > 3)
    LOGGING_ERROR("Invalid args.");

  auto memory_ptr = memory->GetMemory().data();

  std::string mode = "r";

  if (arguments.size() == 3) mode = GetString(memory_ptr + arguments[2]);

  FILE* file =
      fopen(GetString(memory_ptr + arguments[1]).c_str(), mode.c_str());

  if (file == nullptr) LOGGING_WARNING("Unexpected error.");

  SetPtr(memory_ptr + arguments[0], file);

  return 0;
}
int __builtin_close(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args.");

  auto memory_ptr = memory->GetMemory().data();

  SetLong(memory_ptr + arguments[0],
          fclose((FILE*)GetPtr(memory_ptr + arguments[1])));

  return 0;
}
int __builtin_read(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args.");

  auto memory_ptr = memory->GetMemory().data();

  FILE* file_ptr = (FILE*)GetPtr(memory_ptr + arguments[1]);
  if (file_ptr == nullptr) {
    SetString(memory_ptr + arguments[0], "");
    return 0;
  }

  fpos_t original_position;
  fgetpos(file_ptr, &original_position);

  fseek(file_ptr, 0, SEEK_END);
  long file_size = ftell(file_ptr);
  fseek(file_ptr, 0, SEEK_SET);

  char* temp_ptr = new char[file_size + 1];
  std::size_t actually_read =
      fread(temp_ptr, sizeof(char), file_size, file_ptr);
  temp_ptr[actually_read] = '\0';

  std::string result = temp_ptr;
  SetString(memory_ptr + arguments[0], result);

  fsetpos(file_ptr, &original_position);
  delete[] temp_ptr;

  return 0;
}
int __builtin_write(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 3) LOGGING_ERROR("Invalid args.");

  auto memory_ptr = memory->GetMemory().data();

  std::string str = GetString(memory_ptr + arguments[2]);

  std::size_t count = fwrite(str.c_str(), sizeof(char), str.size(),
                             (FILE*)GetPtr(memory_ptr + arguments[1]));

  SetUint64(memory_ptr + arguments[0], count);

  return 0;
}
int __builtin_input(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 1) LOGGING_ERROR("Invalid args.");

  auto memory_ptr = memory->GetMemory().data();

  std::string line;
  if (!std::getline(std::cin, line)) {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    line.clear();
    INTERNAL_ERROR("Unexpected input error.");
  }

  SetString(memory_ptr + arguments[0], line);

  return 0;
}

int __builtin_GUI_CreateWindow(Memory* memory,
                               std::vector<std::size_t> arguments) {
  return 0;
}

int __builtin_math_acos(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::acos(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_asin(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::asin(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_atan(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::atan(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_atan2(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 3) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double y = GetDouble(memory_ptr + arguments[1]);
  double x = GetDouble(memory_ptr + arguments[2]);
  SetDouble(memory_ptr + arguments[0], std::atan2(y, x));
  return 0;
}

int __builtin_math_ceil(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::ceil(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_cos(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::cos(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_cosh(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::cosh(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_exp(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::exp(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_fabs(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::fabs(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_floor(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::floor(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_fmod(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 3) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double numer = GetDouble(memory_ptr + arguments[1]);
  double denom = GetDouble(memory_ptr + arguments[2]);
  SetDouble(memory_ptr + arguments[0], std::fmod(numer, denom));
  return 0;
}

int __builtin_math_frexp(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double value = GetDouble(memory_ptr + arguments[1]);
  int exp;
  double fraction = std::frexp(value, &exp);

  Object fraction_object;
  fraction_object.type = 3;
  fraction_object.data.float_data = fraction;
  fraction_object.constant_type = true;

  Object exp_object;
  exp_object.type = 2;
  exp_object.data.int_data = exp;
  exp_object.constant_type = true;

  Memory* array_memory = new Memory();
  array_memory->GetMemory().push_back(fraction_object);
  array_memory->GetMemory().push_back(exp_object);

  SetArray(memory_ptr + arguments[0], array_memory);
  return 0;
}

int __builtin_math_hypot(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 3) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double x = GetDouble(memory_ptr + arguments[1]);
  double y = GetDouble(memory_ptr + arguments[2]);
  SetDouble(memory_ptr + arguments[0], std::hypot(x, y));
  return 0;
}

int __builtin_math_ldexp(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 3) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double x = GetDouble(memory_ptr + arguments[1]);
  int exp = static_cast<int>(GetLong(memory_ptr + arguments[2]));
  SetDouble(memory_ptr + arguments[0], std::ldexp(x, exp));
  return 0;
}

int __builtin_math_log(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::log(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_log10(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::log10(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_modf(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double value = GetDouble(memory_ptr + arguments[1]);
  double int_part;
  double frac = std::modf(value, &int_part);

  Object frac_object;
  frac_object.type = 3;
  frac_object.data.float_data = frac;
  frac_object.constant_type = true;

  Object int_part_object;
  int_part_object.type = 2;
  int_part_object.data.int_data = int_part;
  int_part_object.constant_type = true;

  Memory* array_memory = new Memory();
  array_memory->GetMemory().push_back(frac_object);
  array_memory->GetMemory().push_back(int_part_object);

  SetArray(memory_ptr + arguments[0], array_memory);
  return 0;
}

int __builtin_math_pow(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 3) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double base = GetDouble(memory_ptr + arguments[1]);
  double exponent = GetDouble(memory_ptr + arguments[2]);
  SetDouble(memory_ptr + arguments[0], std::pow(base, exponent));
  return 0;
}

int __builtin_math_sin(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::sin(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_sinh(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::sinh(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_sqrt(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::sqrt(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_tan(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::tan(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

int __builtin_math_tanh(Memory* memory, std::vector<std::size_t> arguments) {
  if (arguments.size() != 2) LOGGING_ERROR("Invalid args");
  auto memory_ptr = memory->GetMemory().data();
  double result = std::tanh(GetDouble(memory_ptr + arguments[1]));
  SetDouble(memory_ptr + arguments[0], result);
  return 0;
}

}  // namespace Interpreter
}  // namespace Aq
