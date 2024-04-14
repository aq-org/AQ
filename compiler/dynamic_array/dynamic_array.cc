// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/dynamic_array/dynamic_array.h"

#include <cstddef>

#include "compiler/dynamic_array/const_iterator.h"
#include "compiler/dynamic_array/const_reverse_iterator.h"
#include "compiler/dynamic_array/iterator.h"
#include "compiler/dynamic_array/reverse_iterator.h"
#include "debugger/debugger.h"

namespace Aq {
template <typename T>
Compiler::DynamicArray<T>::DynamicArray() = default;
template <typename T>
Compiler::DynamicArray<T>::DynamicArray(std::size_t capacity, const T& value) {
  array_ = new T[size];
  if (array_ == nullptr) {
    Debugger error(
        Debugger::Level::ERROR, "Aq::Compiler::DynamicArray::DynamicArray",
        "DynamicArray_InitError", "Size out of memory occurred.", nullptr);
    return;
  }
  capacity_ = capacity;
  fill(value);
}
template <typename T>
Compiler::DynamicArray<T>::~DynamicArray() {
  delete[] array_;
}

template <typename T>
Compiler::DynamicArray<T>::DynamicArray(const DynamicArray& other) {
  *this = other;
}
template <typename T>
Compiler::DynamicArray<T>::DynamicArray(DynamicArray&& other) noexcept {
  *this = other;
}
template <typename T>
Compiler::DynamicArray<T>& Compiler::DynamicArray<T>::operator=(
    const DynamicArray& other) {
  if (this != &other) {
    clear();
    array_ = new T[other.capacity_];
    if (array_ == nullptr) {
      Debugger error(
          Debugger::Level::ERROR, "Aq::Compiler::DynamicArray::DynamicArray",
          "DynamicArray_InitError", "Size out of memory occurred.", nullptr);
      return *this;
    }
    capacity_ = other.capacity_;
    size_ = other.size_;
    for (size_t i = 0; i < size_; ++i) {
      array_[i] = other.array_[i];
    }
  }
  return *this;
}
template <typename T>
Compiler::DynamicArray<T>& Compiler::DynamicArray<T>::operator=(
    DynamicArray&& other) noexcept {
  if (this != &other) {
    clear();
    array_ = other.array_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    other.array_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }
  return *this;
}

template <typename T>
typename Compiler::DynamicArray<T>::iterator Compiler::DynamicArray<T>::begin() {
  return iterator(this, 0);
}
template <typename T>
typename Compiler::DynamicArray<T>::const_iterator Compiler::DynamicArray<T>::begin()
    const {
  return const_iterator(this, 0);
}
template <typename T>
typename Compiler::DynamicArray<T>::const_iterator Compiler::DynamicArray<T>::cbegin()
    const {
  return const_iterator(this, 0);
}
template <typename T>
typename Compiler::DynamicArray<T>::reverse_iterator
Compiler::DynamicArray<T>::rbegin() {
  return reverse_iterator(this, 0);
}
template <typename T>
typename Compiler::DynamicArray<T>::const_reverse_iterator
Compiler::DynamicArray<T>::rbegin() const {
  return const_reverse_iterator(this, 0);
}
template <typename T>
typename Compiler::DynamicArray<T>::const_reverse_iterator
Compiler::DynamicArray<T>::crbegin() const {
  return const_reverse_iterator(this, 0);
}
template <typename T>
typename Compiler::DynamicArray<T>::iterator Compiler::DynamicArray<T>::end() {
  return iterator(this, size_);
}
template <typename T>
typename Compiler::DynamicArray<T>::const_iterator Compiler::DynamicArray<T>::end()
    const {
  return const_iterator(this, size_);
}
template <typename T>
typename Compiler::DynamicArray<T>::const_iterator Compiler::DynamicArray<T>::cend()
    const {
  return const_iterator(this, size_);
}
template <typename T>
typename Compiler::DynamicArray<T>::reverse_iterator Compiler::DynamicArray<T>::rend() {
  return reverse_iterator(this, size_);
}
template <typename T>
typename Compiler::DynamicArray<T>::const_reverse_iterator
Compiler::DynamicArray<T>::rend() const {
  return const_reverse_iterator(this, size_);
}
template <typename T>
typename Compiler::DynamicArray<T>::const_reverse_iterator
Compiler::DynamicArray<T>::crend() const {
  return const_reverse_iterator(this, size_);
}

template <typename T>
std::size_t Compiler::DynamicArray<T>::size() const {
  return size_;
}
template <typename T>
bool Compiler::DynamicArray<T>::empty() const {
  if (size_ == 0) {
    return true;
  }
  return false;
}
template <typename T>
std::size_t Compiler::DynamicArray<T>::capacity() const {
  return capacity_;
}
template <typename T>
void Compiler::DynamicArray<T>::reserve(std::size_t count) {
  if (count <= capacity_) {
    return;
  }
  T* new_array = new T[count];
  if (new_array == nullptr) {
    Debugger error(
        Debugger::Level::ERROR, "Aq::Compiler::DynamicArray::DynamicArray",
        "DynamicArray_InitError", "Size out of memory occurred.", nullptr);
    return;
  }
  for (size_t i = 0; i < size_; ++i) {
    new_array[i] = array_[i];
  }
  delete[] array_;
  array_ = new_array;
  capacity_ = count;
}
template <typename T>
void Compiler::DynamicArray<T>::shrink_to_fit() {
  if (size_ == capacity_) {
    return;
  }
  T* new_array = new T[size_];
  if (new_array == nullptr) {
    Debugger error(
        Debugger::Level::ERROR, "Aq::Compiler::DynamicArray::DynamicArray",
        "DynamicArray_InitError", "Size out of memory occurred.", nullptr);
    return;
  }
  for (size_t i = 0; i < size_; ++i) {
    new_array[i] = array_[i];
  }
  delete[] array_;
  array_ = new_array;
  capacity_ = size_;
}

template <typename T>
T& Compiler::DynamicArray<T>::operator[](std::size_t index) {
  return at(index);
}
template <typename T>
const T& Compiler::DynamicArray<T>::operator[](std::size_t index) const {
  return at(index);
}
template <typename T>
T& Compiler::DynamicArray<T>::at(std::size_t index) {
  if (index >= size_) {
    Debugger error(Debugger::Level::ERROR, "Aq::Compiler::DynamicArray::at",
                   "IndexOutOfRangeError", "The index is out of range.",
                   nullptr);
    return T();
  }
  return at(index);
}
template <typename T>
const T& Compiler::DynamicArray<T>::at(std::size_t index) const {
  if (index >= size_) {
    Debugger error(Debugger::Level::ERROR, "Aq::Compiler::DynamicArray::at",
                   "IndexOutOfRangeError", "The index is out of range.",
                   nullptr);
    return T();
  }
  return array_[index];
}
template <typename T>
T& Compiler::DynamicArray<T>::front() {
  return at(0);
}
template <typename T>
const T& Compiler::DynamicArray<T>::front() const {
  return at(0);
}
template <typename T>
T& Compiler::DynamicArray<T>::back() {
  return at(size_ - 1);
}
template <typename T>
const T& Compiler::DynamicArray<T>::back() const {
  return at(size_ - 1);
}
template <typename T>
T* Compiler::DynamicArray<T>::data() {
  return array_;
}
template <typename T>
const T* Compiler::DynamicArray<T>::data() const {
  return array_;
}

template <typename T>
void Compiler::DynamicArray<T>::push_back(const T& value) {
  if (size_ > capacity_) {
    Debugger error(
        Debugger::Level::ERROR, "Aq::Compiler::DynamicArray::push_back",
        "PushBack_SizeError", "Size out of capacity occurred.", nullptr);
    return;
  }
  if (size_ == capacity_) {
    reserve(1);
    reserve(capacity_ * 2);
  }
  array_[size_] = value;
  ++size_;
}
template <typename T>
void Compiler::DynamicArray<T>::push_back(T&& value) {
  if (size_ > capacity_) {
    Debugger error(
        Debugger::Level::ERROR, "Aq::Compiler::DynamicArray::push_back",
        "PushBack_SizeError", "Size out of capacity occurred.", nullptr);
    return;
  }
  if (size_ == capacity_) {
    reserve(1);
    reserve(capacity_ * 2);
  }
  array_[size_] = value;
  ++size_;
}
template <typename T>
typename Compiler::DynamicArray<T>::iterator Compiler::DynamicArray<T>::insert(
    typename Compiler::DynamicArray<T>::const_iterator position, const T& value) {
  if (size_ > capacity_) {
    Debugger error(Debugger::Level::ERROR, "Aq::Compiler::DynamicArray::insert",
                   "Insert_SizeError", "Size out of capacity occurred.",
                   nullptr);
    return iterator();
  }
  if (size_ == capacity_) {
    reserve(1);
    reserve(capacity_ * 2);
  }
  for (std::size_t i = size_; i > position.index_; --i) {
    array_[i] = array_[i - 1];
  }
  array_[position.index] = value;
  ++size_;
  return iterator(this, position.index);
}
template <typename T>
typename Compiler::DynamicArray<T>::iterator Compiler::DynamicArray<T>::insert(
    typename Compiler::DynamicArray<T>::const_iterator position, T&& value) {
  if (size_ > capacity_) {
    Debugger error(Debugger::Level::ERROR, "Aq::Compiler::DynamicArray::insert",
                   "Insert_SizeError", "Size out of capacity occurred.",
                   nullptr);
    return iterator();
  }
  if (size_ == capacity_) {
    reserve(1);
    reserve(capacity_ * 2);
  }
  for (std::size_t i = size_; i > position.index_; --i) {
    array_[i] = array_[i - 1];
  }
  array_[position.index] = value;
  ++size_;
  return iterator(this, position.index);
}
template <typename T>
typename Compiler::DynamicArray<T>::iterator Compiler::DynamicArray<T>::insert(
    typename Compiler::DynamicArray<T>::const_iterator position, std::size_t count,
    const T& value) {
  if (size_ > capacity_) {
    Debugger error(Debugger::Level::ERROR, "Aq::Compiler::DynamicArray::insert",
                   "Insert_SizeError", "Size out of capacity occurred.",
                   nullptr);
    return iterator();
  }
  while (size_ + count > capacity_) {
    reserve(1);
    reserve(capacity_ * 2);
  }
  for (std::size_t i = size_ + count - 1; i > position.index_; --i) {
    array_[i] = array_[i - count];
  }
  for (std::size_t i = 0; i < count; ++i) {
    array_[position.index + i] = value;
  }
  size_ += count;
  return iterator(this, position.index);
}
template <typename T>
typename Compiler::DynamicArray<T>::iterator Compiler::DynamicArray<T>::insert(
    typename Compiler::DynamicArray<T>::const_iterator position, iterator first,
    iterator last) {}
}  // namespace Aq