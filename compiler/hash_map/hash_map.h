// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_HASH_MAP_HASH_MAP_H_
#define AQ_COMPILER_HASH_MAP_HASH_MAP_H_

#include "compiler/compiler.h"

#include <cstddef>

#include "compiler/dyn_array/dyn_array.h"

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

  // Find the value of a key.
  ValueType Find(std::string key);

 private:
  struct Pair {
    std::string key;
    ValueType value;
  };

  // A linked list of Pair type, used to resolve hash conflicts.
  class PairList {
   public:
    // Construct a PairList class.
    PairList();
    ~PairList();

    PairList(const PairList&) = default;
    PairList(PairList&&) noexcept = default;
    PairList& operator=(const PairList&) = default;
    PairList& operator=(PairList&&) noexcept = default;

    // Prepend a new pair to the list.
    void Prepend(Pair value);

    // Copy all the data in the linked list to |new_list|.
    void CopyDataToNewList(PairList* new_list, size_t new_capacity);

    // Append a new pair to the list. It is not recommended to use it when
    // dealing with large amounts of data.
    void Append(Pair value);

    // Find the value of a key.
    ValueType Find(std::string key);

   private:
    // A node type of the linked list.
    struct Node {
      Pair data;
      Node* next = nullptr;
      Node(Pair pair) : data(pair){};
    };

    // The head pointer of the linked list.
    Node* head_ptr_ = nullptr;
  };

  // The memory size of the hash table.
  std::size_t capacity_ = 1;

  // The number of elements in the hash table.
  std::size_t size_ = 0;

  // The data collection of the hash table is stored in a linked list of type
  // PairList.
  DynArray<ValueType>* pair_list_ = nullptr;

  // The hash function. Based on DJB2 hashing algorithm.
  unsigned int Hash(std::string key) const;

  // Re-allocate the memory of the hash table.
  int Resize();
};
}  // namespace Aq

#endif