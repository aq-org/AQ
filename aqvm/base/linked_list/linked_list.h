// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_LINKED_LIST_LINKED_LIST_H_
#define AQ_AQVM_BASE_LINKED_LIST_LINKED_LIST_H_

#include <stddef.h>

struct AqvmBaseLinkedList_LinkedList {
  struct AqvmBaseLinkedList_Node* head;
  struct AqvmBaseLinkedList_Node* tail;
  size_t capacity;
};

struct AqvmBaseLinkedList_Node {
  struct AqvmBaseLinkedList_Node* prev;
  struct AqvmBaseLinkedList_Node* next;
  void* data;
};

int AqvmBaseLinkedList_AddNode(struct AqvmBaseLinkedList_LinkedList* list,
                               void* data);

int AqvmBaseLinkedList_CloseLinkedList(
    struct AqvmBaseLinkedList_LinkedList* list);

int AqvmBaseLinkedList_DeleteNode(struct AqvmBaseLinkedList_LinkedList* list,
                                  size_t index);

void* AqvmBaseLinkedList_GetData(struct AqvmBaseLinkedList_LinkedList* list,
                                 size_t index);

int AqvmBaseLinkedList_InitializeLinkedList(
    struct AqvmBaseLinkedList_LinkedList* linked_list);

int AqvmBaseLinkedList_InsertNode(struct AqvmBaseLinkedList_LinkedList* list,
                                  size_t index, void* data);

int AqvmBaseLinkedList_PrependNode(struct AqvmBaseLinkedList_LinkedList* list,
                                   void* data);

int AqvmBaseLinkedList_SetData(struct AqvmBaseLinkedList_LinkedList* list,
                               size_t index, void* data);

#endif