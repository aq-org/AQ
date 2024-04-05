// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_DYN_ARRAY_DYN_ARRAY_H_
#define AQ_COMPILER_DYN_ARRAY_DYN_ARRAY_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "debugger/debugger.h"

namespace Aq {
template <typename ArrayType>
class Compiler::DynArray {
 public:
  /// \brief Creates and initialize a dynamic array. The default capacity is 1.
  /// \param InitCapacity
  DynArray(std::size_t InitCapacity = 1);
  ~DynArray();

  /// \bug These functions have many bugs when called.
  /// \todo Fix these bugs.
  DynArray(const DynArray&) = default;
  DynArray(DynArray&&) noexcept = default;
  DynArray& operator=(const DynArray&) = default;
  DynArray& operator=(DynArray&&) noexcept = default;

  /// \fn operator[]
  /// \brief Returns the data reference of the corresponding index.
  /// \return The data reference of the corresponding index.
  ArrayType& operator[](std::size_t index);

  /// \fn Insert
  /// \brief Adds an element to the end of the dynamic array.
  void Insert(ArrayType data);

  /// \fn Remove
  /// \brief Removes an element from the end of the dynamic array.
  /// \param index std::size_t Type
  void Remove(std::size_t index);

  /// \fn Resize
  /// \brief Increase the dynamaic array capacity.
  /// \details If `new_capacity` is `0`, `capacity_` is increased to `1`. If
  /// greater than `0`, the container capacity is increased to `new_capacity`.
  /// \param new_capacity
  /// \return A return of `0` indicates a successful allocation.
  /// A return of `-1` indicates an error in the allocation.
  int Resize(std::size_t new_capacity = 0);

  /// \fn Size
  /// \brief Returns the number of elements in the container.
  /// \return the number of elements in the container.
  std::size_t Size() const;

  /// \fn Clear
  /// \brief Clears the contents of the dynamic array.
  void Clear();

  /// \class Iterator
  /// \brief The iterator of the dynamic array.
  class Iterator {
   public:
    using iterator_category = std::random_access_iterator_tag;

    Iterator operator+(std::size_t size) const;
    Iterator& operator++();
    Iterator operator++(int);
    Iterator operator-(std::size_t size) const;
    std::size_t operator-(const Iterator& other) const;
    Iterator& operator--();
    Iterator operator--(int);
    Iterator& operator+=(std::size_t size);
    Iterator& operator-=(std::size_t size);
    ArrayType& operator*() const;
    ArrayType* operator->() const;
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;
    bool operator<(const Iterator& other) const;
    bool operator<=(const Iterator& other) const;
    bool operator>(const Iterator& other) const;
    bool operator>=(const Iterator& other) const;

   private:
    /// \fn Iterator
    /// \brief Creates and initialize a dynamic array iterator.
    /// \param array DynArray<ArrayType>* Type
    /// \param index std::size_t Type, optional, default value is 0
    /// \note Generally, it is created by the Begin and End functions. It should
    /// not be created by others.
    Iterator(DynArray<ArrayType>* array, std::size_t index = 0);

    /// \fn ~Iterator
    /// \brief Destroys the iterator.
    /// \note Generally, it is called by the system.
    ~Iterator();

    /// \brief The dynamic array pointer.
    DynArray<ArrayType>* array_;

    /// @brief The index of the iterator.
    std::size_t index_;
  };

  /// \fn Begin
  /// \brief Returns an iterator to the beginning of the container.
  /// \return an iterator to the beginning of the container.
  Iterator Begin();

  /// \fn End
  /// \brief Returns an iterator to the end of the container.
  /// \return an iterator to the end of the container.
  Iterator End();

 private:
  /// @brief The data of the dynamic array.
  ArrayType* data_;

  /// @brief The capacity of the dynamic array.
  std::size_t capacity_;

  /// @brief The size of the dynamic array.
  std::size_t size_;
};
}  // namespace Aq

#endif