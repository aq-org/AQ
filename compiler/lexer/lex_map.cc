// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/lexer/lex_map.h"

#include <cstddef>

#include "debug/debug.h"

namespace Aq {
namespace Compiler {
template <typename T>
LexMap<T>::LexMap(std::size_t init_capacity = 4294967295) {
  pair_list_ = new PairList[init_capacity];
  if (!pair_list_) {
    Debug error_info(Debug::Level::ERROR, "Aq::Compiler::Lexer::LexMap::LexMap",
                     "LexMap_MemoryError", "Memory allocation failed.",
                     nullptr);
    pair_list_ = new PairList[65535];
    if (!pair_list_) {
      Debug error_info(
          Debug::Level::ERROR, "Aq::Compiler::Lexer::LexMap::LexMap",
          "LexMap_MemoryError", "Memory allocation failed.", nullptr);
      capacity_ = 1;
    }
    capacity_ = 65535;
  }
  capacity_ = init_capacity;
}

template <typename T>
LexMap<T>::~LexMap() {
  delete[] pair_list_;
}

template <typename T>
LexMap<T>::PairList::~PairList() {
  while (head_ptr_ != nullptr) {
    Node* temp = head_ptr_;
    head_ptr_ = head_ptr_->next;
    delete temp;
  }
}

template <typename T>
unsigned int LexMap<T>::Hash(char* key) {
  unsigned int hash = 5381;
  while (*key) {
    // hash = hash * 33 + character
    hash = ((hash << 5) + hash) + (*key++);
  }
  return hash;
}

template <typename T>
void LexMap<T>::Insert(char* key, T value) {
  unsigned int hash = Hash(key);

  // Handle out-of-memory issues.
  while (hash > capacity_) {
    Debug error_info(
        Debug::Level::WARNING, "Aq::Compiler::Lexer::LexMap::Insert",
        "Insert_OutOfMemory",
        "The hash value size exceeds the expected memory size.", nullptr);

    if (Resize() == -1) {
      Debug error_info(
          Debug::Level::ERROR, "Aq::Compiler::Lexer::LexMap::Insert",
          "Insert_MemoryError", "Memory allocation failed.", nullptr);
      return -1;
    }
  }

  // Create key-value pairs and insert them into the linked list.
  Pair pair;
  pair.key = key;
  pair.value = value;
  pair_list_[hash].Append(pair);
}

template <typename T>
T* LexMap<T>::Find(char* key) {
  unsigned int hash = Hash(key);
  if (hash > capacity_) {
    return nullptr;
  }
  return pair_list_[hash].Find(key);
}

template <typename T>
int LexMap<T>::Resize() {
  PairList* temp = pair_list_;
  pair_list_ = new PairList[capacity_ * 1.5];
  
  // Memory allocation failed.
  if (!pair_list_) {
    Debug error_info(Debug::Level::ERROR, "Aq::Compiler::Lexer::LexMap::Resize",
                     "Resize_MemoryError", "Memory allocation failed.",
                     nullptr);
    return -1;
  }

  // Copy the original array data to the new array.
  for (int i = 0; i < capacity_; i++) {
    pair_list_[i] = temp[i];
  }

  delete[] temp;
  capacity_ = capacity_ * 1.5;
  return 0;
}

template <typename T>
void LexMap<T>::PairList::Prepend(Pair value) {
  Node* new_node = new Node(value);
  new_node->next = head_ptr_;
  head_ptr_ = new_node;
}

template <typename T>
void LexMap<T>::PairList::Append(Pair value) {
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

template <typename T>
T* LexMap<T>::PairList::Find(char* key) {
  Node* temp = head_ptr_;

  // Compare keys one by one to find the corresponding value.
  while (temp != nullptr) {
    if (*key == *temp->data->key) {
      return temp->data->value
    };
    temp = temp->next;
  }

  // Key not found, return nullptr.
  return nullptr;
}
}  // namespace Compiler
}  // namespace Aq
