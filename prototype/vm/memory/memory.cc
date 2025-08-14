// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "vm/memory/memory.h"

#include <memory>
#include <string>

#include "vm/logging/logging.h"

namespace Aq {
namespace Vm {
namespace Memory {
void ObjectReference::SetType(std::vector<uint8_t> type) {
  memory_.get()[index_].type = type;
}
void ObjectReference::SetData(int8_t data) {
  memory_.get()[index_].data = data;
}
void ObjectReference::SetData(int64_t data) {
  memory_.get()[index_].data = data;
}
void ObjectReference::SetData(double data) {
  memory_.get()[index_].data = data;
}
void ObjectReference::SetData(uint64_t data) {
  memory_.get()[index_].data = data;
}
void ObjectReference::SetData(std::string data) {
  memory_.get()[index_].data = data;
}
void ObjectReference::SetData(std::shared_ptr<struct Object> data) {
  memory_.get()[index_].data = data;
}
void ObjectReference::SetData(void* data) { memory_.get()[index_].data = data; }
void ObjectReference::SetData(ObjectReference* data) {
  memory_.get()[index_].data = *data;
}
void ObjectReference::SetData(
    std::variant<int8_t, int64_t, double, uint64_t, std::string, void*,
                 std::shared_ptr<struct Object>, ObjectReference>* data) {
  memory_.get()[index_].data = *data;
}

void ObjectReference::SetConstant(bool const_type) {
  if (index_ >= memory_.get().size()) {
    LOGGING_ERROR("Invalid reference data.");
  }
  memory_.get()[index_].const_type = const_type;
}

ObjectReference GetOriginDataReference(std::vector<Object>& heap,
                                       size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  auto object = ObjectReference(heap, index);

  while (true) {
    switch (object.Get().type[0]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x06:
      case 0x08:
      case 0x09:
      case 0x0A:
        return object;

      case 0x07:
        object = std::get<ObjectReference>(object.Get().data);
        break;
    }
  }

  INTERNAL_ERROR("Unexpected running location.");
  return object;
}

Object& GetOriginData(std::vector<Object>& heap, size_t index) {
  std::reference_wrapper<Object> origin_data = heap[index];
  while (true) {
    switch (origin_data.get().type[0]) {
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x06:
      case 0x09:
      case 0x0A:
        return origin_data;

      case 0x07:
      case 0x08: {
        auto object = std::get<ObjectReference>(origin_data.get().data);
        origin_data = std::ref(object.Get());
        break;
      }

      default:
        LOGGING_ERROR("Unsupported data type." +
                      std::to_string(origin_data.get().type[0]));
        break;
    }
  }
}

ObjectReference GetLastReference(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = ObjectReference(heap, index);

  if (!object.Get().const_type && object.Get().type[0] != 0x07) {
    object.Get().type.resize(2);
    object.Get().type[0] = 0x07;  // Set as reference type.
    object.Get().type[1] = 0x00;  // Set as non-const reference.
    object.Get().data = ObjectReference(heap, index);
    object.Get().const_type = false;
    heap[index] = object.Get();  // Update the heap with the new reference.
    return object;
  }

  if (object.Get().type[0] != 0x07) LOGGING_ERROR("Not a reference.");

  while (true) {
    if (object.Get().type.size() == 1) {
      if (object.Get().type[0] == 0x07) return object;
      LOGGING_ERROR("Not a reference.");
    }

    switch (object.Get().type[1]) {
      case 0x07:
        object.Set(std::get<ObjectReference>(object.Get().data));
        break;

      case 0x08:
        LOGGING_ERROR("Cannot change const data.");
        break;

      default:
        if (object.Get().type[0] != 0x07) LOGGING_ERROR("Not a reference.");
        return object;
    }
  }

  INTERNAL_ERROR("Unexpected running location.");
  return object;
}

ObjectReference GetLastDataReference(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = ObjectReference(heap, index);

  while (true) {
    if (object.Get().type.size() <= 0)
      LOGGING_ERROR("Object type is empty at index " + std::to_string(index));

    switch (object.Get().type[0]) {
      case 0x07: {
        if (object.Get().type.size() >= 2 && object.Get().type[1] == 0x00) {
          object.SetConstant(false);
        } else {
          object.SetConstant(true);
        }

        auto object_reference = std::get<ObjectReference>(object.Get().data);
        object.Set(object_reference);

        break;
      }

      default:
        return object;
    }
  }

  INTERNAL_ERROR("Unexpected running location.");
  return object;
}

uint8_t GetObjectType(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  if (heap[index].type.empty()) INTERNAL_ERROR("Object type is empty.");

  return heap[index].type.back();
}

std::vector<Object>& GetArrayData(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Object& object = GetOriginData(heap, index);

  if (object.type[0] != 0x06) LOGGING_ERROR("Unsupported array type.");

  return std::get<ObjectReference>(object.data).GetMemory().get();
}

int8_t GetByteData(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Object& object = GetOriginData(heap, index);

  switch (object.type[0]) {
    case 0x01:
      return std::get<int8_t>(object.data);

    case 0x02:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return std::get<int64_t>(object.data);

    case 0x03:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return std::get<double>(object.data);

    case 0x04:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return std::get<uint64_t>(object.data);

    default:
      LOGGING_ERROR("Unsupported data type.");
      return 0;
  }
}

int64_t GetLongData(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  // LOGGING_INFO("Get long data at index " + std::to_string(index) +
  //    " with type " + std::to_string(heap[index].type[0]) + ".");
  Object& object = GetOriginData(heap, index);

  switch (object.type[0]) {
    case 0x01:
      return std::get<int8_t>(object.data);

    case 0x02:
      return std::get<int64_t>(object.data);

    case 0x03:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return std::get<double>(object.data);

    case 0x04:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return std::get<uint64_t>(object.data);

    default:
      LOGGING_ERROR("Unsupported data type.");
      return 0;
  }
}

double GetDoubleData(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Object& object = GetOriginData(heap, index);

  switch (object.type[0]) {
    case 0x01:
      return std::get<int8_t>(object.data);

    case 0x02:
      return std::get<int64_t>(object.data);

    case 0x03:
      return std::get<double>(object.data);

    case 0x04:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return std::get<uint64_t>(object.data);

    default:
      LOGGING_ERROR("Unsupported data type.");
      return 0;
  }
}

uint64_t GetUint64tData(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Object& object = GetOriginData(heap, index);

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
      // LOGGING_INFO(std::get<std::string>(object.data));
      LOGGING_ERROR("Unsupported data type.");
      return 0;
  }
}

std::string GetStringData(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Object& object = GetOriginData(heap, index);

  if (object.type[0] != 0x05) LOGGING_ERROR("Unsupported data type.");

  return std::get<std::string>(object.data);
}

std::vector<Object>& GetObjectData(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Object& object = GetOriginData(heap, index);

  if (object.type[0] != 0x09) {
    LOGGING_INFO("Object type: " + std::to_string(object.type[0]) +
                 std::to_string(object.type[1]));
    LOGGING_ERROR("Unsupported data type.");
  }

  return std::get<ObjectReference>(object.data).GetMemory().get();
}

void SetArrayData(std::vector<Object>& heap, size_t index,
                  ObjectReference array) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = GetLastDataReference(heap, index);

  if (!object.Get().const_type || object.Get().type[0] == 0x06) {
    object.Get().type[0] = 0x06;
    auto temp = new std::vector<struct Object>(array.GetMemory());

    object.Get().data = ObjectReference(*temp, 0);
  }
}

void SetByteData(std::vector<Object>& heap, size_t index, int8_t value) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = GetLastDataReference(heap, index);

  if (!object.Get().const_type || object.Get().type[0] == 0x01) {
    object.SetType({0x01});
    object.SetData(value);
  }
}

void SetLongData(std::vector<Object>& heap, size_t index, int64_t value) {
  // LOGGING_INFO("1Set long data at index " + std::to_string(index)+", Heap
  // Size:"+ std::to_string(heap.size())+".");
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  // LOGGING_INFO("2Set long data at index " + std::to_string(index) +
  //    " with type " + std::to_string(heap[index].type[0]) + ".");

  // LOGGING_INFO("01 Set long data at index " + std::to_string(index)+", Heap
  // Size:"+ std::to_string(heap.size())+".");

  auto object = GetLastDataReference(heap, index);

  // LOGGING_INFO("02 Set long data at index " + std::to_string(index)+", Heap
  // Size:"+ std::to_string(heap.size())+".");

  // LOGGING_INFO("3(END)Set long data at index " + std::to_string(index) +
  //      " with type " + std::to_string(object.Get().type[0]) + ".");

  if (!object.Get().const_type || object.Get().type[0] == 0x02) {
    object.SetType({0x02});
    object.SetData(value);
  }
}

void SetDoubleData(std::vector<Object>& heap, size_t index, double value) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = GetLastDataReference(heap, index);

  if (!object.Get().const_type || object.Get().type[0] == 0x03) {
    object.SetType({0x03});
    object.SetData(value);
  }
}

void SetUint64tData(std::vector<Object>& heap, size_t index, uint64_t value) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = GetLastDataReference(heap, index);

  if (!object.Get().const_type || object.Get().type[0] == 0x04) {
    object.SetType({0x04});
    object.SetData(value);
  }
}

void SetStringData(std::vector<Object>& heap, size_t index,
                   std::string string) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = GetLastDataReference(heap, index);

  if (!object.Get().const_type || object.Get().type[0] == 0x05) {
    object.SetType({0x05});
    object.SetData(string);
  }
}

void SetReferenceData(std::vector<Object>& heap, size_t index,
                      std::vector<struct Object>& reference_memory,
                      std::size_t reference_index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto reference_object = GetLastReference(heap, index);

  if (reference_object.Get().type.size() < 2)
    INTERNAL_ERROR("Unexpected type size.");

  if (reference_object.Get().type[1] != 0x00 &&
      reference_object.Get().type[1] != reference_object.Get().type.back())
    LOGGING_ERROR("Cannot change reference type.");

  auto object = ObjectReference(reference_memory, reference_index);

  reference_object.SetData(&object);
}

void SetConstData(std::vector<Object>& heap, size_t index,
                  std::vector<struct Object>& reference_memory,
                  std::size_t reference_index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto const_object = GetLastDataReference(heap, index);

  if (reference_memory[reference_index].const_type &&
      reference_memory[reference_index].type[0] != 0x08)
    LOGGING_ERROR("Cannot change reference type.");

  if (const_object.Get().type.size() < 2)
    INTERNAL_ERROR("Unexpected type size.");

  if (const_object.Get().type[1] != 0x00 &&
      const_object.Get().type[1] !=
          reference_memory[reference_index].type.back())
    LOGGING_ERROR("Cannot change reference type.");

  auto object = ObjectReference(reference_memory, reference_index);

  const_object.SetData(&object);
}

void SetObjectData(std::vector<Object>& heap, size_t index,
                   ObjectReference objects) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto const_object = GetLastDataReference(heap, index);

  if (const_object.Get().type.size() < 1)
    INTERNAL_ERROR("Unexpected type size.");

  if (!const_object.Get().const_type || const_object.Get().type[0] == 0x09) {
    const_object.Get().type[0] = 0x09;
    auto temp = new std::vector<struct Object>(objects.GetMemory());

    const_object.Get().data = ObjectReference(*temp, 0);
  }

  // LOGGING_INFO("Set object data at index " + std::to_string(index) +
  //  " with type " + std::to_string(heap[index].type[0]) + ".");
}

/*int SetOriginData(std::vector<Object>& heap, size_t index, void* object) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto origin_object = GetLastDataReference(heap, index);

  if (!origin_object.first[object.second].const_type ||
origin_object.first[object.second].type[0] == 0x0A) {
    origin_object.first[object.second].type[0] = 0x0A;
    origin_object.first[object.second].data = object;
  } else {
    LOGGING_ERROR("Unsupported data type.");
  }

  return 0;
}*/

}  // namespace Memory
}  // namespace Vm
}  // namespace Aq