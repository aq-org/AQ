// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/memory.h"

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
  LOGGING_WARNING(
      "This method has been deprecated and there may be risks associated with "
      "its use.");
  memory_.push_back({type, 0, true});

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
  // TODO
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
  ObjectReference* reference = new ObjectReference();
  reference->is_class = false;
  reference->memory.memory = memory;
  reference->index.index = index;

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
  // TODO
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");

  auto& origin_data = GetOriginData(index);

  if (origin_data.constant_type &&
      (origin_data.type.empty() || origin_data.type[0] != 0x09))
    LOGGING_ERROR("Cannot set data to constant type memory.");

  origin_data.type = {0x09};
  origin_data.data = object;
  origin_data.guard_tag = 0x09;
  origin_data.guard_ptr = nullptr;
}

void Memory::SetObjectData(std::size_t index, ClassMemory* object) {
  // TODO
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");
  if (memory_[index].constant_data)
    LOGGING_ERROR("Cannot set data to constant memory.");

  auto& origin_data = GetOriginData(index);

  if (origin_data.constant_type &&
      (origin_data.type.empty() || origin_data.type[0] != 0x09))
    LOGGING_ERROR("Cannot set data to constant type memory.");

  origin_data.type = {0x09};
  origin_data.data = object;
  origin_data.guard_tag = 0x09;
  origin_data.guard_ptr = nullptr;
}

void Memory::SetArrayData(std::size_t index, Memory* object) {
  // TODO
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
      static_cast<void*>(&std::get<Memory*>(origin_data.data)));
}

Object& Memory::GetOriginData(std::size_t index) {
  // TODO
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");
  std::reference_wrapper<Object> object = memory_[index];
  if (object.get().type.empty()) INTERNAL_ERROR("Object type is empty.");

  while (object.get().type[0] == 0x07) {
    LOGGING_INFO("Get reference object.");
    auto reference = std::get<ObjectReference>(object.get().data);
    if (std::holds_alternative<Memory*>(reference.memory)) {
      object =
          std::ref(std::get<Memory*>(reference.memory)
                       ->GetMemory()[std::get<std::size_t>(reference.index)]);
    } else {
      object =
          std::ref(std::get<ClassMemory*>(reference.memory)
                       ->GetMembers()[std::get<std::string>(reference.index)]);
    }
  }

  if (object.get().guard_tag == 0x00) {
    object.get().guard_tag = object.get().type[0];
    switch (object.get().guard_tag) {
      case 0x00:
        object.get().guard_ptr = nullptr;
        break;
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
      case 0x06:
      case 0x09:
        object.get().guard_ptr = nullptr;
        break;
      default:
        LOGGING_ERROR("Unexpected object type guard tag: " +
                      std::to_string(object.get().guard_tag));
    }
  }

  return object;
}

void Memory::GetLastReference(ObjectReference& object) {
  // TODO
  Object temp;
  if (std::holds_alternative<Memory*>(object.memory)) {
    temp = std::get<Memory*>(object.memory)
               ->GetMemory()[std::get<std::size_t>(object.index)];
  } else {
    temp = std::get<ClassMemory*>(object.memory)
               ->GetMembers()[std::get<std::string>(object.index)];
  }

  while (temp.type[0] == 0x07) {
    if (std::holds_alternative<Memory*>(object.memory)) {
      temp = std::get<Memory*>(object.memory)
                 ->GetMemory()[std::get<std::size_t>(object.index)];
      if (temp.type[0] == 0x07) return;
      object = ObjectReference{std::get<Memory*>(object.memory),
                               std::get<std::size_t>(object.index)};
    } else {
      temp = std::get<ClassMemory*>(object.memory)
                 ->GetMembers()[std::get<std::string>(object.index)];
      if (temp.type[0] == 0x07) return;
      object = ObjectReference{std::get<ClassMemory*>(object.memory),
                               std::get<std::string>(object.index)};
    }
  }
}

uint64_t Memory::GetUint64tData(std::size_t index) {
  // TODO
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
  // TODO
  if (index >= memory_.size()) INTERNAL_ERROR("Out of memory.");

  Object object = GetOriginData(index);

  if (object.type[0] != 0x05) {
    LOGGING_ERROR("Expected string type, but got: " +
                  std::to_string(object.type[0]));
    return "";
  }

  return std::get<std::string>(object.data);
}

void ClassMemory::Add(std::string name) { members_[name] = {0x00, 0, false}; }

void ClassMemory::AddWithType(std::string name, uint8_t type) {
  LOGGING_WARNING(
      "This method has been deprecated and there may be risks associated with "
      "its use.");
  members_[name] = {type, 0, true};
}

void ClassMemory::AddByte(std::string name, int8_t value) {
  members_[name] = {0x01, value, true};
}

void ClassMemory::AddLong(std::string name, int64_t value) {
  Object object;
  object.type = 0x02;
  object.data.int_data = value;
  object.constant_type = true;
  members_[name] = object;
}

void ClassMemory::AddDouble(std::string name, double value) {
  Object object;
  object.type = 0x03;
  object.data.float_data = value;
  object.constant_type = true;
  members_[name] = object;
}

void ClassMemory::AddUint64t(std::string name, uint64_t value) {
  Object object;
  object.type = 0x04;
  object.data.uint64t_data = value;
  object.constant_type = true;
  members_[name] = object;
}

void ClassMemory::AddString(std::string name, std::string value) {
  Object object;
  object.type = 0x05;
  object.data.string_data = new std::string(value);
  object.constant_type = true;
  members_[name] = object;
}

void ClassMemory::AddReference(std::string name, Memory* memory,
                               std::size_t index) {
  ObjectReference reference = new ObjectReference{memory, index};
  Object object;
  object.type = 0x07;
  object.data.reference_data = reference;
  object.constant_type = true;
  members_[name] = object;
}

void ClassMemory::AddReference(std::string name, ClassMemory* memory,
                               std::string index) {
  ObjectReference reference = new ObjectReference();
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
  // TODO
  if (members_.find(index) == members_.end())
    LOGGING_ERROR("Class member not found: " + index);
  std::reference_wrapper<Object> object = members_[index];
  if (object.get().type.empty()) INTERNAL_ERROR("Object type is empty.");

  while (object.get().type[0] == 0x07) {
    auto reference = std::get<ObjectReference>(object.get().data);
    if (std::holds_alternative<Memory*>(reference.memory)) {
      object =
          std::ref(std::get<Memory*>(reference.memory)
                       ->GetMemory()[std::get<std::size_t>(reference.index)]);
    } else {
      object =
          std::ref(std::get<ClassMemory*>(reference.memory)
                       ->GetMembers()[std::get<std::string>(reference.index)]);
    }
  }

  if (object.get().guard_tag == 0x00) {
    object.get().guard_tag = object.get().type[0];
    switch (object.get().guard_tag) {
      case 0x00:
        object.get().guard_ptr = nullptr;
        break;
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
        object.get().guard_ptr =
            static_cast<void*>(&std::get<Memory*>(object.get().data));
        break;
      case 0x09:
        object.get().guard_ptr =
            static_cast<void*>(&std::get<ClassMemory*>(object.get().data));
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