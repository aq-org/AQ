// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/dynamic_array/reverse_iterator.h"

#include <cstddef>

#include "debugger/debugger.h"

namespace Aq {
template <typename T>
Compiler::DynamicArray<T>::reverse_iterator::reverse_iterator(
    DynamicArray<T>* array, std::size_t index) {
  array_ = array;
  index_ = array_->size() - 1 - index;
  if (!CheckIfTheIndexIsInRange(index_)) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynamicArray::reverse_iterator::operator++",
                   "operator++_IndexError", "Index out of range.", nullptr);
    index_ = 0;
    return;
  }
  if (index_ == -1) {
    data_ = T();
    return;
  }
  data_ = array_->at(index_);
}
template <typename T>
Compiler::DynamicArray<T>::reverse_iterator::~reverse_iterator() = default;

template <typename T>
T& Compiler::DynamicArray<T>::reverse_iterator::operator*() const {
  return data_;
}
template <typename T>
T* Compiler::DynamicArray<T>::reverse_iterator::operator->() const {
  return &data_;
}

template <typename T>
Compiler::DynamicArray<T>::reverse_iterator&
Compiler::DynamicArray<T>::reverse_iterator::operator++() {
  return *this += 1;
}
template <typename T>
Compiler::DynamicArray<T>::reverse_iterator
Compiler::DynamicArray<T>::reverse_iterator::operator++(int) {
  reverse_iterator copy = *this;
  *this += 1;
  return copy;
}

template <typename T>
Compiler::DynamicArray<T>::reverse_iterator&
Compiler::DynamicArray<T>::reverse_iterator::operator--() {
  return *this -= 1;
}
template <typename T>
Compiler::DynamicArray<T>::reverse_iterator
Compiler::DynamicArray<T>::reverse_iterator::operator--(int) {
  reverse_iterator copy = *this;
  *this -= 1;
  return copy;
}

template <typename T>
Compiler::DynamicArray<T>::reverse_iterator
Compiler::DynamicArray<T>::reverse_iterator::operator+(std::ptrdiff_t n) const {
  reverse_iterator copy = *this;
  copy += n;
  return copy;
}
template <typename T>
Compiler::DynamicArray<T>::reverse_iterator&
Compiler::DynamicArray<T>::reverse_iterator::operator+=(std::ptrdiff_t n) {
  index_ -= n;
  if (!CheckIfTheIndexIsInRange(index_)) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynamicArray::reverse_iterator::operator++",
                   "operator++_IndexError", "Index out of range.", nullptr);
    index_ += n;
    return *this;
  }
  if (index_ == -1) {
    data_ = T();
    return *this;
  }
  data_ = array_->at(index_);
  return *this;
}

template <typename T>
Compiler::DynamicArray<T>::reverse_iterator
Compiler::DynamicArray<T>::reverse_iterator::operator-(std::ptrdiff_t n) const {
  reverse_iterator copy = *this;
  copy -= n;
  return copy;
}
template <typename T>
Compiler::DynamicArray<T>::reverse_iterator
Compiler::DynamicArray<T>::reverse_iterator::operator-=(std::ptrdiff_t n) {
  index_ += n;
  if (!CheckIfTheIndexIsInRange(index_)) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynamicArray::reverse_iterator::operator--",
                   "operator--_IndexError", "Index out of range.", nullptr);
    index_ -= n;
    return *this;
  }
  if (index_ == -1) {
    data_ = T();
    return *this;
  }
  data_ = array_->at(index_);
  return *this;
}

template <typename T>
Compiler::DynamicArray<T>::reverse_iterator&
Compiler::DynamicArray<T>::reverse_iterator::operator[](std::size_t n) {
  std::size_t original_index = index_;
  index_ = n;
  if (!CheckIfTheIndexIsInRange(index_)) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::DynamicArray::reverse_iterator::operator[]",
                   "operator[]_IndexError", "Index out of range.", nullptr);
    index_ = original_index;
    return *this;
  }
  if (index_ == -1) {
    data_ = T();
    return *this;
  }
  data_ = array_->at(index_);
  return *this;
}

template <typename T>
bool Compiler::DynamicArray<T>::reverse_iterator::operator==(
    const reverse_iterator& other) const {
  return array_ == other.array_ && index_ == other.index_;
}
template <typename T>
bool Compiler::DynamicArray<T>::reverse_iterator::operator!=(
    const reverse_iterator& other) const {
  return !(*this == other);
}

template <typename T>
std::ptrdiff_t Compiler::DynamicArray<T>::reverse_iterator::operator-(
    const reverse_iterator& other) const {
  return other.index_ - index_;
}

template <typename T>
bool Compiler::DynamicArray<T>::reverse_iterator::CheckIfTheIndexIsInRange(
    std::size_t index) const {
  if (index_ > array_->size() - 1 || index_ < -1) {
    return true;
  }
  return false;
}

template <typename T>
Compiler::DynamicArray<T>::reverse_iterator&
Compiler::DynamicArray<T>::reverse_iterator::HandleIteratorMoves(
    std::size_t index) {
        
    }
}  // namespace Aq