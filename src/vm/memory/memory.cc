// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "vm/memory/memory.h"

#include <memory>

#include "vm/logging/logging.h"

namespace Aq {
namespace Vm {
namespace Memory {
std::shared_ptr<Object> GetOriginDataReference(std::vector<Object>& heap,
                                               size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  auto object = std::shared_ptr<Object>(&heap[index], [](void*) {});

  while (true) {
    switch (object->type[0]) {
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
        object = std::get<std::shared_ptr<Object>>(object->data);
        break;
    }
  }

  INTERNAL_ERROR("Unexpected running location.");
  return object;
}

Object GetOriginData(std::vector<Object>& heap, size_t index) {
  Object origin_data = heap[index];
  while (true) {
    switch (origin_data.type[0]) {
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
      case 0x08:
        origin_data = *std::get<std::shared_ptr<Object>>(origin_data.data);
        break;

      default:
        LOGGING_ERROR("Object type is error at index " + std::to_string(index));
        LOGGING_ERROR("Unsupported data type."+std::to_string(origin_data.type[0]));
        break;
    }
  }
}

std::shared_ptr<Object> GetLastReference(std::vector<Object>& heap,
                                         size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = std::shared_ptr<Object>(&heap[index], [](void*) {});

  if(!object->const_type&&object->type[0] != 0x07){ 
    object->type.resize(2);
    object->type[0] = 0x07; // Set as reference type.
    object->type[1] = 0x00; // Set as non-const reference.
    object->data = std::make_shared<Object>(heap[index]);
    object->const_type = false;
    heap[index] = *object; // Update the heap with the new reference.
    return object;}

  if (object->type[0] != 0x07) LOGGING_ERROR("Not a reference.");

  while (true) {
    if (object->type.size() == 1) {
      if (object->type[0] == 0x07) return object;
      LOGGING_ERROR("Not a reference.");
    }

    switch (object->type[1]) {
      case 0x07:
        object = std::get<std::shared_ptr<Object>>(object->data);
        break;

      case 0x08:
        LOGGING_ERROR("Cannot change const data.");
        break;

      default:
        if (object->type[0] != 0x07) LOGGING_ERROR("Not a reference.");
        return object;
    }
  }

  INTERNAL_ERROR("Unexpected running location.");
  return object;
}

std::shared_ptr<Object> GetLastDataReference(std::vector<Object>& heap,
                                             size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = std::shared_ptr<Object>(&heap[index], [](void*) {});

  while (true) {
    switch (object->type[0]) {
      case 0x07:
        if (object->type.size() >= 2 && object->type[1] == 0x00) {
          object = std::get<std::shared_ptr<Object>>(object->data);
          object->const_type = false;
        } else {
          object = std::get<std::shared_ptr<Object>>(object->data);
          object->const_type = true;
        }
        break;

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

std::vector<Object> GetArrayData(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Object object = GetOriginData(heap, index);

  if (object.type[0] != 0x06) LOGGING_ERROR("Unsupported array type.");

  return std::get<std::vector<Object>>(object.data);
}

int8_t GetByteData(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Object object = GetOriginData(heap, index);

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

  Object object = GetOriginData(heap, index);

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

  Object object = GetOriginData(heap, index);

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

  Object object = GetOriginData(heap, index);

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

std::string GetStringData(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Object object = GetOriginData(heap, index);

  if (object.type[0] != 0x05) LOGGING_ERROR("Unsupported data type.");

  return std::get<std::string>(object.data);
}

std::vector<Object> GetObjectData(std::vector<Object>& heap, size_t index) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Object object = GetOriginData(heap, index);

  if (object.type[0] != 0x09) LOGGING_ERROR("Unsupported data type.");

  return std::get<std::vector<Object>>(object.data);
}

void SetArrayData(std::vector<Object>& heap, size_t index,
                  std::vector<Object> array) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = GetLastDataReference(heap, index);

  if (!object->const_type || object->type[0] == 0x06) {
    object->type[0] = 0x06;
    object->data = array;
  }
}

void SetByteData(std::vector<Object>& heap, size_t index, int8_t value) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = GetLastDataReference(heap, index);

  if (!object->const_type || object->type[0] == 0x01) {
    object->type[0] = 0x01;
    object->data = value;
  }
}

void SetLongData(std::vector<Object>& heap, size_t index, int64_t value) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = GetLastDataReference(heap, index);

  if (!object->const_type || object->type[0] == 0x02) {
    object->type[0] = 0x02;
    object->data = value;
  }
}

void SetDoubleData(std::vector<Object>& heap, size_t index, double value) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = GetLastDataReference(heap, index);

  if (!object->const_type || object->type[0] == 0x03) {
    object->type[0] = 0x03;
    object->data = value;
  }
}

void SetUint64tData(std::vector<Object>& heap, size_t index, uint64_t value) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = GetLastDataReference(heap, index);

  if (!object->const_type || object->type[0] == 0x04) {
    object->type[0] = 0x04;
    object->data = value;
  }
}

void SetStringData(std::vector<Object>& heap, size_t index,
                   std::string string) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto object = GetLastDataReference(heap, index);

  if (!object->const_type || object->type[0] == 0x05) {
    object->type[0] = 0x05;
    object->data = string;
  }
}

void SetReferenceData(std::vector<Object>& heap, size_t index,
                      std::shared_ptr<Object> object) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto reference_object = GetLastReference(heap, index);

  if (reference_object->type.size() < 2)
    INTERNAL_ERROR("Unexpected type size.");

  if (reference_object->type[1] != 0x00 &&
      reference_object->type[1] != object->type.back())
    LOGGING_ERROR("Cannot change reference type.");

  reference_object->data = object;
}

void SetConstData(std::vector<Object>& heap, size_t index,
                  std::shared_ptr<Object> object) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto const_object = GetLastDataReference(heap, index);

  if (object->const_type && object->type[0] != 0x08)
    LOGGING_ERROR("Cannot change reference type.");

  if (const_object->type.size() < 2) INTERNAL_ERROR("Unexpected type size.");

  if (const_object->type[1] != 0x00 &&
      const_object->type[1] != object->type.back())
    LOGGING_ERROR("Cannot change reference type.");

  const_object->data = object;
}

void SetObjectData(std::vector<Object>& heap, size_t index,
                   std::vector<Object> objects) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto const_object = GetLastDataReference(heap, index);

  if (const_object->type.size() < 1) INTERNAL_ERROR("Unexpected type size.");

  if (!const_object->const_type || const_object->type[0] == 0x09) {
    const_object->type[0] = 0x09;
    const_object->data = objects;
  }

  LOGGING_INFO("Set object data at index " + std::to_string(index) +
                   " with type " + std::to_string(heap[index].type[0]) + ".");
}

/*int SetOriginData(std::vector<Object>& heap, size_t index, void* object) {
  if (index >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (heap[index].type[0] == 0x08) LOGGING_ERROR("Cannot change const data.");

  auto origin_object = GetLastDataReference(heap, index);

  if (!origin_object->const_type || origin_object->type[0] == 0x0A) {
    origin_object->type[0] = 0x0A;
    origin_object->data = object;
  } else {
    LOGGING_ERROR("Unsupported data type.");
  }

  return 0;
}*/

}  // namespace Memory
}  // namespace Vm
}  // namespace Aq