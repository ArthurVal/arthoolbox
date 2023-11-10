#pragma once

#include <assert.h>

/**
 *  \brief Double linked list pointing towards next/prev elements
 *
 *  This data structure is meant to be used inside any other data structure that
 *  should be part of a double linked list.
 *
 *  Example:
 *
 *  struct Toto {
 *    struct atb_DLinkedLists list;
 *  };
 *
 *  // This is the list HEAD, used to points to the begin/end of the list
 *  struct atb_DLinkedLists head = atb_DLinkedList_HEAD_INITIALIZER(head);
 *
 *  struct Toto first;
 *  atb_DLinkedList_Init(&(first.list));
 *
 *  struct Toto second;
 *  atb_DLinkedList_Init(&(second.list));
 *
 *  atb_DLinkedList_InsertAfter(&(head), &(first.list));
 *  atb_DLinkedList_InsertBefore(&(head), &(second.list));
 *
 *  // Here we have the following circular linked list:
 *  //             *--> [HEAD] ---*
 *  //             |              |
 *  //             |              v
 *  //      [second.list] <- [first.list]
 *
 *  const atb_DLinkedList *node = NULL;
 *  atb_DLinkedlist_ForEach(node, &head) {
 *    const Toto* toto_node = atb_DLinkedlist_Entry(node, struct Toto, list);
 *    // toto_node points to first then second.
 *  }
 *
 *  node = NULL;
 *  atb_DLinkedlist_ForEachReverse(node, &head) {
 *    const Toto* toto_node = atb_DLinkedlist_Entry(node, struct Toto, list);
 *    // toto_node points to second then first.
 *  }
 *
 */
struct atb_DLinkedList {
  struct atb_DLinkedList *next, *prev;
};

/**
 *  \brief Statically initialize an atb_DLinkedList (set prev & next to itself)
 */
#define atb_DLinkedList_HEAD_INITIALIZER(name)                                 \
  { &(name), &(name) }

/**
 *  \brief Initialize an atb_DLinkedList (set prev & next to itself)
 */
static inline void atb_DLinkedList_Init(struct atb_DLinkedList *list) {
  assert(list);

  list->next = list;
  list->prev = list;
}

/**
 *  \brief Connects second as the next node of first (and vice versa)
 *
 *  \warning This is not meant to be used directly as an INSERTION into
 *           a list
 */
static inline void atb_DLinkedList_Connect(struct atb_DLinkedList *first,
                                           struct atb_DLinkedList *second) {
  assert(first);
  assert(second);

  first->next = second;
  second->prev = first;
}

/**
 *  \brief Insert new_node AFTER node (node->next)
 */
static inline void
atb_DLinkedList_InsertAfter(struct atb_DLinkedList *node,
                            struct atb_DLinkedList *new_node) {
  assert(node);

  atb_DLinkedList_Connect(new_node, node->next);
  atb_DLinkedList_Connect(node, new_node);
}

/**
 *  \brief Insert new_node BEFORE node (node->prev)
 */
static inline void
atb_DLinkedList_InsertBefore(struct atb_DLinkedList *node,
                             struct atb_DLinkedList *new_node) {
  assert(node);

  atb_DLinkedList_Connect(node->prev, new_node);
  atb_DLinkedList_Connect(new_node, node);
}

/**
 *  \brief Remove node from the list
 */
static inline void atb_DLinkedList_Pop(struct atb_DLinkedList *node) {
  assert(node);

  atb_DLinkedList_Connect(node->prev, node->next);
  atb_DLinkedList_Init(node);
}

/**
 *  \brief Retreive the original container type, given its list ptr, type and
 *         the list member name
 *
 *  \param[in] ptr A atb_DLinkedlist ptr
 *  \param[in] type The parent struct type containing the list ptr
 *  \param[in] member The name of the atb_DLinkedlist inside the parent struct
 *
 *  \return type* The parent struct ptr containing ptr
 */
#define atb_DLinkedList_Entry(ptr, type, member)                               \
  ((type *)((char *)(ptr)-offsetof(type, member)))

/**
 *  \brief Forward iterate (using ->next) over the double linked list
 *
 *  \param[in] node A atb_DLinkedlist* variable used as iterator
 *  \param[in] list_head A atb_DLinkedlist* List head we wish to iterate over
 *
 *  \note This ForEach function iterates over all nodes EXCEPT the list_head
 */
#define atb_DLinkedList_ForEach(node, list_head)                               \
  for (node = (list_head)->next; node != (list_head); node = node->next)

/**
 *  \brief Backward iterate (using ->prev) over the double linked list
 *
 *  \param[in] node A atb_DLinkedlist* variable used as iterator
 *  \param[in] list_head A atb_DLinkedlist* List head we wish to iterate over
 *
 *  \note This ForEach function iterates over all nodes EXCEPT the list_head
 */
#define atb_DLinkedList_ForEachReverse(node, list_head)                        \
  for (node = (list_head)->prev; node != (list_head); node = node->prev)
