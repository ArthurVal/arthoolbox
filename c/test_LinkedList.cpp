#include "gtest/gtest.h"
#include <iterator>

extern "C" {
#include "LinkedList.h"
}

namespace {
TEST(CoreLinkedList, StaticInit) {
  atb_DLinkedList list = atb_DLinkedList_HEAD_INITIALIZER(list);
  EXPECT_EQ(list.next, &list);
  EXPECT_EQ(list.prev, &list);
}

TEST(CoreLinkedList, Init) {
  atb_DLinkedList list;
  atb_DLinkedList_Init(&list);
  EXPECT_EQ(list.next, &list);
  EXPECT_EQ(list.prev, &list);
}

TEST(CoreLinkedList, InsertAfter) {
  atb_DLinkedList first = atb_DLinkedList_HEAD_INITIALIZER(first);
  atb_DLinkedList second = atb_DLinkedList_HEAD_INITIALIZER(second);
  atb_DLinkedList_InsertAfter(&(first), &(second));
  EXPECT_EQ(first.next, &(second));

  EXPECT_EQ(second.prev, &(first));
  EXPECT_EQ(second.next, &(first));

  EXPECT_EQ(first.prev, &(second));

  atb_DLinkedList third = atb_DLinkedList_HEAD_INITIALIZER(third);
  atb_DLinkedList_InsertAfter(&(second), &(third));
  EXPECT_EQ(first.next, &(second));

  EXPECT_EQ(second.prev, &(first));
  EXPECT_EQ(second.next, &(third));

  EXPECT_EQ(third.prev, &(second));
  EXPECT_EQ(third.next, &(first));

  EXPECT_EQ(first.prev, &(third));
}

TEST(CoreLinkedList, InsertBefore) {
  atb_DLinkedList first = atb_DLinkedList_HEAD_INITIALIZER(first);
  atb_DLinkedList second = atb_DLinkedList_HEAD_INITIALIZER(second);
  atb_DLinkedList_InsertBefore(&(first), &(second));
  EXPECT_EQ(first.next, &(second));

  EXPECT_EQ(second.prev, &(first));
  EXPECT_EQ(second.next, &(first));

  EXPECT_EQ(first.prev, &(second));

  atb_DLinkedList third = atb_DLinkedList_HEAD_INITIALIZER(third);
  atb_DLinkedList_InsertBefore(&(second), &(third));
  EXPECT_EQ(first.next, &(third));

  EXPECT_EQ(third.prev, &(first));
  EXPECT_EQ(third.next, &(second));

  EXPECT_EQ(second.prev, &(third));
  EXPECT_EQ(second.next, &(first));

  EXPECT_EQ(first.prev, &(second));
}

TEST(CoreLinkedList, Pop) {
  atb_DLinkedList first = atb_DLinkedList_HEAD_INITIALIZER(first);

  // Poping an empty list does nothing
  EXPECT_EQ(first.prev, &(first));
  EXPECT_EQ(first.next, &(first));
  atb_DLinkedList_Pop(&(first));
  EXPECT_EQ(first.prev, &(first));
  EXPECT_EQ(first.next, &(first));

  atb_DLinkedList second = atb_DLinkedList_HEAD_INITIALIZER(second);
  atb_DLinkedList_InsertAfter(&(first), &(second));

  atb_DLinkedList third = atb_DLinkedList_HEAD_INITIALIZER(third);
  atb_DLinkedList_InsertAfter(&(second), &(third));

  atb_DLinkedList_Pop(&(second));

  EXPECT_EQ(second.next, &(second));
  EXPECT_EQ(second.prev, &(second));

  EXPECT_EQ(first.next, &(third));

  EXPECT_EQ(third.prev, &(first));
  EXPECT_EQ(third.next, &(first));

  EXPECT_EQ(first.prev, &(third));
}

struct Toto {
  std::size_t useless_0;
  atb_DLinkedList list;
  std::size_t useless_1;
};

TEST(CoreLinkedList, Entry) {
  Toto toto = {
      0,
      atb_DLinkedList_HEAD_INITIALIZER(toto.list),
      1,
  };

  atb_DLinkedList *toto_list = &(toto.list);

  EXPECT_EQ(&toto, atb_DLinkedList_Entry(toto_list, struct Toto, list));
}

TEST(CoreLinkedList, ForEach) {
  atb_DLinkedList head = atb_DLinkedList_HEAD_INITIALIZER(head);

  Toto first{0, atb_DLinkedList_HEAD_INITIALIZER(first.list), 0};
  atb_DLinkedList_InsertAfter(&(head), &(first.list));

  Toto second{1, atb_DLinkedList_HEAD_INITIALIZER(second.list), 1};
  atb_DLinkedList_InsertAfter(&(first.list), &(second.list));

  Toto third{2, atb_DLinkedList_HEAD_INITIALIZER(third.list), 2};
  atb_DLinkedList_InsertAfter(&(second.list), &(third.list));

  const atb_DLinkedList *elem = nullptr;
  std::size_t expected_id = 0;

  atb_DLinkedList_ForEach(elem, &head) {
    EXPECT_EQ(atb_DLinkedList_Entry(elem, struct Toto, list)->useless_0,
              expected_id++);
  }

  expected_id = third.useless_0;
  atb_DLinkedList_ForEachReverse(elem, &head) {
    EXPECT_EQ(atb_DLinkedList_Entry(elem, struct Toto, list)->useless_0,
              expected_id--);
  }
}

} // namespace
