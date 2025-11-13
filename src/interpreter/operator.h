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
#define _AQVM_OPERATOR_ADDI 0x1C
#define _AQVM_OPERATOR_SUBI 0x1D
#define _AQVM_OPERATOR_MULI 0x1E
#define _AQVM_OPERATOR_DIVI 0x1F
#define _AQVM_OPERATOR_REMI 0x20
#define _AQVM_OPERATOR_ADDF 0x21
#define _AQVM_OPERATOR_SUBF 0x22
#define _AQVM_OPERATOR_MULF 0x23
#define _AQVM_OPERATOR_DIVF 0x24
#define _AQVM_OPERATOR_LOAD_MODULE_MEMBER 0x25
#define _AQVM_OPERATOR_INVOKE_MODULE_METHOD 0x26
#define _AQVM_OPERATOR_NEW_MODULE 0x27
#define _AQVM_OPERATOR_WIDE 0xFF

namespace Aq {
namespace Interpreter {
int NOP();

int NEW(Object* memory, std::unordered_map<std::string, Class>& classes,
        std::size_t ptr, std::size_t size, std::size_t type,
        std::unordered_map<
            std::string, std::function<int(Memory*, std::vector<std::size_t>)>>&
            builtin_functions);

int ARRAY(Object* memory, std::size_t result, std::size_t ptr,
          std::size_t index, std::unordered_map<std::string, Class>& classes);

int ADD(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int SUB(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int MUL(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int DIV(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int REM(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);
int NEG(Object* memory, std::size_t result, std::size_t operand1);

int SHL(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int SHR(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int REFER(Memory* memory, std::size_t result, std::size_t operand1);

size_t IF(Object* memory, std::size_t condition, std::size_t true_branche,
          std::size_t false_branche);

int AND(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

int OR(Object* memory, std::size_t result, std::size_t operand1,
       std::size_t operand2);

int XOR(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2);

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

int LOAD_MODULE_MEMBER(Memory* local_memory, Memory* module_memory,
                       std::size_t result, std::size_t module_var_index);

int INVOKE_MODULE_METHOD(
    Memory* local_memory, Memory* module_memory,
    std::unordered_map<std::string, Class>& module_classes,
    std::unordered_map<std::string,
                       std::function<int(Memory*, std::vector<std::size_t>)>>&
        module_builtin_functions,
    std::vector<std::size_t> arguments);

int NEW_MODULE(Memory* local_memory, Memory* module_memory,
               std::unordered_map<std::string, Class>& module_classes,
               std::size_t result, std::size_t size, std::size_t type,
               std::unordered_map<
                   std::string,
                   std::function<int(Memory*, std::vector<std::size_t>)>>&
                   module_builtin_functions);

}  // namespace Interpreter
}  // namespace Aq

#endif