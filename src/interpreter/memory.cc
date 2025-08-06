// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/memory.h"

#include "logging/logging.h"

namespace Aq {
namespace Interpreter {
std::size_t Memory::Add(std::size_t size, bool is_constant_data) {
  std::size_t index = memory_.size();
  for (size_t i = 0; i < size; i++) {
    memory_.push_back(
        {{0x00}, static_cast<int64_t>(0), false, is_constant_data});
  }

  return index;
}

std::size_t Memory::AddWithType(std::vector<uint8_t> type,
                                bool is_constant_data) {
  memory_.push_back({type, static_cast<int64_t>(0), true, is_constant_data});

  return memory_.size() - 1;
}

std::size_t Memory::AddByte(int8_t value, bool is_constant_data) {
  memory_.push_back({{0x01}, value, true, is_constant_data});
  return memory_.size() - 1;
}

std::size_t Memory::AddLong(int64_t value, bool is_constant_data) {
  memory_.push_back({{0x02}, value, true, is_constant_data});
  return memory_.size() - 1;
}

std::size_t Memory::AddDouble(double value, bool is_constant_data) {
  memory_.push_back({{0x03}, value, true, is_constant_data});
  return memory_.size() - 1;
}

std::size_t Memory::AddUint64t(uint64_t value, bool is_constant_data) {
  memory_.push_back({{0x04}, value, true, is_constant_data});
  return memory_.size() - 1;
}

void Memory::SetUint64tValue(std::size_t index, uint64_t value,
                             bool is_constant_data) {
  memory_.at(index) = {{0x04}, value, true, is_constant_data};
}

std::size_t Memory::AddString(std::string value, bool is_constant_data) {
  memory_.push_back({{0x05}, value, true, is_constant_data});
  return memory_.size() - 1;
}

std::size_t Memory::AddReference(std::shared_ptr<Memory> memory,
                                 std::size_t index, std::vector<uint8_t> type,
                                 bool is_constant_data) {
  ObjectReference reference = {memory, index};
  memory_.push_back({type, reference, true, is_constant_data});
  return memory_.size() - 1;
}

std::size_t Memory::AddReference(std::shared_ptr<ClassMemory> memory,
                                 std::string index, std::vector<uint8_t> type,
                                 bool is_constant_data) {
  ObjectReference reference = {memory, index};
  memory_.push_back({type, reference, true, is_constant_data});
  return memory_.size() - 1;
}

void Memory::InitObjectData(std::size_t index,
                            std::shared_ptr<ClassMemory> object) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");

  auto& origin_data = GetOriginData(index);

  if (origin_data.constant_type &&
      (origin_data.type.empty() || origin_data.type[0] != 0x07))
    LOGGING_ERROR("Cannot set data to constant type memory.");

  origin_data.type = {0x09};
  origin_data.data = object;
  origin_data.guard_tag = 0x09;
  origin_data.guard_ptr = static_cast<void*>(static_cast<void*>(
      &std::get<std::shared_ptr<ClassMemory>>(origin_data.data)));
}

void Memory::SetObjectData(std::size_t index,
                           std::shared_ptr<ClassMemory> object) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");
  if (memory_[index].constant_data)
    LOGGING_ERROR("Cannot set data to constant memory.");

  auto& origin_data = GetOriginData(index);

  if (origin_data.constant_type &&
      (origin_data.type.empty() || origin_data.type[0] != 0x07))
    LOGGING_ERROR("Cannot set data to constant type memory.");

  origin_data.type = {0x09};
  origin_data.data = object;
  origin_data.guard_tag = 0x09;
  origin_data.guard_ptr = static_cast<void*>(static_cast<void*>(
      &std::get<std::shared_ptr<ClassMemory>>(origin_data.data)));
}

void Memory::SetArrayData(std::size_t index, std::shared_ptr<Memory> object) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");
  if (memory_[index].constant_data)
    LOGGING_ERROR("Cannot set data to constant memory.");

  auto& origin_data = GetOriginData(index);

  if (origin_data.constant_type) {
    if (origin_data.type.empty() || origin_data.type[0] != 0x06)
      LOGGING_ERROR("Cannot set data to constant type memory.");

  } else {
    origin_data.type = {0x06, 0x00};
  }

  origin_data.data = object;
  origin_data.guard_tag = 0x06;
  origin_data.guard_ptr = static_cast<void*>(
      static_cast<void*>(&std::get<std::shared_ptr<Memory>>(origin_data.data)));
}

Object& Memory::GetOriginData(std::size_t index) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");
  std::reference_wrapper<Object> object = memory_[index];
  if (object.get().type.empty()) INTERNAL_ERROR("Object type is empty.");

  while (object.get().type[0] == 0x07) {
    auto reference = std::get<ObjectReference>(object.get().data);
    if (std::holds_alternative<std::shared_ptr<Memory>>(reference.memory)) {
      object =
          std::ref(std::get<std::shared_ptr<Memory>>(reference.memory)
                       ->GetMemory()[std::get<std::size_t>(reference.index)]);
    } else {
      object =
          std::ref(std::get<std::shared_ptr<ClassMemory>>(reference.memory)
                       ->GetMembers()[std::get<std::string>(reference.index)]);
    }
  }

  if (object.get().guard_tag == 0x00) {
    object.get().guard_tag = object.get().type[0];
    switch (object.get().guard_tag) {
      case 0x01:
        object.get().guard_ptr =
            static_cast<void*>(&std::get<int8_t>(object.get().data));
        break;
      case 0x02:
        object.get().guard_ptr =
            static_cast<void*>(&std::get<int64_t>(object.get().data));
        break;
      case 0x03:
        object.get().guard_ptr =
            static_cast<void*>(&std::get<double>(object.get().data));
        break;
      case 0x04:
        object.get().guard_ptr =
            static_cast<void*>(&std::get<uint64_t>(object.get().data));
        break;
      case 0x05:
        object.get().guard_ptr =
            static_cast<void*>(&std::get<std::string>(object.get().data));
        break;
      case 0x06:
        object.get().guard_ptr = static_cast<void*>(
            &std::get<std::shared_ptr<Memory>>(object.get().data));
        break;
      case 0x09:
        object.get().guard_ptr = static_cast<void*>(
            &std::get<std::shared_ptr<ClassMemory>>(object.get().data));
        break;
      default:
        LOGGING_ERROR("Unexpected object type guard tag: " +
                      std::to_string(object.get().guard_tag));
    }
  }

  return object;
}

void Memory::GetLastReference(ObjectReference& object) {
  Object temp;
  if (std::holds_alternative<std::shared_ptr<Memory>>(object.memory)) {
    temp = std::get<std::shared_ptr<Memory>>(object.memory)
               ->GetMemory()[std::get<std::size_t>(object.index)];
  } else {
    temp = std::get<std::shared_ptr<ClassMemory>>(object.memory)
               ->GetMembers()[std::get<std::string>(object.index)];
  }

  while (temp.type[0] == 0x07) {
    if (std::holds_alternative<std::shared_ptr<Memory>>(object.memory)) {
      temp = std::get<std::shared_ptr<Memory>>(object.memory)
                 ->GetMemory()[std::get<std::size_t>(object.index)];
      if (temp.type[0] == 0x07) return;
      object = ObjectReference{std::get<std::shared_ptr<Memory>>(object.memory),
                               std::get<std::size_t>(object.index)};
    } else {
      temp = std::get<std::shared_ptr<ClassMemory>>(object.memory)
                 ->GetMembers()[std::get<std::string>(object.index)];
      if (temp.type[0] == 0x07) return;
      object = ObjectReference{std::get<std::shared_ptr<ClassMemory>>(object.memory),
                               std::get<std::string>(object.index)};
    }
  }
}

uint64_t Memory::GetUint64tData(std::size_t index) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");

  Object object = GetOriginData(index);

  switch (object.type[0]) {
    case 0x01:
      return std::get<int8_t>(object.data);

    case 0x02:
      return std::get<int64_t>(object.data);

    case 0x03:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return std::get<double>(object.data);

    case 0x04:
      return std::get<uint64_t>(object.data);

    default:
      LOGGING_ERROR("Unsupported data type.");
      return 0;
  }
}

std::string Memory::GetStringData(std::size_t index) {
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");

  Object object = GetOriginData(index);

  if (object.type[0] != 0x05) {
    LOGGING_ERROR("Expected string type, but got: " +
                  std::to_string(object.type[0]));
    return "";
  }

  return std::get<std::string>(object.data);
}

void ClassMemory::Add(std::string name, bool is_constant_data) {
  members_[name] = {{0x00}, static_cast<int64_t>(0), false, is_constant_data};
}

void ClassMemory::AddWithType(std::string name, std::vector<uint8_t> type,
                              bool is_constant_data) {
  members_[name] = {type, static_cast<int64_t>(0), true, is_constant_data};
}

void ClassMemory::AddByte(std::string name, int8_t value,
                          bool is_constant_data) {
  members_[name] = {{0x01}, value, true, is_constant_data};
}

void ClassMemory::AddLong(std::string name, int64_t value,
                          bool is_constant_data) {
  members_[name] = {{0x02}, value, true, is_constant_data};
}

void ClassMemory::AddDouble(std::string name, double value,
                            bool is_constant_data) {
  members_[name] = {{0x03}, value, true, is_constant_data};
}

void ClassMemory::AddUint64t(std::string name, uint64_t value,
                             bool is_constant_data) {
  members_[name] = {{0x04}, value, true, is_constant_data};
}

void ClassMemory::AddString(std::string name, std::string value,
                            bool is_constant_data) {
  members_[name] = {{0x05}, value, true, is_constant_data};
}

void ClassMemory::AddReference(std::string name, std::shared_ptr<Memory> memory,
                               std::size_t index, std::vector<uint8_t> type,
                               bool is_constant_data) {
  ObjectReference reference = {memory, index};
  members_[name] = {type, reference, true, is_constant_data};
}

void ClassMemory::AddReference(std::string name,
                               std::shared_ptr<ClassMemory> memory,
                               std::string index, std::vector<uint8_t> type,
                               bool is_constant_data) {
  ObjectReference reference = {memory, index};
  members_[name] = {type, reference, true, is_constant_data};
}

Object& ClassMemory::GetOriginData(std::string index) {
  if (members_.find(index) == members_.end())
    LOGGING_ERROR("Class member not found: " + index);
  std::reference_wrapper<Object> object = members_[index];
  if (object.get().type.empty()) INTERNAL_ERROR("Object type is empty.");

  while (object.get().type[0] == 0x07) {
    auto reference = std::get<ObjectReference>(object.get().data);
    if (std::holds_alternative<std::shared_ptr<Memory>>(reference.memory)) {
      object =
          std::ref(std::get<std::shared_ptr<Memory>>(reference.memory)
                       ->GetMemory()[std::get<std::size_t>(reference.index)]);
    } else {
      object =
          std::ref(std::get<std::shared_ptr<ClassMemory>>(reference.memory)
                       ->GetMembers()[std::get<std::string>(reference.index)]);
    }
  }

  if (object.get().guard_tag == 0x00) {
    object.get().guard_tag = object.get().type[0];
    switch (object.get().guard_tag) {
      case 0x01:
        object.get().guard_ptr =
            static_cast<void*>(&std::get<int8_t>(object.get().data));
        break;
      case 0x02:
        object.get().guard_ptr =
            static_cast<void*>(&std::get<int64_t>(object.get().data));
        break;
      case 0x03:
        object.get().guard_ptr =
            static_cast<void*>(&std::get<double>(object.get().data));
        break;
      case 0x04:
        object.get().guard_ptr =
            static_cast<void*>(&std::get<uint64_t>(object.get().data));
        break;
      case 0x05:
        object.get().guard_ptr =
            static_cast<void*>(&std::get<std::string>(object.get().data));
        break;
      case 0x06:
        object.get().guard_ptr = static_cast<void*>(
            &std::get<std::shared_ptr<Memory>>(object.get().data));
        break;
      case 0x09:
        object.get().guard_ptr = static_cast<void*>(
            &std::get<std::shared_ptr<ClassMemory>>(object.get().data));
        break;
      default:
        LOGGING_ERROR("Unexpected object type guard tag: " +
                      std::to_string(object.get().guard_tag));
    }
  }

  return object;
}

}  // namespace Interpreter
}  // namespace Aq