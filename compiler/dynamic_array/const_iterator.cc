// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/dynamic_array/const_iterator.h"

#include <cstddef>

#include "debugger/debugger.h"

namespace Aq {
template <typename T>
Compiler::DynamicArray<T>::const_iterator::const_iterator(DynamicArray<T>* array,
                                              std::size_t index) {
  array_ = array;
  index_ = index;
  data_ = array_->at(index);
}
template <typename T>
Compiler::DynamicArray<T>::const_iterator::~const_iterator() = default;

template <typename T>
const T& Compiler::DynamicArray<T>::const_iterator::operator*() const {
  return data_;
}
template <typename T>
const T* Compiler::DynamicArray<T>::const_iterator::operator->() const {
  return &data_;
}

template <typename T>
Compiler::DynamicArray<T>::const_iterator&
Compiler::DynamicArray<T>::const_iterator::operator++() {
  return *this += 1;
}
template <typename T>
Compiler::DynamicArray<T>::const_iterator
Compiler::DynamicArray<T>::const_iterator::operator++(int) {
  const_iterator copy = *this;
  *this += 1;
  return copy;
}

template <typename T>
Compiler::DynamicArray<T>::const_iterator&
Compiler::DynamicArray<T>::const_iterator::operator--() {
  return *this -= 1;
}
template <typename T>
Compiler::DynamicArray<T>::const_iterator
Compiler::DynamicArray<T>::const_iterator::operator--(int) {
  const_iterator copy = *this;
  *this -= 1;
  return copy;
}

template <typename T>
Compiler::DynamicArray<T>::const_iterator
Compiler::DynamicArray<T>::const_iterator::operator+(std::ptrdiff_t n) const {
  const_iterator copy = *this;
  copy += n;
  return copy;
}
template <typename T>
Compiler::DynamicArray<T>::const_iterator&
Compiler::DynamicArray<T>::const_iterator::operator+=(std::ptrdiff_t n) {
  index_ += n;
  if (index_ > array_->size()) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynamicArray::const_iterator::operator++",
                   "operator++_IndexError", "Index out of range.", nullptr);
  }
  if (index_ == array_->size()) {
    data_ = T();
    return *this;
  }
  data_ = array_->at(index_);
  return *this;
}

template <typename T>
Compiler::DynamicArray<T>::const_iterator
Compiler::DynamicArray<T>::const_iterator::operator-(std::ptrdiff_t n) const {
  const_iterator copy = *this;
  copy -= n;
  return copy;
}
template <typename T>
Compiler::DynamicArray<T>::const_iterator
Compiler::DynamicArray<T>::const_iterator::operator-=(std::ptrdiff_t n) {
  index_ -= n;
  if (index_ < 0) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynamicArray::const_iterator::operator--",
                   "operator--_IndexError", "Index out of range.", nullptr);
  }
  if (index_ == array_->size()) {
    data_ = T();
    return *this;
  }
  data_ = array_->at(index_);
  return *this;
}

template <typename T>
Compiler::DynamicArray<T>::const_iterator&
Compiler::DynamicArray<T>::const_iterator::operator[](std::size_t n) {
  std::size_t original_index = index_;
  index_ = n;
  if (index_ > array_->size() || index_ < 0) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynamicArray::const_iterator::operator[]",
                   "operator[]_IndexError", "Index out of range.", nullptr);
    index_ = original_index;
    return *this;
  }
  if (index_ == array_->size()) {
    data_ = T();
    return *this;
  }
  data_ = array_->at(index_);
  return *this;
}

template <typename T>
bool Compiler::DynamicArray<T>::const_iterator::operator==(
    const const_iterator& other) const {
  return array_ == other.array_ && index_ == other.index_;
}
template <typename T>
bool Compiler::DynamicArray<T>::const_iterator::operator!=(
    const const_iterator& other) const {
  return !(*this == other);
}

template <typename T>
std::ptrdiff_t Compiler::DynamicArray<T>::const_iterator::operator-(
    const const_iterator& other) const {
  return index_ - other.index_;
}
}  // namespace Aq