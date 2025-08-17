// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/memory.h"

#include <functional>
#include <string>

#include "logging/logging.h"

namespace Aq {
namespace Interpreter {
std::size_t Memory::Add(std::size_t size) {
  std::size_t index = memory_.size();
  for (size_t i = 0; i < size; i++) {
    memory_.push_back({0x00, 0, false});
  }

  return index;
}

std::size_t Memory::AddWithType(uint8_t type) {
  memory_.push_back({type, 0, type != 0x00});

  return memory_.size() - 1;
}

std::size_t Memory::AddByte(int8_t value) {
  memory_.push_back({0x01, value, true});
  return memory_.size() - 1;
}

std::size_t Memory::AddLong(int64_t value) {
  Object object;
  object.type = 0x02;
  object.data.int_data = value;
  object.constant_type = true;

  memory_.push_back(object);
  return memory_.size() - 1;
}

std::size_t Memory::AddDouble(double value) {
  Object object;
  object.type = 0x03;
  object.data.float_data = value;
  object.constant_type = true;

  memory_.push_back(object);
  return memory_.size() - 1;
}

std::size_t Memory::AddUint64t(uint64_t value) {
  Object object;
  object.type = 0x04;
  object.data.uint64t_data = value;
  object.constant_type = true;

  memory_.push_back(object);
  return memory_.size() - 1;
}

void Memory::SetUint64tValue(std::size_t index, uint64_t value) {
  Object object;
  object.type = 0x04;
  object.data.uint64t_data = value;
  object.constant_type = true;

  memory_.at(index) = object;
}

std::size_t Memory::AddString(std::string value) {
  Object object;
  object.type = 0x05;
  object.data.string_data = new std::string(value);
  object.constant_type = true;

  memory_.push_back(object);
  return memory_.size() - 1;
}

std::size_t Memory::AddReference(Memory* memory, std::size_t index) {
  ObjectReference* reference = new ObjectReference{false, memory, index};

  Object object;
  object.type = 0x07;
  object.data.reference_data = reference;
  object.constant_type = true;

  memory_.push_back(object);
  return memory_.size() - 1;
}

std::size_t Memory::AddReference(ClassMemory* memory, std::string index) {
  ObjectReference* reference = new ObjectReference();
  reference->is_class = true;
  reference->memory.class_memory = memory;
  reference->index.variable_name = new std::string(index);

  Object object;
  object.type = 0x07;
  object.data.reference_data = reference;
  object.constant_type = true;

  memory_.push_back(object);
  return memory_.size() - 1;
}

void Memory::InitObjectData(std::size_t index, ClassMemory* object) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");

  auto& origin_data = GetOriginData(index);

  origin_data.type = 0x09;
  origin_data.data.class_data = object;
}

void Memory::SetObjectData(std::size_t index, ClassMemory* object) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");

  auto& origin_data = GetOriginData(index);

  origin_data.type = 0x09;
  origin_data.data.class_data = object;
}

void Memory::SetArrayData(std::size_t index, Memory* object) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");

  auto& origin_data = GetOriginData(index);

  origin_data.type = 0x06;
  origin_data.data.array_data = object;
}

Object& Memory::GetOriginData(std::size_t index) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");
  std::reference_wrapper<Object> object = memory_[index];

  while (object.get().type == 0x07) {
    auto reference = object.get().data.reference_data;
    if (reference->is_class) {
      object = std::ref(reference->memory.class_memory
                            ->GetMembers()[*reference->index.variable_name]);
    } else {
      object = std::ref(
          reference->memory.memory->GetMemory()[reference->index.index]);
    }
  }

  return object;
}

uint64_t Memory::GetUint64tData(std::size_t index) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");

  Object object = GetOriginData(index);

  switch (object.type) {
    case 0x01:
      return object.data.byte_data;

    case 0x02:
      return object.data.int_data;

    case 0x03:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return object.data.float_data;

    case 0x04:
      return object.data.uint64t_data;

    default:
      LOGGING_ERROR("Unsupported data type.");
      return 0;
  }
}

std::string Memory::GetStringData(std::size_t index) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");

  Object object = GetOriginData(index);

  if (object.type != 0x05) {
    LOGGING_ERROR("Expected string type, but got: " +
                  std::to_string(object.type));
    return "";
  }

  return *object.data.string_data;
}

void ClassMemory::Add(std::string name) { members_[name] = {0x00, 0, false}; }

void ClassMemory::AddWithType(std::string name, uint8_t type) {
  members_[name] = {type, 0, type != 0x00};
}

void ClassMemory::AddByte(std::string name, int8_t value) {
  members_[name] = {0x01, value, true};
}

void ClassMemory::AddLong(std::string name, int64_t value) {
  Object object;
  object.type = 0x02;
  object.data.int_data = value;
  members_[name] = object;
}

void ClassMemory::AddDouble(std::string name, double value) {
  Object object;
  object.type = 0x03;
  object.data.float_data = value;
  members_[name] = object;
}

void ClassMemory::AddUint64t(std::string name, uint64_t value) {
  Object object;
  object.type = 0x04;
  object.data.uint64t_data = value;
  members_[name] = object;
}

void ClassMemory::AddString(std::string name, std::string value) {
  Object object;
  object.type = 0x05;
  object.data.string_data = new std::string(value);
  members_[name] = object;
}

void ClassMemory::AddReference(std::string name, Memory* memory,
                               std::size_t index) {
  ObjectReference* reference = new ObjectReference{false, memory, index};

  Object object;
  object.type = 0x07;
  object.data.reference_data = reference;
  members_[name] = object;
}

void ClassMemory::AddReference(std::string name, ClassMemory* memory,
                               std::string index) {
  ObjectReference* reference = new ObjectReference();
  reference->is_class = true;
  reference->memory.class_memory = memory;
  reference->index.variable_name = new std::string(index);

  Object object;
  object.type = 0x07;
  object.data.reference_data = reference;
  object.constant_type = true;
  members_[name] = object;
}

Object& ClassMemory::GetOriginData(std::string index) {
  if (members_.find(index) == members_.end())
    LOGGING_ERROR("Class member not found: " + index);
  std::reference_wrapper<Object> object = members_[index];

  while (object.get().type == 0x07) {
    auto reference = object.get().data.reference_data;
    if (reference->is_class) {
      object = std::ref(reference->memory.class_memory
                            ->GetMembers()[*reference->index.variable_name]);
    } else {
      object = std::ref(
          reference->memory.memory->GetMemory()[reference->index.index]);
    }
  }

  return object;
}

}  // namespace Interpreter
}  // namespace Aq