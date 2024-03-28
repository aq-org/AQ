// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LINKED_LIST_LINKED_LIST_H_
#define AQ_COMPILER_LINKED_LIST_LINKED_LIST_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "compiler/pair/pair.h"

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

  LinkedList& operator=(const LinkedList& other);

  struct Node {
    Pair<Node*, Node*> location;
    DataType data;
  };

  void Insert(Node* prev_node, DataType new_data);

  void Remove(Node* delete_node);

 private:
  Node* head_;
  Node* tail_;
};

}  // namespace Aq

#endif