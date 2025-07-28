// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/memory.h"

namespace Aq {
namespace Interpreter {

std::size_t Memory::Add(std::size_t size) {
  std::size_t index = memory_->size();
  for (size_t i = 0; i < size; i++) {
    memory_->push_back({{0x00}, static_cast<int64_t>(0), false});
  }

  return index;
}

std::size_t Memory::AddWithType(std::vector<uint8_t> type) {
  memory_->push_back({type, static_cast<int64_t>(0), false});

  return memory_->size() - 1;
}

std::size_t Memory::AddByte(int8_t value) {
  memory_->push_back({{0x01}, value, false});
  return memory_->size() - 1;
}

std::size_t Memory::AddLong(int64_t value) {
  memory_->push_back({{0x02}, value, false});
  return memory_->size() - 1;
}

std::size_t Memory::AddDouble(double value) {
  memory_->push_back({{0x03}, value, false});
  return memory_->size() - 1;
}

std::size_t Memory::AddUint64t(uint64_t value) {
  memory_->push_back({{0x04}, value, false});
  return memory_->size() - 1;
}

void Memory::SetUint64tValue(std::size_t index, uint64_t value) {
  memory_->at(index) = {{0x04}, value, false};
}

std::size_t Memory::AddString(std::string value) {
  memory_->push_back({{0x05}, value, false});
  return memory_->size() - 1;
}

void ClassMemory::Add(std::string name) {
  members_[name] = {{0x00}, static_cast<int64_t>(0), false};
}

void ClassMemory::AddWithType(std::string name, std::vector<uint8_t> type) {
  members_[name] = {type, static_cast<int64_t>(0), false};
}

void ClassMemory::AddByte(std::string name, int8_t value) {
  members_[name] = {{0x01}, value, false};
}

void ClassMemory::AddLong(std::string name, int64_t value) {
  members_[name] = {{0x02}, value, false};
}

void ClassMemory::AddDouble(std::string name, double value) {
  members_[name] = {{0x03}, value, false};
}

void ClassMemory::AddUint64t(std::string name, uint64_t value) {
  members_[name] = {{0x04}, value, false};
}

void ClassMemory::AddString(std::string name, std::string value) {
  members_[name] = {{0x05}, value, false};
}

}  // namespace Interpreter
}  // namespace Aq