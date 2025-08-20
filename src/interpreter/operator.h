// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_OPERATOR_H_
#define AQ_INTERPRETER_OPERATOR_H_

#include <functional>

#include "interpreter/function.h"
#include "src/interpreter/class.h"
#include "src/interpreter/memory.h"

#define _AQVM_OPERATOR_NOP 0x00
#define _AQVM_OPERATOR_LOAD 0x01
#define _AQVM_OPERATOR_STORE 0x02
#define _AQVM_OPERATOR_NEW 0x03
#define _AQVM_OPERATOR_ARRAY 0x04
#define _AQVM_OPERATOR_PTR 0x05
#define _AQVM_OPERATOR_ADD 0x06
#define _AQVM_OPERATOR_SUB 0x07
#define _AQVM_OPERATOR_MUL 0x08
#define _AQVM_OPERATOR_DIV 0x09
#define _AQVM_OPERATOR_REM 0x0A
#define _AQVM_OPERATOR_NEG 0x0B
#define _AQVM_OPERATOR_SHL 0x0C
#define _AQVM_OPERATOR_SHR 0x0D
#define _AQVM_OPERATOR_REFER 0x0E
#define _AQVM_OPERATOR_IF 0x0F
#define _AQVM_OPERATOR_AND 0x10
#define _AQVM_OPERATOR_OR 0x11
#define _AQVM_OPERATOR_XOR 0x12
#define _AQVM_OPERATOR_CMP 0x13
#define _AQVM_OPERATOR_INVOKE 0x14
#define _AQVM_OPERATOR_EQUAL 0x15
#define _AQVM_OPERATOR_GOTO 0x16
#define _AQVM_OPERATOR_LOAD_CONST 0x17
#define _AQVM_OPERATOR_CONVERT 0x18
#define _AQVM_OPERATOR_CONST 0x19
#define _AQVM_OPERATOR_INVOKE_METHOD 0x1A
#define _AQVM_OPERATOR_LOAD_MEMBER 0x1B
#define _AQVM_OPERATOR_WIDE 0xFF

namespace Aq {
namespace Interpreter {
int NOP();

int NEW(Object* memory, std::unordered_map<std::string, Class> classes,
        std::size_t ptr, std::size_t size, std::size_t type,
        std::unordered_map<
            std::string, std::function<int(Memory*, std::vector<std::size_t>)>>&
            builtin_functions);

int ARRAY(Object* memory, std::size_t result, std::size_t ptr,
          std::size_t index, std::unordered_map<std::string, Class>& classes);

/*int ADD(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int SUB(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int MUL(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int DIV(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int REM(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);*/
int NEG(Object* memory, std::size_t result, std::size_t operand1);

/*int SHL(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int SHR(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);*/

int REFER(Memory* memory, std::size_t result, std::size_t operand1);

size_t IF(Object* memory, std::size_t condition, std::size_t true_branche,
          std::size_t false_branche);

/*int AND(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int OR(Object* memory, std::size_t result, std::size_t operand1,
       std::size_t operand2);

int XOR(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);*/

int CMP(Object* memory, std::size_t result, std::size_t opcode,
        std::size_t operand1, std::size_t operand2);

int EQUAL(Object* memory, std::size_t result, std::size_t value);

size_t GOTO(Object* memory, std::size_t location);

int INVOKE_METHOD(
    Memory* memory, std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory*, std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<std::size_t> arguments);

int InvokeClassMethod(
    Memory* memory, std::size_t class_object, std::size_t method_name_object,
    std::vector<std::size_t> arguments,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory*, std::vector<std::size_t>)>>&
        builtin_functions);
Function SelectBestFunction(Object* memory, std::vector<Function>& functions,
                            std::vector<std::size_t>& arguments);
int64_t GetFunctionOverloadValue(Object* memory, Function& function,
                                 std::vector<std::size_t>& arguments);

int LOAD_MEMBER(Memory* memory, std::unordered_map<std::string, Class>& classes,
                std::size_t result, std::size_t class_index,
                std::size_t operand);

#define ADD(result, operand1, operand2)                                      \
  {                                                                          \
    GET_ORIGIN(result);                                                      \
    GET_ORIGIN(operand1);                                                    \
    GET_ORIGIN(operand2);                                                    \
    if (operand1->type == 0x02 && operand2->type == 0x02) {                  \
      SET_INT(result, operand1->data.int_data + operand2->data.int_data)     \
    } else if (operand1->type <= 0x03 && operand2->type <= 0x03) {           \
      SET_FLOAT(result,                                                      \
                (operand1->type == 0x03 ? operand1->data.float_data          \
                                        : operand1->data.int_data) +         \
                    (operand2->type == 0x03 ? operand2->data.float_data      \
                                            : operand2->data.int_data))      \
    } else if (operand1->type <= 0x04 && operand2->type <= 0x04) {           \
      SET_UINT64T(                                                           \
          result,                                                            \
          (operand1->type == 0x04                                            \
               ? operand1->data.uint64t_data                                 \
               : (operand1->type == 0x02 ? operand1->data.int_data           \
                                         : operand1->data.float_data)) +     \
              (operand2->type == 0x04                                        \
                   ? operand2->data.uint64t_data                             \
                   : (operand2->type == 0x02 ? operand2->data.int_data       \
                                             : operand2->data.float_data)))  \
    } else if (operand1->type == 0x05 && operand2->type == 0x05) {           \
      SET_STRING(result,                                                     \
                 *operand1->data.string_data + *operand2->data.string_data); \
    } else if (operand1->type == 0x06 && operand2->type == 0x06) {           \
      const auto& array1 = GetArray(operand1)->GetMemory();                  \
      const auto& array2 = GetArray(operand2)->GetMemory();                  \
      std::vector<Object> new_array;                                         \
      new_array.reserve(array1.size() + array2.size());                      \
      new_array.insert(new_array.end(), array1.begin(), array1.end());       \
      new_array.insert(new_array.end(), array2.begin(), array2.end());       \
      SET_ARRAY_CONTENT(result, new_array);                                  \
      break;                                                                 \
    } else {                                                                 \
      LOGGING_ERROR("Unsupported data type");                                \
    }                                                                        \
  }

#define SUB(result, operand1, operand2)                                     \
  {                                                                         \
    GET_ORIGIN(result);                                                     \
    GET_ORIGIN(operand1);                                                   \
    GET_ORIGIN(operand2);                                                   \
    if (operand1->type == 0x02 && operand2->type == 0x02) {                 \
      SET_INT(result, operand1->data.int_data - operand2->data.int_data)    \
    } else if (operand1->type <= 0x03 && operand2->type <= 0x03) {          \
      SET_FLOAT(result,                                                     \
                (operand1->type == 0x03 ? operand1->data.float_data         \
                                        : operand1->data.int_data) -        \
                    (operand2->type == 0x03 ? operand2->data.float_data     \
                                            : operand2->data.int_data))     \
    } else if (operand1->type <= 0x04 && operand2->type <= 0x04) {          \
      SET_UINT64T(                                                          \
          result,                                                           \
          (operand1->type == 0x04                                           \
               ? operand1->data.uint64t_data                                \
               : (operand1->type == 0x02 ? operand1->data.int_data          \
                                         : operand1->data.float_data)) -    \
              (operand2->type == 0x04                                       \
                   ? operand2->data.uint64t_data                            \
                   : (operand2->type == 0x02 ? operand2->data.int_data      \
                                             : operand2->data.float_data))) \
    } else {                                                                \
      LOGGING_ERROR("Unsupported data type");                               \
    }                                                                       \
  }

#define MUL(result, operand1, operand2)                                     \
  {                                                                         \
    GET_ORIGIN(result);                                                     \
    GET_ORIGIN(operand1);                                                   \
    GET_ORIGIN(operand2);                                                   \
    if (operand1->type == 0x02 && operand2->type == 0x02) {                 \
      SET_INT(result, operand1->data.int_data * operand2->data.int_data)    \
    } else if (operand1->type <= 0x03 && operand2->type <= 0x03) {          \
      SET_FLOAT(result,                                                     \
                (operand1->type == 0x03 ? operand1->data.float_data         \
                                        : operand1->data.int_data) *        \
                    (operand2->type == 0x03 ? operand2->data.float_data     \
                                            : operand2->data.int_data))     \
    } else if (operand1->type <= 0x04 && operand2->type <= 0x04) {          \
      SET_UINT64T(                                                          \
          result,                                                           \
          (operand1->type == 0x04                                           \
               ? operand1->data.uint64t_data                                \
               : (operand1->type == 0x02 ? operand1->data.int_data          \
                                         : operand1->data.float_data)) *    \
              (operand2->type == 0x04                                       \
                   ? operand2->data.uint64t_data                            \
                   : (operand2->type == 0x02 ? operand2->data.int_data      \
                                             : operand2->data.float_data))) \
    } else {                                                                \
      LOGGING_ERROR("Unsupported data type");                               \
    }                                                                       \
  }

#define DIV(result, operand1, operand2)                                     \
  {                                                                         \
    GET_ORIGIN(result);                                                     \
    GET_ORIGIN(operand1);                                                   \
    GET_ORIGIN(operand2);                                                   \
    if (operand1->type == 0x02 && operand2->type == 0x02) {                 \
      SET_INT(result, operand1->data.int_data / operand2->data.int_data)    \
    } else if (operand1->type <= 0x03 && operand2->type <= 0x03) {          \
      SET_FLOAT(result,                                                     \
                (operand1->type == 0x03 ? operand1->data.float_data         \
                                        : operand1->data.int_data) /        \
                    (operand2->type == 0x03 ? operand2->data.float_data     \
                                            : operand2->data.int_data))     \
    } else if (operand1->type <= 0x04 && operand2->type <= 0x04) {          \
      SET_UINT64T(                                                          \
          result,                                                           \
          (operand1->type == 0x04                                           \
               ? operand1->data.uint64t_data                                \
               : (operand1->type == 0x02 ? operand1->data.int_data          \
                                         : operand1->data.float_data)) /    \
              (operand2->type == 0x04                                       \
                   ? operand2->data.uint64t_data                            \
                   : (operand2->type == 0x02 ? operand2->data.int_data      \
                                             : operand2->data.float_data))) \
    } else {                                                                \
      LOGGING_ERROR("Unsupported data type");                               \
    }                                                                       \
  }

#define REM(result, operand1, operand2)                                   \
  {                                                                       \
    GET_ORIGIN(result);                                                   \
    GET_ORIGIN(operand1);                                                 \
    GET_ORIGIN(operand2);                                                 \
    if (operand1->type == 0x02 && operand2->type == 0x02) {               \
      SET_INT(result, operand1->data.int_data % operand2->data.int_data)  \
    } else if (operand1->type <= 0x03 && operand2->type <= 0x03) {        \
      LOGGING_ERROR("Cannot calculate remainder for doubles.");           \
    } else if (operand1->type <= 0x04 && operand2->type <= 0x04) {        \
      SET_UINT64T(result,                                                 \
                  (operand1->type == 0x04                                 \
                       ? operand1->data.uint64t_data                      \
                       : (operand1->type == 0x02                          \
                              ? operand1->data.int_data                   \
                              : (int64_t)operand1->data.float_data)) %    \
                      (operand2->type == 0x04                             \
                           ? operand2->data.uint64t_data                  \
                           : (operand2->type == 0x02                      \
                                  ? operand2->data.int_data               \
                                  : (int64_t)operand2->data.float_data))) \
    } else {                                                              \
      LOGGING_ERROR("Unsupported data type");                             \
    }                                                                     \
  }

#define SHL(result, operand1, operand2)                                        \
  {                                                                            \
    GET_ORIGIN(result);                                                        \
    GET_ORIGIN(operand1);                                                      \
    GET_ORIGIN(operand2);                                                      \
    if (operand1->type == 0x02 && operand2->type == 0x02) {                    \
      SET_INT(result, operand1->data.int_data << operand2->data.int_data)      \
    } else if (operand1->type <= 0x03 && operand2->type <= 0x03) {             \
      LOGGING_ERROR("Cannot shift doubles. Use multiplication for shifting."); \
    } else if (operand1->type <= 0x04 && operand2->type <= 0x04) {             \
      SET_UINT64T(result,                                                      \
                  (operand1->type == 0x04                                      \
                       ? operand1->data.uint64t_data                           \
                       : (operand1->type == 0x02                               \
                              ? operand1->data.int_data                        \
                              : (int64_t)operand1->data.float_data))           \
                      << (operand2->type == 0x04                               \
                              ? operand2->data.uint64t_data                    \
                              : (operand2->type == 0x02                        \
                                     ? operand2->data.int_data                 \
                                     : (int64_t)operand2->data.float_data)))   \
    } else {                                                                   \
      LOGGING_ERROR("Unsupported data type");                                  \
    }                                                                          \
  }

#define SHR(result, operand1, operand2)                                        \
  {                                                                            \
    GET_ORIGIN(result);                                                        \
    GET_ORIGIN(operand1);                                                      \
    GET_ORIGIN(operand2);                                                      \
    if (operand1->type == 0x02 && operand2->type == 0x02) {                    \
      SET_INT(result, operand1->data.int_data >> operand2->data.int_data)      \
    } else if (operand1->type <= 0x03 && operand2->type <= 0x03) {             \
      LOGGING_ERROR("Cannot shift doubles. Use multiplication for shifting."); \
    } else if (operand1->type <= 0x04 && operand2->type <= 0x04) {             \
      SET_UINT64T(result,                                                      \
                  (operand1->type == 0x04                                      \
                       ? operand1->data.uint64t_data                           \
                       : (operand1->type == 0x02                               \
                              ? operand1->data.int_data                        \
                              : (int64_t)operand1->data.float_data)) >>        \
                      (operand2->type == 0x04                                  \
                           ? operand2->data.uint64t_data                       \
                           : (operand2->type == 0x02                           \
                                  ? operand2->data.int_data                    \
                                  : (int64_t)operand2->data.float_data)))      \
    } else {                                                                   \
      LOGGING_ERROR("Unsupported data type");                                  \
    }                                                                          \
  }

#define AND(result, operand1, operand2)                                   \
  {                                                                       \
    GET_ORIGIN(result);                                                   \
    GET_ORIGIN(operand1);                                                 \
    GET_ORIGIN(operand2);                                                 \
    if (operand1->type == 0x02 && operand2->type == 0x02) {               \
      SET_INT(result, operand1->data.int_data & operand2->data.int_data)  \
    } else if (operand1->type <= 0x03 && operand2->type <= 0x03) {        \
      LOGGING_ERROR("Cannot perform AND operation on float.");            \
    } else if (operand1->type <= 0x04 && operand2->type <= 0x04) {        \
      SET_UINT64T(result,                                                 \
                  (operand1->type == 0x04                                 \
                       ? operand1->data.uint64t_data                      \
                       : (operand1->type == 0x02                          \
                              ? operand1->data.int_data                   \
                              : (int64_t)operand1->data.float_data)) &    \
                      (operand2->type == 0x04                             \
                           ? operand2->data.uint64t_data                  \
                           : (operand2->type == 0x02                      \
                                  ? operand2->data.int_data               \
                                  : (int64_t)operand2->data.float_data))) \
    } else {                                                              \
      LOGGING_ERROR("Unsupported data type");                             \
    }                                                                     \
  }

#define OR(result, operand1, operand2)                                    \
  {                                                                       \
    GET_ORIGIN(result);                                                   \
    GET_ORIGIN(operand1);                                                 \
    GET_ORIGIN(operand2);                                                 \
    if (operand1->type == 0x02 && operand2->type == 0x02) {               \
      SET_INT(result, operand1->data.int_data | operand2->data.int_data)  \
    } else if (operand1->type <= 0x03 && operand2->type <= 0x03) {        \
      LOGGING_ERROR("Cannot perform OR operation on float.");             \
    } else if (operand1->type <= 0x04 && operand2->type <= 0x04) {        \
      SET_UINT64T(result,                                                 \
                  (operand1->type == 0x04                                 \
                       ? operand1->data.uint64t_data                      \
                       : (operand1->type == 0x02                          \
                              ? operand1->data.int_data                   \
                              : (int64_t)operand1->data.float_data)) |    \
                      (operand2->type == 0x04                             \
                           ? operand2->data.uint64t_data                  \
                           : (operand2->type == 0x02                      \
                                  ? operand2->data.int_data               \
                                  : (int64_t)operand2->data.float_data))) \
    } else {                                                              \
      LOGGING_ERROR("Unsupported data type");                             \
    }                                                                     \
  }

#define XOR(result, operand1, operand2)                                   \
  {                                                                       \
    GET_ORIGIN(result);                                                   \
    GET_ORIGIN(operand1);                                                 \
    GET_ORIGIN(operand2);                                                 \
    if (operand1->type == 0x02 && operand2->type == 0x02) {               \
      SET_INT(result, operand1->data.int_data ^ operand2->data.int_data)  \
    } else if (operand1->type <= 0x03 && operand2->type <= 0x03) {        \
      LOGGING_ERROR("Cannot perform XOR operation on float.");            \
    } else if (operand1->type <= 0x04 && operand2->type <= 0x04) {        \
      SET_UINT64T(result,                                                 \
                  (operand1->type == 0x04                                 \
                       ? operand1->data.uint64t_data                      \
                       : (operand1->type == 0x02                          \
                              ? operand1->data.int_data                   \
                              : (int64_t)operand1->data.float_data)) ^    \
                      (operand2->type == 0x04                             \
                           ? operand2->data.uint64t_data                  \
                           : (operand2->type == 0x02                      \
                                  ? operand2->data.int_data               \
                                  : (int64_t)operand2->data.float_data))) \
    } else {                                                              \
      LOGGING_ERROR("Unsupported data type");                             \
    }                                                                     \
  }

}  // namespace Interpreter
}  // namespace Aq

#endif