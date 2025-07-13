// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_VM_MEMORY_MEMORY_H_
#define AQ_VM_MEMORY_MEMORY_H_

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <functional>

namespace Aq {
namespace Vm {
namespace Memory {
class ObjectReference{
public:
//ObjectReference() = default;
~ObjectReference()=default;

ObjectReference(std::vector<struct Object>& memory,std::size_t index): memory_(memory),  
index_(index) {
}

struct Object& Get(){
  return memory_.get()[index_];
}

std::reference_wrapper<std::vector<struct Object>> GetMemory(){
  return memory_;
}

std::size_t GetIndex(){
  return index_;
}

bool operator==(const ObjectReference& other) const noexcept {
  return &memory_.get() == &other.memory_.get() && 
         index_ == other.index_;
}

private:
std::reference_wrapper<std::vector<struct Object>> memory_;
std::size_t index_ = 0;
};

using Data = std::variant<int8_t, int64_t, double, uint64_t, std::string,
                          std::vector<struct Object>,
                          std::shared_ptr<struct Object>, void*, ObjectReference>;

struct Object {
  std::vector<uint8_t> type;
  bool const_type;
  Data data;
};

struct Memory {
  std::vector<Object> heap;
  std::vector<Object> constant_pool;
};

inline bool operator==(const Object& lhs, const Object& rhs) {
  return lhs.type == rhs.type && lhs.data == rhs.data;
}

ObjectReference GetOriginDataReference(std::vector<Object>& heap,
                                               size_t index);

Object GetOriginData(std::vector<Object>& heap, size_t index);

ObjectReference GetLastReference(std::vector<Object>& heap,
                                         size_t index);

ObjectReference GetLastDataReference(std::vector<Object>& heap,
                                             size_t index);

uint8_t GetObjectType(std::vector<Object>& heap, size_t index);

std::vector<Object> GetArrayData(std::vector<Object>& heap, size_t index);

int8_t GetByteData(std::vector<Object>& heap, size_t index);

int64_t GetLongData(std::vector<Object>& heap, size_t index);

double GetDoubleData(std::vector<Object>& heap, size_t index);

uint64_t GetUint64tData(std::vector<Object>& heap, size_t index);

std::string GetStringData(std::vector<Object>& heap, size_t index);

std::vector<Object> GetObjectData(std::vector<Object>& heap, size_t index);

void SetArrayData(std::vector<Object>& heap, size_t index,
                  std::vector<Object> array);

void SetByteData(std::vector<Object>& heap, size_t index, int8_t value);

void SetLongData(std::vector<Object>& heap, size_t index, int64_t value);

void SetDoubleData(std::vector<Object>& heap, size_t index, double value);

void SetUint64tData(std::vector<Object>& heap, size_t index, uint64_t value);

void SetStringData(std::vector<Object>& heap, size_t index, std::string string);

void SetReferenceData(std::vector<Object>& heap, size_t index,
  std::vector<struct Object>& reference_memory,std::size_t reference_index);

void SetConstData(std::vector<Object>& heap, size_t index,std::vector<struct Object>& reference_memory,std::size_t reference_index);

void SetObjectData(std::vector<Object>& heap, size_t index,
                   std::vector<Object> object);

}  // namespace Memory
}  // namespace Vm
}  // namespace Aq

#endif