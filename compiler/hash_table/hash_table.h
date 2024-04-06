// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_HASH_TABLE_HASH_TABLE_H_
#define AQ_COMPILER_HASH_TABLE_HASH_TABLE_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "compiler/dyn_array/dyn_array.h"
#include "compiler/linked_list/linked_list.h"
#include "compiler/pair/pair.h"

namespace Aq {
template <typename ValueType>
class Compiler::HashTable {
 public:
  /// \fn HashTable
  /// \brief Creates and initialize a hash table. 
  /// \param init_capacity std::size_t Type, optional, default value is 1024 
  HashTable(std::size_t init_capacity = 1024);
  ~HashTable();

  /// \bug These functions have many bugs when called.
  /// \todo Fix these bugs.
  HashTable(const HashTable&) = default;
  HashTable(HashTable&&) noexcept = default;
  HashTable& operator=(const HashTable&) = default;
  HashTable& operator=(HashTable&&) noexcept = default;

  /// \fn Insert
  /// \brief Insert a new value into the hash table.
  /// \param key std::string Type
  /// \param value ValueType Type
  void Insert(std::string key, ValueType value);

  /// \fn Find
  /// \brief Find the key in a hash table and store the corresponding value in value.
  /// \param key std::string Type
  /// \param value ValueType& Type, Get the corresponding value
  /// \return true if found, false otherwise.
  bool Find(std::string key, ValueType& value);

 private:
  /// \brief The memory size of the hash table. The default capacity is 0.
  std::size_t capacity_ = 0;

  /// \brief The number of elements in the hash table. The default size is 0.
  std::size_t size_ = 0;

  /// \brief The data collection of the hash table is stored in a linked list.
  DynArray<LinkedList<Pair<std::string, std::string>>>* pair_list_ = nullptr;

  /// \fn Hash
  /// \brief Get hash value of the key.
  /// \details Based on DJB2 hashing algorithm.
  /// \param key std::string Type
  /// \return hash value of the key. unsigned int Type.
  unsigned int Hash(std::string key) const;

  /// \fn Resize
  /// \brief Re-allocate the memory of the hash table.
  /// \return 0 if success, -1 if failed.
  int Resize();
};
}  // namespace Aq

#endif