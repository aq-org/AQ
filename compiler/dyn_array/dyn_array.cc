/// Copyright 2024 AQ authors, All Rights Reserved.
/// This program is licensed under the AQ License. You can find the AQ license in
/// the root directory.

#include "compiler/dyn_array/dyn_array.h"

#include <cstddef>

#include "debugger/debugger.h"

namespace Aq {
template <typename ArrayType>
Compiler::DynArray<ArrayType>::DynArray(std::size_t InitCapacity) {
  data_ = new ArrayType[InitCapacity];
  if (data_ != nullptr) {
    capacity_ = InitCapacity;
  } else {
    Debugger error(Debugger::Level::ERROR, "Aq::Compiler::DynArray::DynArray",
                   "DynArray_InitError", "Size out of memory occurred.",
                   nullptr);
    capacity_ = 0;
  }
  size_ = 0;
}

template <typename ArrayType>
Compiler::DynArray<ArrayType>::~DynArray() {
  delete[] data_;
}

template <typename ArrayType>
ArrayType& Compiler::DynArray<ArrayType>::operator[](std::size_t index) {
  return data_[index];
}

template <typename ArrayType>
void Compiler::DynArray<ArrayType>::Insert(ArrayType data) {
  if (capacity_ == 0 && Resize(1) == -1) {
    return;
  }
  if (size_ > capacity_) {
    Debugger error(Debugger::Level::ERROR, "Aq::Compiler::DynArray::Insert",
                   "Insert_SizeError", "Size out of capacity occurred.",
                   nullptr);
    return;
  }
  if (size_ == capacity_) {
    if (Resize(capacity_ * 2) != 0) {
      Debugger Error(Debugger::Level::ERROR, "Aq::Compiler::DynArray::Insert",
                     "Insert_ResizeError", "Resize out of memory occurred.",
                     nullptr);
      return;
    }
  }
  data_[size_] = data;
  ++size_;
}

template <typename ArrayType>
int Compiler::DynArray<ArrayType>::Resize(std::size_t new_capacity) {
  if (new_capacity == 0) {
    ++capacity_;
  } else {
    capacity_ = new_capacity;
  }
  ArrayType* new_data = new ArrayType[capacity_];
  if (new_data == nullptr) {
    Debugger error(Debugger::Level::ERROR, "Aq::Compiler::DynArray::Resize",
                   "Resize_ResizeError", "New capacity out of memory occurred.",
                   nullptr);
    return -1;
  }
  for (std::size_t i = 0; i < size_ && i < capacity_; ++i) {
    new_data[i] = data_[i];
  }
  delete[] data_;
  data_ = new_data;
  return 0;
}

template <typename ArrayType>
std::size_t Compiler::DynArray<ArrayType>::Size() const {
  return size_;
}

template <typename ArrayType>
Compiler::DynArray<ArrayType>::Iterator::Iterator(DynArray<ArrayType>* array,
                                                  std::size_t index)
    : array_(array), index_(index) {
  if (index_ >= array_->size_) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynArray::Iterator::Iterator",
                   "Iterator_IndexError", "Index out of range.", nullptr);
  }
}
template <typename ArrayType>
Compiler::DynArray<ArrayType>::Iterator::~Iterator() = default;
template <typename ArrayType>
typename Compiler::DynArray<ArrayType>::Iterator
Compiler::DynArray<ArrayType>::Iterator::operator+(std::size_t size) const {
  return Iterator(array_, index_ + size);
}
template <typename ArrayType>
typename Compiler::DynArray<ArrayType>::Iterator&
Compiler::DynArray<ArrayType>::Iterator::operator++() {
  ++index_;
  if (index_ >= array_->size_) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynArray::Iterator::operator+=",
                   "operator++_IndexError", "Index out of range.", nullptr);
  }
  return *this;
}
template <typename ArrayType>
typename Compiler::DynArray<ArrayType>::Iterator
Compiler::DynArray<ArrayType>::Iterator::operator++(int) {
  Iterator temp(*this);
  ++(*this);
  return temp;
}
template <typename ArrayType>
typename Compiler::DynArray<ArrayType>::Iterator
Compiler::DynArray<ArrayType>::Iterator::operator-(std::size_t size) const {
  return Iterator(array_, index_ - size);
}
template <typename ArrayType>
std::size_t Compiler::DynArray<ArrayType>::Iterator::operator-(
    const Iterator& other) const {
  return static_cast<std::size_t>(index_) -
         static_cast<std::size_t>(other.index_);
}
template <typename ArrayType>
typename Compiler::DynArray<ArrayType>::Iterator&
Compiler::DynArray<ArrayType>::Iterator::operator--() {
  --index_;
  if (index_ < 0) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynArray::Iterator::operator--",
                   "operator--_IndexError", "Index out of range.", nullptr);
  }
  return *this;
}
template <typename ArrayType>
typename Compiler::DynArray<ArrayType>::Iterator
Compiler::DynArray<ArrayType>::Iterator::operator--(int) {
  Iterator temp(*this);
  --(*this);
  return temp;
}
template <typename ArrayType>
typename Compiler::DynArray<ArrayType>::Iterator&
Compiler::DynArray<ArrayType>::Iterator::operator+=(std::size_t size) {
  index_ += size;
  if (index_ >= array_->size_) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynArray::Iterator::operator+=",
                   "operator+=_IndexError", "Index out of range.", nullptr);
  }
  return *this;
}
template <typename ArrayType>
typename Compiler::DynArray<ArrayType>::Iterator&
Compiler::DynArray<ArrayType>::Iterator::operator-=(std::size_t size) {
  index_ -= size;
  if (index_ < 0) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynArray::Iterator::operator-=",
                   "operator-=_IndexError", "Index out of range.", nullptr);
  }
  return *this;
}
template <typename ArrayType>
ArrayType& Compiler::DynArray<ArrayType>::Iterator::operator*() const {
  if (index_ >= array_->size_) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynArray::Iterator::operator*",
                   "operator*_IndexError", "Index out of range.", nullptr);
  }
  return (*array_)[index_];
}
template <typename ArrayType>
ArrayType* Compiler::DynArray<ArrayType>::Iterator::operator->() const {
  return &(operator*());
}
template <typename ArrayType>
bool Compiler::DynArray<ArrayType>::Iterator::operator==(
    const Iterator& other) const {
  return array_ == other.array_ && index_ == other.index_;
}
template <typename ArrayType>
bool Compiler::DynArray<ArrayType>::Iterator::operator!=(
    const Iterator& other) const {
  return !(*this == other);
}
template <typename ArrayType>
bool Compiler::DynArray<ArrayType>::Iterator::operator<(
    const Iterator& other) const {
  return index_ < other.index_;
}
template <typename ArrayType>
bool Compiler::DynArray<ArrayType>::Iterator::operator<=(
    const Iterator& other) const {
  return index_ <= other.index_;
}
template <typename ArrayType>
bool Compiler::DynArray<ArrayType>::Iterator::operator>(
    const Iterator& other) const {
  return index_ > other.index_;
}
template <typename ArrayType>
bool Compiler::DynArray<ArrayType>::Iterator::operator>=(
    const Iterator& other) const {
  return index_ >= other.index_;
}

}  // namespace Aq