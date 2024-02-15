// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_LEX_MAP_H_
#define AQ_COMPILER_LEXER_LEX_MAP_H_

#include <cstddef>
#include <limits>

#include "debugger/debugger.h"

namespace Aq {
namespace Compiler {
// A hash table for the lexer. Used to find special definitions such as compiler
// keywords.
template <typename T>
class LexMap {
 public:
  // Construct a LexMap class, and the default hash table memory size is the
  // upper limit of the std::size_t type. Do not modify it unless necessary.
  LexMap() {
    std::size_t init_capacity = 1024;
    pair_list_ = new PairList[init_capacity];
    if (!pair_list_) {
      Debugger error_info(
          Debugger::Level::ERROR, "Aq::Compiler::Lexer::LexMap::LexMap",
          "LexMap_MemoryError", "Memory allocation failed.", nullptr);
      pair_list_ = new PairList[1];
      capacity_ = 1;
    }
    capacity_ = init_capacity;
  };
  ~LexMap() {
    for (std::size_t i = 0; i < capacity_; ++i) {
      pair_list_[i].~PairList();
    }
    delete[] pair_list_;
  };

  // Insert a new pair into the hash table.
  void Insert(const char* key, T value) {
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
  };

  // Find the value of a key.
  T Find(const char* key) {
    unsigned int hash = Hash(key);
    if (hash > capacity_) {
      return static_cast<T>(0);
    }
    return pair_list_[hash].Find(key);
  };

 private:
  struct Pair {
    const char* key;
    T value;
  };
  // A linked list of Pair type, used to resolve hash conflicts.
  class PairList {
   public:
    // Construct a PairList class.
    PairList() : head_ptr_(nullptr){};
    ~PairList() {
      while (head_ptr_ != nullptr) {
        Node* temp = head_ptr_;
        head_ptr_ = head_ptr_->next;
        delete temp;
      }
    };

    // Prepend a new pair to the list.
    void Prepend(Pair value) {
      Node* new_node = new Node(value);
      new_node->next = head_ptr_;
      head_ptr_ = new_node;
    };

    // Append a new pair to the list. It is not recommended to use it when
    // dealing with large amounts of data.
    void Append(Pair value) {
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
    };

    // Find the value of a key.
    T Find(const char* key) {
      Node* temp = head_ptr_;

      // Compare keys one by one to find the corresponding value.
      while (temp != nullptr) {
        if (*key == *temp->data.key) {
          return temp->data.value;
        };
        temp = temp->next;
      }

      // Key not found, return nullptr.
      return static_cast<T>(0);
    };

   private:
    // A node type of the linked list.
    struct Node {
      Pair data;
      Node* next;
      Node(Pair pair) : data(pair), next(nullptr){};
    };

    // The head pointer of the linked list.
    Node* head_ptr_;
  };

  // The memory size of the hash table.
  std::size_t capacity_;

  // The number of elements in the hash table.
  std::size_t size_;

  // The data collection of the hash table is stored in a linked list of type
  // PairList.
  PairList* pair_list_;

  // The hash function. Based on DJB2 hashing algorithm.
  unsigned int Hash(const char* key) {
    unsigned int hash = 5381;
    while (*key) {
      // hash = hash * 33 + character
      hash = ((hash << 5) + hash) + (*key++);
    }
    hash = hash % capacity_;
    return hash;
  };

  // Re-allocate the memory of the hash table.
  int Resize() {
    PairList* temp = pair_list_;
    std::size_t new_capacity = capacity_ * 1.5;
    pair_list_ = new PairList[new_capacity];

    // Memory allocation failed.
    if (!pair_list_) {
      Debugger error_info(
          Debugger::Level::ERROR, "Aq::Compiler::Lexer::LexMap::Resize",
          "Resize_MemoryError", "Memory allocation failed.", nullptr);
      return -1;
    }

    // Initialize the new PairList objects with default constructors.
    for (int i = 0; i < new_capacity; i++) {
      new (pair_list_ + i) PairList();
    }

    // Copy the original linked list data to the new array.
    for (int i = 0; i < capacity_; i++) {
      pair_list_[i] = temp[i];
    }

    capacity_ = capacity_ * 1.5;
    return 0;
  };
};
}  // namespace Compiler
}  // namespace Aq

#endif