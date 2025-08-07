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

int LOAD(std::shared_ptr<Memory> memory, std::size_t ptr, std::size_t operand);

int STORE(std::shared_ptr<Memory> memory, std::size_t ptr, std::size_t operand);

int NEW(
    std::shared_ptr<Memory> memory,
    std::unordered_map<std::string, Class> classes, std::size_t ptr,
    std::size_t size, std::size_t type,
    std::unordered_map<
        std::string,
        std::function<int(std::shared_ptr<Memory>, std::vector<std::size_t>)>>&
        builtin_functions);

int CrossMemoryNew(std::shared_ptr<Memory> memory,
                   std::unordered_map<std::string, Class> classes,
                   std::size_t ptr, std::size_t size, std::size_t type);

int ARRAY(std::shared_ptr<Memory> memory, std::size_t result, std::size_t ptr,
          std::size_t index, std::unordered_map<std::string, Class>& classes);

int PTR(std::shared_ptr<Memory> memory, std::size_t index, std::size_t ptr);

int ADD(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int SUB(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int MUL(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int DIV(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int REM(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2);
int NEG(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1);

int SHL(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int SHR(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int REFER(std::shared_ptr<Memory> memory, std::size_t result,
          std::size_t operand1);

size_t IF(std::shared_ptr<Memory> memory, std::size_t condition,
          std::size_t true_branche, std::size_t false_branche);

int AND(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int OR(std::shared_ptr<Memory> memory, std::size_t result, std::size_t operand1,
       std::size_t operand2);

int XOR(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int CMP(std::shared_ptr<Memory> memory, std::size_t result, std::size_t opcode,
        std::size_t operand1, std::size_t operand2);

int INVOKE(
    std::shared_ptr<Memory> memory,
    std::unordered_map<
        std::string,
        std::function<int(std::shared_ptr<Memory>, std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<std::size_t> arguments,
    std::unordered_map<std::string, Class>& classes);
int EQUAL(std::shared_ptr<Memory> memory, std::size_t result,
          std::size_t value);

int CrossMemoryEqual(std::shared_ptr<Memory> result_heap, std::size_t result,
                     std::shared_ptr<Memory> value_heap, std::size_t value);

size_t GOTO(std::shared_ptr<Memory> memory, std::size_t location);
int LOAD_CONST(std::shared_ptr<Memory> memory,
               std::shared_ptr<Memory> constant_pool, std::size_t object,
               std::size_t const_object);

int CONVERT(std::shared_ptr<Memory> memory, std::size_t result,
            std::size_t operand1);
int CONST(std::shared_ptr<Memory> memory, std::size_t result,
          std::size_t operand1);

int INVOKE_METHOD(
    std::shared_ptr<Memory> memory,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<
        std::string,
        std::function<int(std::shared_ptr<Memory>, std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<std::size_t> arguments);

int InvokeClassMethod(
    std::shared_ptr<Memory> memory, Object& class_object,
    Object& method_name_object, std::vector<std::size_t> arguments,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<
        std::string,
        std::function<int(std::shared_ptr<Memory>, std::vector<std::size_t>)>>&
        builtin_functions);
Function SelectBestFunction(std::shared_ptr<Memory> memory,
                            std::vector<Function>& functions,
                            std::vector<std::size_t>& arguments);
int64_t GetFunctionOverloadValue(std::shared_ptr<Memory> memory,
                                 Function& function,
                                 std::vector<std::size_t>& arguments);

int LOAD_MEMBER(std::shared_ptr<Memory> memory,
                std::unordered_map<std::string, Class>& classes,
                std::size_t result, std::size_t class_index,
                std::size_t operand);

int WIDE();

int8_t GetByte(Object& object);

void SetByte(Object& object, int8_t data);

int64_t GetLong(Object& object);

void SetLong(Object& object, int64_t data);

double GetDouble(Object& object);

void SetDouble(Object& object, double data);

uint64_t GetUint64(Object& object);

void SetUint64(Object& object, uint64_t data);

std::string GetString(Object& object);

void SetString(Object& object, const std::string& data);

std::shared_ptr<Memory> GetArray(Object& object);

void SetArray(Object& object, std::vector<Object>& memory);

std::shared_ptr<ClassMemory> GetObject(Object& object);

void SetObject(Object& object, std::shared_ptr<ClassMemory> data);

void CreateCacheGuard(Object& object);
}  // namespace Interpreter
}  // namespace Aq

#endif