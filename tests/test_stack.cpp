#include "test_framework.hpp"
#include "canteen/ds/Stack.hpp"
#include "canteen/models/AdminAction.hpp"
#include "canteen/core/MenuManager.hpp"

using namespace canteen;

TEST_CASE(Stack_LIFOBehavior) {
    Stack<int> s;
    ASSERT_TRUE(s.isEmpty());
    ASSERT_EQ(s.size(), 0);

    s.push(10);
    s.push(20);
    s.push(30);

    ASSERT_EQ(s.size(), 3);
    ASSERT_EQ(s.peek(), 30);

    // Strictly LIFO order
    ASSERT_EQ(s.pop(), 30);
    ASSERT_EQ(s.peek(), 20);
    ASSERT_EQ(s.pop(), 20);
    ASSERT_EQ(s.peek(), 10);
    ASSERT_EQ(s.pop(), 10);

    ASSERT_TRUE(s.isEmpty());
    ASSERT_EQ(s.size(), 0);
}

TEST_CASE(Stack_EmptyStackHandling) {
    Stack<std::string> s;
    ASSERT_THROWS(s.pop(), std::underflow_error);
    ASSERT_THROWS(s.peek(), std::underflow_error);
}

TEST_CASE(Stack_DeepCopyAndOrderPreservation) {
    Stack<int> original;
    original.push(100);
    original.push(200);
    original.push(300);

    Stack<int> copy(original);
    ASSERT_EQ(copy.size(), 3);
    ASSERT_EQ(copy.pop(), 300);
    ASSERT_EQ(copy.pop(), 200);
    ASSERT_EQ(copy.pop(), 100);

    // Original must be untouched
    ASSERT_EQ(original.size(), 3);
    ASSERT_EQ(original.pop(), 300);
}

TEST_CASE(Stack_UndoRestoresPreviousMenuAvailability) {
    MenuManager menu;
    menu.addItem(FoodItem(1, "Veg Burger", "Fast Food", 50.0, 5, true));

    ASSERT_TRUE(menu.getItemById(1)->getIsAvailable());
    ASSERT_EQ(menu.getUndoStackSize(), 0);

    // Admin action: change to OUT OF STOCK
    menu.updateItemAvailability(1, false);
    ASSERT_FALSE(menu.getItemById(1)->getIsAvailable());
    ASSERT_EQ(menu.getUndoStackSize(), 1);

    // Undo via stack pop
    std::string undoMessage;
    bool ok = menu.undoLastAction(undoMessage);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(menu.getItemById(1)->getIsAvailable());
    ASSERT_EQ(menu.getUndoStackSize(), 0);

    // Second undo on empty stack fails gracefully
    bool secondUndo = menu.undoLastAction(undoMessage);
    ASSERT_FALSE(secondUndo);
}
