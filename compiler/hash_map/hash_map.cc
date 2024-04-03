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
  auto hash = static_cast<std::size_t>(Hash(key));

  // Increase the size of the hash table.
  size_++;
  if (size_ / capacity_ > 0.8) {
    Resize();
  }

  LinkedList<Pair<std::string, std::string>> insert_list = pair_list_[hash];

  // Insert key-value pairs into the linked list.
  insert_list.Insert(insert_list.End(), {key, value});
}

template <typename ValueType>
ValueType* Compiler::HashMap<ValueType>::Find(std::string key) {
  auto hash = static_cast<std::size_t>(Hash(key));
  LinkedList<Pair<std::string, std::string>> find_list = pair_list_[hash];
  typename LinkedList<Pair<std::string, ValueType>>::Iterator temp_node =
      find_list.Begin();

  // Compare keys one by one to find the corresponding value.
  while (temp_node != find_list.End()) {
    if (key == *temp_node.first) {
      return *temp_node.second;
    }
    temp_node++;
  }

  return nullptr;
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
  DynArray<LinkedList<Pair<std::string, std::string>>>* temp = pair_list_;
  std::size_t new_capacity = capacity_ * 1.5;
  pair_list_ =
      new DynArray<LinkedList<Pair<std::string, std::string>>>[new_capacity];

  // Memory allocation failed.
  if (!pair_list_) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::Lexer::HashMap::Resize", "Resize_MemoryError",
                   "Memory allocation failed.", nullptr);
    return -1;
  }

  // Copy data.
  for (std::size_t i = 0; i < capacity_; i++) {
    LinkedList<Pair<std::string, ValueType>>& origin_list = temp[i];
    typename LinkedList<Pair<std::string, ValueType>>::Iterator temp_node =
        origin_list.Begin();
    while (temp_node != origin_list.End()) {
      auto hash = static_cast<std::size_t>(Hash(*temp_node.first));
      LinkedList<Pair<std::string, ValueType>>& insert_list = pair_list_[hash];
      insert_list.Insert(insert_list.End(), *temp_node);
      temp_node++;
    }
  }

  // Release the memory of the original linked list.
  delete[] temp;

  capacity_ = new_capacity;
  return 0;
}

}  // namespace Aq