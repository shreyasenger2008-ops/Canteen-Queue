#include "test_framework.hpp"
#include "canteen/ds/SinglyLinkedList.hpp"
#include <string>

using namespace canteen;

TEST_CASE(LinkedList_EmptyListHandling) {
    SinglyLinkedList<int> list;
    ASSERT_TRUE(list.isEmpty());
    ASSERT_EQ(list.size(), 0);

    ASSERT_THROWS(list.front(), std::underflow_error);
    ASSERT_THROWS(list.back(), std::underflow_error);
    ASSERT_THROWS(list.deleteFront(), std::underflow_error);

    // Deleting from empty list returns false
    bool deleted = list.deleteBy([](int val) { return val == 10; });
    ASSERT_FALSE(deleted);
}

TEST_CASE(LinkedList_SingleNodeOperations) {
    SinglyLinkedList<int> list;
    list.insertBack(42);

    ASSERT_FALSE(list.isEmpty());
    ASSERT_EQ(list.size(), 1);
    ASSERT_EQ(list.front(), 42);
    ASSERT_EQ(list.back(), 42);

    list.deleteFront();
    ASSERT_TRUE(list.isEmpty());
    ASSERT_EQ(list.size(), 0);
}

TEST_CASE(LinkedList_MultipleNodesInsertFrontAndBack) {
    SinglyLinkedList<int> list;
    list.insertBack(20);
    list.insertFront(10);
    list.insertBack(30);

    // Expected order: 10 -> 20 -> 30
    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(list.front(), 10);
    ASSERT_EQ(list.back(), 30);
}

TEST_CASE(LinkedList_DeleteFirstMiddleLastNode) {
    SinglyLinkedList<int> list;
    list.insertBack(10);
    list.insertBack(20);
    list.insertBack(30);
    list.insertBack(40);
    list.insertBack(50);

    // 1. Delete middle node (30)
    bool deletedMid = list.deleteBy([](int v) { return v == 30; });
    ASSERT_TRUE(deletedMid);
    ASSERT_EQ(list.size(), 4);
    ASSERT_TRUE(list.find([](int v) { return v == 30; }) == nullptr);

    // 2. Delete first node (10)
    bool deletedFirst = list.deleteBy([](int v) { return v == 10; });
    ASSERT_TRUE(deletedFirst);
    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(list.front(), 20);

    // 3. Delete last node (50)
    bool deletedLast = list.deleteBy([](int v) { return v == 50; });
    ASSERT_TRUE(deletedLast);
    ASSERT_EQ(list.size(), 2);
    ASSERT_EQ(list.back(), 40);

    // 4. Try deleting absent value
    bool deletedAbsent = list.deleteBy([](int v) { return v == 999; });
    ASSERT_FALSE(deletedAbsent);
    ASSERT_EQ(list.size(), 2);
}

TEST_CASE(LinkedList_SearchPresentAndAbsent) {
    SinglyLinkedList<std::string> list;
    list.insertBack("Apple");
    list.insertBack("Banana");
    list.insertBack("Cherry");

    std::string* found = list.find([](const std::string& s) { return s == "Banana"; });
    ASSERT_TRUE(found != nullptr);
    ASSERT_EQ(*found, "Banana");

    std::string* missing = list.find([](const std::string& s) { return s == "Mango"; });
    ASSERT_TRUE(missing == nullptr);
}

TEST_CASE(LinkedList_DeepCopyAndMoveRuleOfFive) {
    SinglyLinkedList<int> original;
    original.insertBack(1);
    original.insertBack(2);
    original.insertBack(3);

    // 1. Copy Constructor (Deep copy)
    SinglyLinkedList<int> copy(original);
    ASSERT_EQ(copy.size(), original.size());
    ASSERT_EQ(copy.front(), 1);
    ASSERT_EQ(copy.back(), 3);

    // Mutate copy; original must remain untouched
    copy.deleteFront();
    ASSERT_EQ(copy.size(), 2);
    ASSERT_EQ(original.size(), 3);

    // 2. Copy Assignment
    SinglyLinkedList<int> assigned;
    assigned = original;
    ASSERT_EQ(assigned.size(), 3);
    ASSERT_EQ(assigned.front(), 1);

    // 3. Move Constructor
    SinglyLinkedList<int> moved(std::move(copy));
    ASSERT_EQ(moved.size(), 2);
    ASSERT_TRUE(copy.isEmpty()); // NOLINT

    // 4. Move Assignment
    SinglyLinkedList<int> moveAssigned;
    moveAssigned = std::move(moved);
    ASSERT_EQ(moveAssigned.size(), 2);
    ASSERT_TRUE(moved.isEmpty()); // NOLINT
}
