// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/hash_map/hash_map.h"

#include <cstddef>

#include "compiler/compiler.h"
#include "compiler/dyn_array/dyn_array.h"
#include "debugger/debugger.h"

namespace Aq {
template <typename ValueType>
Compiler::HashMap<ValueType>::HashMap(std::size_t init_capacity) {
  pair_list_ = new DynArray<ValueType>(init_capacity);
  if (!pair_list_) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::Lexer::HashMap::HashMap",
                   "HashMap_MemoryError", "Memory allocation failed.", nullptr);
    return;
  }
  capacity_ = init_capacity;
}
template <typename ValueType>
Compiler::HashMap<ValueType>::~HashMap() {
  delete pair_list_;
}

template <typename ValueType>
void Compiler::HashMap<ValueType>::Insert(std::string key, ValueType value) {
  unsigned int hash = Hash(key);

  // Increase the size of the hash table.
  size_++;
  if (size_ / capacity_ > 0.8) {
    Resize();
  }

  // Create key-value pairs and insert them into the linked list.
  Pair<std::string,std::string> pair;
  pair.first = key;
  pair.second = value;
  pair_list_[hash].Insert(pair_list_[hash].End(),pair);
}

template <typename ValueType>
ValueType Compiler::HashMap<ValueType>::Find(std::string key) {
  unsigned int hash = Hash(key);
  return pair_list_[hash].Find(key);
}

template <typename ValueType>
unsigned int Compiler::HashMap<ValueType>::Hash(std::string key) const {
  unsigned int hash = 5381;
  for (char character : key) {
    // hash = hash * 33 + static_cast<unsigned int>(character)
    hash = ((hash << 5) + hash) + static_cast<unsigned int>(character);
  }
  hash = hash % capacity_;
  return hash;
};

template <typename ValueType>
int Compiler::HashMap<ValueType>::Resize() {
  PairList* temp = pair_list_;
  std::size_t new_capacity = capacity_ * 1.5;
  pair_list_ = new PairList[new_capacity];

  // Memory allocation failed.
  if (!pair_list_) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::Lexer::HashMap::Resize", "Resize_MemoryError",
                   "Memory allocation failed.", nullptr);
    return -1;
  }

  // Copy data.
  for (int i = 0; i < capacity_; i++) {
    temp[i].CopyDataToNewList(pair_list_, new_capacity);
  }

  // Release the memory of the original linked list.
  delete[] temp;

  capacity_ = new_capacity;
  return 0;
}

}  // namespace Aq