// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_DYN_ARRAY_DYN_ARRAY_H_
#define AQ_COMPILER_DYN_ARRAY_DYN_ARRAY_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "debugger/debugger.h"

namespace Aq {
template <typename ArrayType, std::size_t InitCapacity>
class Compiler::DynArray {
 public:
  DynArray();
  ~DynArray();

  DynArray(const DynArray&) = default;
  DynArray(DynArray&&) noexcept = default;
  DynArray& operator=(const DynArray&) = default;
  DynArray& operator=(DynArray&&) noexcept = default;

  // Returns the data reference of the corresponding index.
  ArrayType& operator[](std::size_t index) {
    if (index >= size_) {
      Debugger error_info(
          Debugger::Level::ERROR, "Aq::Compiler::DynArray::operator[]",
          "DynArray_IndexError", "Index out of range occurred.", nullptr);
    }
    return data_[index];
  }

  // Adds an element to the end of the container. No return value.
  void PushBack(ArrayType data);

  // Increase the container capacity.
  // If |new_capacity| is 0, it is increased to 2 times |capacity_|. If
  // greater than 0, the container capacity is increased to |new_capacity|.
  // A return of 0 indicates a successful allocation, a return of -1 indicates
  // an error in the allocation.
  int Resize(std::size_t new_capacity = 0);

  class Iterator {
   public:
    using iterator_category = std::random_access_iterator_tag;

    Iterator(DynArray<ArrayType, InitCapacity>* array, std::size_t index = 0);

    ~Iterator();

    Iterator operator+(std::size_t size) const {
      return Iterator(array_, index_ + size);
    }

    friend Iterator operator+(std::size_t size, const Iterator& it) {
      return it + size;
    }

    Iterator& operator++() {
      ++index_;
      if (index_ >= array_->size_) {
        Debugger error_info(Debugger::Level::ERROR,
                            "Aq::Compiler::DynArray::Iterator::operator+=",
                            "operator++_IndexError", "Index out of range.",
                            nullptr);
      }
      return *this;
    }

    Iterator operator++(int) {
      Iterator temp(*this);
      ++(*this);
      return temp;
    }

    Iterator operator-(std::size_t size) const {
      return Iterator(array_, index_ - size);
    }

    std::size_t operator-(const Iterator& other) const {
      return static_cast<std::size_t>(index_) -
             static_cast<std::size_t>(other.index_);
    }

    Iterator& operator--() {
      --index_;
      if (index_ < 0) {
        Debugger error_info(Debugger::Level::ERROR,
                            "Aq::Compiler::DynArray::Iterator::operator--",
                            "operator--_IndexError", "Index out of range.",
                            nullptr);
      }
      return *this;
    }

    Iterator operator--(int) {
      Iterator temp(*this);
      --(*this);
      return temp;
    }

    Iterator& operator+=(std::size_t size) {
      index_ += size;
      if (index_ >= array_->size_) {
        Debugger error_info(Debugger::Level::ERROR,
                            "Aq::Compiler::DynArray::Iterator::operator+=",
                            "operator+=_IndexError", "Index out of range.",
                            nullptr);
      }
      return *this;
    }

    Iterator& operator-=(std::size_t size) {
      index_ -= size;
      if (index_ < 0) {
        Debugger error_info(Debugger::Level::ERROR,
                            "Aq::Compiler::DynArray::Iterator::operator-=",
                            "operator-=_IndexError", "Index out of range.",
                            nullptr);
      }
      return *this;
    }

    ArrayType& operator*() const {
      if (index_ >= array_->size_) {
        Debugger error_info(Debugger::Level::ERROR,
                            "Aq::Compiler::DynArray::Iterator::operator*",
                            "operator*_IndexError", "Index out of range.",
                            nullptr);
      }
      return (*array_)[index_];
    }

    ArrayType* operator->() const { return &(operator*()); }

    bool operator==(const Iterator& other) const {
      return array_ == other.array_ && index_ == other.index_;
    }

    bool operator!=(const Iterator& other) const { return !(*this == other); }

    bool operator<(const Iterator& other) const {
      return index_ < other.index_;
    }
    bool operator<=(const Iterator& other) const {
      return index_ <= other.index_;
    }
    bool operator>(const Iterator& other) const {
      return index_ > other.index_;
    }
    bool operator>=(const Iterator& other) const {
      return index_ >= other.index_;
    }

   private:
    DynArray<ArrayType, InitCapacity>* array_;
    std::size_t index_;
  };

  // Returns an iterator to the beginning of the container.
  Iterator Begin() { return Iterator(this, 0); }

  // Returns an iterator to the end of the container.
  Iterator End() { return Iterator(this, size_); }

 private:
  ArrayType* data_;
  std::size_t capacity_;
  std::size_t size_;
};
}  // namespace Aq

#endif