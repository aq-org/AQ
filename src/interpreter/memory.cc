// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/memory.h"

#include "logging/logging.h"

namespace Aq {
namespace Interpreter {

bool is_run = false;

std::size_t Memory::Add(std::size_t size, bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  std::size_t index = memory_.size();
  for (size_t i = 0; i < size; i++) {
    memory_.push_back(
        {{0x00}, static_cast<int64_t>(0), false, is_constant_data});
  }

  return index;
}

std::size_t Memory::AddWithType(std::vector<uint8_t> type,
                                bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  memory_.push_back({type, static_cast<int64_t>(0), true, is_constant_data});

  return memory_.size() - 1;
}

std::size_t Memory::AddByte(int8_t value, bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  memory_.push_back({{0x01}, value, true, is_constant_data});
  return memory_.size() - 1;
}

std::size_t Memory::AddLong(int64_t value, bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  memory_.push_back({{0x02}, value, true, is_constant_data});
  return memory_.size() - 1;
}

std::size_t Memory::AddDouble(double value, bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  memory_.push_back({{0x03}, value, true, is_constant_data});
  return memory_.size() - 1;
}

std::size_t Memory::AddUint64t(uint64_t value, bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  memory_.push_back({{0x04}, value, true, is_constant_data});
  return memory_.size() - 1;
}

void Memory::SetUint64tValue(std::size_t index, uint64_t value,
                             bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  memory_.at(index) = {{0x04}, value, true, is_constant_data};
}

std::size_t Memory::AddString(std::string value, bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  memory_.push_back({{0x05}, value, true, is_constant_data});
  return memory_.size() - 1;
}

std::size_t Memory::AddReference(Memory& memory, std::size_t index,
                                 bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  ObjectReference reference = {std::ref(memory), index};
  memory_.push_back({{0x07, 0x00}, reference, true, is_constant_data});
  return memory_.size() - 1;
}

std::size_t Memory::AddReference(ClassMemory& memory, std::string index,
                                 bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  ObjectReference reference = {std::ref(memory), index};
  memory_.push_back({{0x07, 0x00}, reference, true, is_constant_data});
  return memory_.size() - 1;
}

void ClassMemory::Add(std::string name, bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  members_[name] = {{0x00}, static_cast<int64_t>(0), false, is_constant_data};
}

void ClassMemory::AddWithType(std::string name, std::vector<uint8_t> type,
                              bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  members_[name] = {type, static_cast<int64_t>(0), true, is_constant_data};
}

void ClassMemory::AddByte(std::string name, int8_t value,
                          bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  members_[name] = {{0x01}, value, true, is_constant_data};
}

void ClassMemory::AddLong(std::string name, int64_t value,
                          bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  members_[name] = {{0x02}, value, true, is_constant_data};
}

void ClassMemory::AddDouble(std::string name, double value,
                            bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  members_[name] = {{0x03}, value, true, is_constant_data};
}

void ClassMemory::AddUint64t(std::string name, uint64_t value,
                             bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  members_[name] = {{0x04}, value, true, is_constant_data};
}

void ClassMemory::AddString(std::string name, std::string value,
                            bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  members_[name] = {{0x05}, value, true, is_constant_data};
}

void ClassMemory::AddReference(std::string name, Memory& memory,
                               std::size_t index, bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  ObjectReference reference = {std::ref(memory), index};
  members_[name] = {{0x07, 0x00}, reference, true, is_constant_data};
}

void ClassMemory::AddReference(std::string name, ClassMemory& memory,
                               std::string index, bool is_constant_data) {
  if (is_run) INTERNAL_ERROR("Memory is running, cannot add new memory.");
  ObjectReference reference = {std::ref(memory), index};
  members_[name] = {{0x07, 0x00}, reference, true, is_constant_data};
}

}  // namespace Interpreter
}  // namespace Aq