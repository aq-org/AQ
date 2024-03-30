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
    throw Debugger(Debugger::Level::ERROR,
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
  Pair pair;
  pair.key = key;
  pair.value = value;
  pair_list_[hash].Prepend(pair);
}

template <typename ValueType>
ValueType Compiler::HashMap<ValueType>::Find(std::string key) {
  unsigned int hash = Hash(key);
  return pair_list_[hash].Find(key);
}

template <typename ValueType>
Compiler::HashMap<ValueType>::PairList::PairList() = default;

template <typename ValueType>
Compiler::HashMap<ValueType>::PairList::~PairList() {
  while (head_ptr_ != nullptr) {
    Node* temp = head_ptr_;
    head_ptr_ = head_ptr_->next;
    delete temp;
  }
}

template <typename ValueType>
void Compiler::HashMap<ValueType>::PairList::Prepend(Pair value) {
  Node* new_node = new Node(value);
  new_node->next = head_ptr_;
  head_ptr_ = new_node;
}

template <typename ValueType>
void Compiler::HashMap<ValueType>::PairList::CopyDataToNewList(
    PairList* new_list, size_t new_capacity) {
  PairList::Node* temp_node = head_ptr_;
  while (temp_node != nullptr) {
    unsigned int hash = 5381;
    for (char character : temp_node->data.key) {
      // hash = hash * 33 + static_cast<unsigned int>(character)
      hash = ((hash << 5) + hash) + static_cast<unsigned int>(character);
    }
    hash = hash % new_capacity;
    new_list[hash].Append(temp_node->data);
    temp_node = temp_node->next;
  }
}

template <typename ValueType>
void Compiler::HashMap<ValueType>::PairList::Append(Pair value) {
  if (head_ptr_ == nullptr) {
    head_ptr_ = new Node(value);
  } else {
    // Find the last node and append the new node.
    Node* temp = head_ptr_;
    while (temp->next != nullptr) {
      temp = temp->next;
    }
    temp->next = new Node(value);
  }
}

template <typename ValueType>
ValueType Compiler::HashMap<ValueType>::PairList::Find(std::string key) {
  Node* temp = head_ptr_;

  // Compare keys one by one to find the corresponding value.
  while (temp != nullptr) {
    if (key == temp->data.key) {
      return temp->data.value;
    };
    temp = temp->next;
  }

  // Key not found, return nullptr.
  return static_cast<ValueType>(0);
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
    throw Debugger(Debugger::Level::ERROR,
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