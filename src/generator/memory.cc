// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "generator/memory.h"

#include "generator/operator.h"
#include "generator/uleb128.h"

namespace Aq {
namespace Generator {

std::size_t Memory::Add(std::size_t size) {
  std::size_t index = memory_size_;
  for (size_t i = 0; i < size; i++) {
    memory_type_.push_back(0x00);
    memory_size_++;
  }

  return index;
}

std::size_t Memory::AddWithType(std::vector<uint8_t> type) {
  std::size_t index = memory_size_;
  for (std::size_t i = 0; i < type.size(); i++) {
    memory_type_.push_back(type[i]);
  }
  memory_size_++;

  return index;
}

std::size_t Memory::AddByte(int8_t value) {
  constant_table_.push_back(0x01);
  constant_table_.push_back(value);
  memory_type_.push_back(0x01);

  std::size_t constant_index = constant_table_size_++;
  std::size_t index = memory_size_++;

  init_code_->push_back(
      Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, index, constant_index));
  return index;
}

std::size_t Memory::AddLong(int64_t value) {
  constant_table_.push_back(0x02);
  memory_type_.push_back(0x02);

  value = is_big_endian_ ? value : SwapLong(value);
  for (int i = 0; i < 8; ++i) {
    constant_table_.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
  }

  std::size_t constant_index = constant_table_size_++;
  std::size_t index = memory_size_++;

  init_code_->push_back(
      Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, index, constant_index));
  return index;
}

std::size_t Memory::AddDouble(double value) {
  constant_table_.push_back(0x03);
  memory_type_.push_back(0x03);

  value = is_big_endian_ ? value : SwapDouble(value);
  uint64_t int_value;
  std::memcpy(&int_value, &value, sizeof(double));
  for (int i = 0; i < 8; ++i) {
    constant_table_.push_back(
        static_cast<uint8_t>((int_value >> (i * 8)) & 0xFF));
  }

  std::size_t constant_index = constant_table_size_++;
  std::size_t index = memory_size_++;

  init_code_->push_back(
      Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, index, constant_index));
  return index;
}

std::size_t Memory::AddUint64t(uint64_t value) {
  constant_table_.push_back(0x04);
  memory_type_.push_back(0x04);

  value = is_big_endian_ ? value : SwapUint64t(value);
  for (int i = 0; i < 8; ++i) {
    constant_table_.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
  }

  std::size_t constant_index = constant_table_size_++;
  std::size_t index = memory_size_++;

  init_code_->push_back(
      Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, index, constant_index));
  return index;
}

std::size_t Memory::AddUint64tWithoutValue(std::size_t& code) {
  memory_type_.push_back(0x04);
  std::size_t index = memory_size_++;

  code = init_code_->size();

  init_code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, index, 0));
  return index;
}

void Memory::SetUint64tValue(std::size_t code, uint64_t value) {
  constant_table_.push_back(0x04);
  std::size_t constant_index = constant_table_size_++;

  value = is_big_endian_ ? value : SwapUint64t(value);
  for (int i = 0; i < 8; ++i) {
    constant_table_.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
  }

  auto args = (*init_code_)[code].GetArgs();
  args[1] = constant_index;
  (*init_code_)[code].SetArgs(args);
}

std::size_t Memory::AddString(std::string value) {
  constant_table_.push_back(0x05);
  memory_type_.push_back(0x05);

  EncodeUleb128(value.size() + 1, constant_table_);
  for (std::size_t i = 0; i < value.size(); i++) {
    constant_table_.push_back(value[i]);
  }
  // Add a null terminator for the string.
  constant_table_.push_back(0x00);

  std::size_t constant_index = constant_table_size_++;
  std::size_t index = memory_size_++;

  init_code_->push_back(
      Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, index, constant_index));
  return index;
}

std::size_t ClassMemory::Add(std::string name) {
  std::size_t index = memory_size_++;
  memory_type_.push_back(0x00);
  variable_name_.push_back(name);

  memory_info_.insert(
      memory_info_.end(), reinterpret_cast<const uint8_t*>(name.c_str()),
      reinterpret_cast<const uint8_t*>(name.c_str() + name.size() + 1));
  memory_info_.push_back(0x00);
  return index;
}

std::size_t ClassMemory::AddWithType(std::string name,
                                     std::vector<uint8_t> type) {
  std::size_t index = memory_size_++;
  for (std::size_t i = 0; i < type.size(); i++) {
    memory_type_.push_back(type[i]);
  }
  variable_name_.push_back(name);

  memory_info_.insert(
      memory_info_.end(), reinterpret_cast<const uint8_t*>(name.c_str()),
      reinterpret_cast<const uint8_t*>(name.c_str() + name.size() + 1));
  memory_info_.insert(memory_info_.end(), type.begin(), type.end());

  return index;
}

std::size_t ClassMemory::AddByte(std::string name, int8_t value) {
  memory_type_.push_back(0x01);
  memory_size_++;

  memory_info_.insert(
      memory_info_.end(), reinterpret_cast<const uint8_t*>(name.c_str()),
      reinterpret_cast<const uint8_t*>(name.c_str() + name.size() + 1));
  memory_info_.push_back(0x01);

  std::size_t index = global_memory_->Add(1);
  variable_name_.push_back(name);

  init_code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 0,
                                 global_memory_->AddString(name)));
  init_code_->push_back(
      Bytecode(_AQVM_OPERATOR_EQUAL, 2, index, global_memory_->AddByte(value)));
  return memory_size_ - 1;
}

std::size_t ClassMemory::AddLong(std::string name, int64_t value) {
  memory_type_.push_back(0x02);
  memory_size_++;

  memory_info_.insert(
      memory_info_.end(), reinterpret_cast<const uint8_t*>(name.c_str()),
      reinterpret_cast<const uint8_t*>(name.c_str() + name.size() + 1));
  memory_info_.push_back(0x02);

  std::size_t index = global_memory_->Add(1);
  variable_name_.push_back(name);

  init_code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 0,
                                 global_memory_->AddString(name)));
  init_code_->push_back(
      Bytecode(_AQVM_OPERATOR_EQUAL, 2, index, global_memory_->AddLong(value)));
  return memory_size_ - 1;
}

std::size_t ClassMemory::AddDouble(std::string name, double value) {
  memory_type_.push_back(0x03);
  memory_size_++;

  memory_info_.insert(
      memory_info_.end(), reinterpret_cast<const uint8_t*>(name.c_str()),
      reinterpret_cast<const uint8_t*>(name.c_str() + name.size() + 1));
  memory_info_.push_back(0x03);

  std::size_t index = global_memory_->Add(1);
  variable_name_.push_back(name);

  init_code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 0,
                                 global_memory_->AddString(name)));
  init_code_->push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, index,
                                 global_memory_->AddDouble(value)));
  return memory_size_ - 1;
}

std::size_t ClassMemory::AddUint64t(std::string name, uint64_t value) {
  memory_type_.push_back(0x04);
  memory_size_++;

  memory_info_.insert(
      memory_info_.end(), reinterpret_cast<const uint8_t*>(name.c_str()),
      reinterpret_cast<const uint8_t*>(name.c_str() + name.size() + 1));
  memory_info_.push_back(0x04);

  std::size_t index = global_memory_->Add(1);
  variable_name_.push_back(name);

  init_code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 0,
                                 global_memory_->AddString(name)));
  init_code_->push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, index,
                                 global_memory_->AddUint64t(value)));
  return memory_size_ - 1;
}

std::size_t ClassMemory::AddString(std::string name, std::string value) {
  memory_type_.push_back(0x05);
  memory_size_++;

  memory_info_.insert(
      memory_info_.end(), reinterpret_cast<const uint8_t*>(name.c_str()),
      reinterpret_cast<const uint8_t*>(name.c_str() + name.size() + 1));
  memory_info_.push_back(0x05);

  std::size_t index = global_memory_->Add(1);
  variable_name_.push_back(name);

  init_code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 0,
                                 global_memory_->AddString(name)));
  init_code_->push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, index,
                                 global_memory_->AddString(value)));
  return memory_size_ - 1;
}

}  // namespace Generator
}  // namespace Aq