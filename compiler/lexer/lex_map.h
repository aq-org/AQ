// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_LEX_MAP_H_
#define AQ_COMPILER_LEXER_LEX_MAP_H_

#include <cstddef>

namespace Aq {
namespace Compiler {
// A hash table for the lexer. Used to find special definitions such as compiler
// keywords.
template <typename T>
class LexMap {
 public:
  // Construct a LexMap class, and the default hash table memory size is the
  // upper limit of the 32-bit unsigned int type. Do not modify it unless
  // necessary.
  LexMap(std::size_t init_capacity = 4294967295);
  ~LexMap();

  // Insert a new pair into the hash table.
  void Insert(char* key, T value);

  // Find the value of a key.
  T* Find(char* key);

 private:
  struct Pair {
    char* key;
    T* value;
  };
  // A linked list of Pair type, used to resolve hash conflicts.
  class PairList {
   public:
    // Construct a PairList class.
    PairList() : head_ptr_(nullptr){};
    ~PairList();

    // Prepend a new pair to the list.
    void Prepend(Pair value);

    // Append a new pair to the list. It is not recommended to use it when
    // dealing with large amounts of data.
    void Append(Pair value);

    // Find the value of a key.
    T* Find(char* key);

   private:
    // A node type of the linked list.
    struct Node {
      Pair* data;
      Node* next;
      Node(Pair* pair) : data(pair), next(nullptr){};
    };

    // The head pointer of the linked list.
    Node* head_ptr_;
  };

  // The memory size of the hash table.
  std::size_t capacity_;

  // The data collection of the hash table is stored in a linked list of type
  // PairList.
  PairList* pair_list_;

  // The hash function. Based on DJB2 hashing algorithm.
  static unsigned int Hash(char* key);

  // Re-allocate the memory of the hash table.
  int Resize();
};
}  // namespace Compiler
}  // namespace Aq

#endif