// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/hash_table/hash_table.h"

#include <cstddef>

#include "compiler/compiler.h"
#include "compiler/dyn_array/dyn_array.h"
#include "debugger/debugger.h"

namespace Aq {
template <typename ValueType>
Compiler::HashTable<ValueType>::HashTable(std::size_t init_capacity) {
  pair_list_ = new DynArray<ValueType>(init_capacity);
  if (!pair_list_) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::Lexer::HashTable::HashTable",
                   "HashTable_MemoryError", "Memory allocation failed.", nullptr);
    return;
  }
  capacity_ = init_capacity;
}
template <typename ValueType>
Compiler::HashTable<ValueType>::~HashTable() {
  delete pair_list_;
}

template <typename ValueType>
void Compiler::HashTable<ValueType>::Insert(std::string key, ValueType value) {
  auto hash = static_cast<std::size_t>(Hash(key));
  ++size_;
  if (size_ / capacity_ > 0.8) {
    Resize();
  }
  LinkedList<Pair<std::string, std::string>> insert_list = pair_list_[hash];
  insert_list.Insert(insert_list.End(), {key, value});
}

template <typename ValueType>
bool Compiler::HashTable<ValueType>::Find(std::string key, ValueType& value) {
  auto hash = static_cast<std::size_t>(Hash(key));
  LinkedList<Pair<std::string, std::string>> find_list = pair_list_[hash];
  typename LinkedList<Pair<std::string, ValueType>>::Iterator temp_node =
      find_list.Begin();
  while (temp_node != find_list.End()) {
    if (key == *temp_node.first) {
      value = *temp_node.second;
      return true;
    }
    ++temp_node;
  }

  return false;
}

template <typename ValueType>
unsigned int Compiler::HashTable<ValueType>::Hash(std::string key) const {
  unsigned int hash = 5381;
  for (char character : key) {
    hash = ((hash << 5) + hash) + static_cast<unsigned int>(character);
  }
  hash = hash % capacity_;
  return hash;
};

template <typename ValueType>
int Compiler::HashTable<ValueType>::Resize() {
  DynArray<LinkedList<Pair<std::string, std::string>>>* temp = pair_list_;
  std::size_t new_capacity = capacity_ * 1.5;
  pair_list_ =
      new DynArray<LinkedList<Pair<std::string, std::string>>>[new_capacity];
  if (!pair_list_) {
    Debugger error(Debugger::Level::ERROR,
                   "Aq::Compiler::Lexer::HashTable::Resize", "Resize_MemoryError",
                   "Memory allocation failed.", nullptr);
    return -1;
  }
  for (std::size_t i = 0; i < capacity_; ++i) {
    LinkedList<Pair<std::string, ValueType>>& origin_list = temp[i];
    typename LinkedList<Pair<std::string, ValueType>>::Iterator temp_node =
        origin_list.Begin();
    while (temp_node != origin_list.End()) {
      auto hash = static_cast<std::size_t>(Hash(*temp_node.first));
      LinkedList<Pair<std::string, ValueType>>& insert_list = pair_list_[hash];
      insert_list.Insert(insert_list.End(), *temp_node);
      ++temp_node;
    }
  }
  delete[] temp;
  capacity_ = new_capacity;
  return 0;
}

}  // namespace Aq