#include "test_framework.hpp"
#include "canteen/ds/Queue.hpp"
#include "canteen/models/Order.hpp"

using namespace canteen;

TEST_CASE(Queue_FIFOOrdering) {
    Queue<int> q;
    ASSERT_TRUE(q.isEmpty());
    ASSERT_EQ(q.size(), 0);

    q.enqueue(101);
    q.enqueue(102);
    q.enqueue(103);

    ASSERT_EQ(q.size(), 3);
    ASSERT_EQ(q.front(), 101);
    ASSERT_EQ(q.rear(), 103);

    // Strictly FIFO order
    ASSERT_EQ(q.dequeue(), 101);
    ASSERT_EQ(q.dequeue(), 102);
    ASSERT_EQ(q.dequeue(), 103);

    ASSERT_TRUE(q.isEmpty());
    ASSERT_EQ(q.size(), 0);
}

TEST_CASE(Queue_EmptyQueueHandling) {
    Queue<int> q;
    ASSERT_THROWS(q.dequeue(), std::underflow_error);
    ASSERT_THROWS(q.front(), std::underflow_error);
    ASSERT_THROWS(q.rear(), std::underflow_error);
}

TEST_CASE(Queue_RepeatedEnqueueDequeueCycles) {
    Queue<int> q;
    for (int cycle = 0; cycle < 50; ++cycle) {
        q.enqueue(cycle);
        q.enqueue(cycle + 100);
        ASSERT_EQ(q.dequeue(), cycle);
        ASSERT_EQ(q.dequeue(), cycle + 100);
    }
    ASSERT_TRUE(q.isEmpty());
    ASSERT_EQ(q.size(), 0);
}

TEST_CASE(Queue_FrontAndRearUpdates) {
    Queue<std::string> q;
    q.enqueue("OrderA");
    ASSERT_EQ(q.front(), "OrderA");
    ASSERT_EQ(q.rear(), "OrderA");

    q.enqueue("OrderB");
    ASSERT_EQ(q.front(), "OrderA");
    ASSERT_EQ(q.rear(), "OrderB");

    q.enqueue("OrderC");
    ASSERT_EQ(q.front(), "OrderA");
    ASSERT_EQ(q.rear(), "OrderC");

    q.dequeue();
    ASSERT_EQ(q.front(), "OrderB");
    ASSERT_EQ(q.rear(), "OrderC");
}

TEST_CASE(Queue_RemoveIfOrderCancellation) {
    Queue<int> q;
    q.enqueue(1);
    q.enqueue(2);
    q.enqueue(3);
    q.enqueue(4);

    // Cancel order #2 in the middle
    bool removed = q.removeIf([](int val) { return val == 2; });
    ASSERT_TRUE(removed);
    ASSERT_EQ(q.size(), 3);

    // Verify remaining orders retain FIFO sequence: 1, 3, 4
    ASSERT_EQ(q.dequeue(), 1);
    ASSERT_EQ(q.dequeue(), 3);
    ASSERT_EQ(q.dequeue(), 4);
    ASSERT_TRUE(q.isEmpty());
}
