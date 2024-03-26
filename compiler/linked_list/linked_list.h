// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LINKED_LIST_LINKED_LIST_H_
#define AQ_COMPILER_LINKED_LIST_LINKED_LIST_H_

#include "compiler/compiler.h"
#include "compiler/pair/pair.h"

namespace Aq{
template <typename ValueType>
class Compiler::LinkedList{
public:
struct Node{
Pair::Pair<Node*,Node*> data;
};

// Wait development.
};
}

#endif