// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LINKED_LIST_LINKED_LIST_H_
#define AQ_COMPILER_LINKED_LIST_LINKED_LIST_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "compiler/pair/pair.h"
#include "debugger/debugger.h"

namespace Aq {

template <typename DataType>
class Compiler::LinkedList {
 public:
  LinkedList();
  ~LinkedList();

  LinkedList(const LinkedList&) = delete;
  LinkedList(LinkedList&&) noexcept = delete;
  LinkedList& operator=(const LinkedList&) = delete;
  LinkedList& operator=(LinkedList&&) noexcept = delete;

  /// The node type of the linked list.
  struct Node {
    Pair<Node*, Node*> location;
    DataType data;
  };

  /// The iterator type of the linked list.
  class Iterator {
   public:
    Iterator(Node* node);
    ~Iterator();

    Iterator(const Iterator&) = default;
    Iterator(Iterator&&) noexcept = default;
    Iterator& operator=(const Iterator&) = default;
    Iterator& operator=(Iterator&&) noexcept = default;

    DataType& operator*() const;
    Iterator& operator++();
    Iterator& operator--();
    Iterator operator++(int);
    Iterator operator--(int);
    Iterator& operator+=(std::size_t n);
    Iterator& operator-=(std::size_t n);
    Iterator operator+(std::size_t n) const;
    Iterator operator-(std::size_t n) const;
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    Node* node_ = nullptr;
  };

  /// Inserts |new_data| after |prev_node|. If |prev_node| is |nullptr|,
  /// |new_data| will be inserted at the beginning of the list.
  void Insert(Iterator prev_node, DataType new_data);

  /// Removes |delete_node| from the list.
  void Remove(Iterator* delete_node);

  /// Returns the beginning iterator of the linked list.
  Iterator Begin() const;

  /// Returns the end iterator of the linked list.
  Iterator End() const;

 private:
  Node* head_ = nullptr;
  Node* tail_ = nullptr;
};

}  // namespace Aq

#endif