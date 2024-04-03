// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_HASH_MAP_HASH_MAP_H_
#define AQ_COMPILER_HASH_MAP_HASH_MAP_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "compiler/dyn_array/dyn_array.h"
#include "compiler/linked_list/linked_list.h"
#include "compiler/pair/pair.h"

namespace Aq {
template <typename ValueType>
class Compiler::HashMap {
 public:
  HashMap(std::size_t init_capacity = 1024);
  ~HashMap();

  HashMap(const HashMap&) = default;
  HashMap(HashMap&&) noexcept = default;
  HashMap& operator=(const HashMap&) = default;
  HashMap& operator=(HashMap&&) noexcept = default;

  // Insert a new pair into the hash table.
  void Insert(std::string key, ValueType value);

  // Find the key in a hash table and store it in value. Returns true if found,
  // false otherwise.
  bool Find(std::string key, ValueType& value);

 private:
  // The memory size of the hash table.
  std::size_t capacity_ = 0;

  // The number of elements in the hash table.
  std::size_t size_ = 0;

  // The data collection of the hash table is stored in a linked list of type
  // PairList.
  DynArray<LinkedList<Pair<std::string, std::string>>>* pair_list_ = nullptr;

  // The hash function. Based on DJB2 hashing algorithm.
  unsigned int Hash(std::string key) const;

  // Re-allocate the memory of the hash table.
  int Resize();
};
}  // namespace Aq

#endif