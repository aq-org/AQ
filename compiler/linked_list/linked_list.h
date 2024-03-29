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

  struct Node {
    Pair<Node*, Node*> location;
    DataType data;
  };

  void Insert(Node* prev_node, DataType new_data);

  void Remove(Node* delete_node);

  Node* GetHead() const;

  Node* GetTail() const;

 private:
  Node* head_ = nullptr;
  Node* tail_ = nullptr;
};

}  // namespace Aq

#endif