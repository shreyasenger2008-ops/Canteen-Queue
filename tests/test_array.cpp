#include "test_framework.hpp"
#include "canteen/ds/FixedArray.hpp"
#include "canteen/models/Counter.hpp"

using namespace canteen;

TEST_CASE(Array_ValidIndexingAndTraversal) {
    FixedArray<int, 5> arr;
    ASSERT_TRUE(arr.isEmpty());
    ASSERT_EQ(arr.size(), 0);
    ASSERT_EQ(arr.capacity(), 5);

    arr.add(10);
    arr.add(20);
    arr.add(30);

    ASSERT_EQ(arr.size(), 3);
    ASSERT_EQ(arr.at(0), 10);
    ASSERT_EQ(arr.at(1), 20);
    ASSERT_EQ(arr.at(2), 30);
    ASSERT_EQ(arr[1], 20);

    // Update
    arr.update(1, 99);
    ASSERT_EQ(arr.at(1), 99);

    // Traversal sum
    int sum = 0;
    arr.traverse([&sum](int val, size_t /*idx*/) {
        sum += val;
    });
    ASSERT_EQ(sum, 10 + 99 + 30);
}

TEST_CASE(Array_OutOfBoundsProtection) {
    FixedArray<int, 3> arr;
    arr.add(100);
    arr.add(200);

    // Valid indices are 0 and 1
    ASSERT_NO_THROW(arr.at(0));
    ASSERT_NO_THROW(arr.at(1));

    // Out of bounds accesses must throw std::out_of_range
    ASSERT_THROWS(arr.at(2), std::out_of_range);
    ASSERT_THROWS(arr.at(3), std::out_of_range);
    ASSERT_THROWS(arr.at(999), std::out_of_range);

    // Update out of bounds
    ASSERT_THROWS(arr.update(2, 50), std::out_of_range);
}

TEST_CASE(Array_CapacityOverflow) {
    FixedArray<std::string, 2> arr;
    arr.add("CounterA");
    arr.add("CounterB");
    ASSERT_TRUE(arr.isFull());

    // Adding beyond capacity throws std::overflow_error
    ASSERT_THROWS(arr.add("CounterC"), std::overflow_error);
}

TEST_CASE(Array_CounterRepresentationAndAvailability) {
    FixedArray<Counter, 4> counters;
    counters.add(Counter(1, "Fast Food", "Snacks", true));
    counters.add(Counter(2, "Beverages", "Drinks", false)); // Offline
    counters.add(Counter(3, "Meals", "Lunch", true));

    ASSERT_EQ(counters.size(), 3);
    ASSERT_TRUE(counters.at(0).getIsAvailable());
    ASSERT_FALSE(counters.at(1).getIsAvailable());
    ASSERT_TRUE(counters.at(2).getIsAvailable());

    // Toggle availability
    counters.at(1).setIsAvailable(true);
    ASSERT_TRUE(counters.at(1).getIsAvailable());
}

TEST_CASE(Array_RemoveAtAndShift) {
    FixedArray<int, 5> arr;
    arr.add(1);
    arr.add(2);
    arr.add(3);
    arr.add(4);

    arr.removeAt(1); // Remove '2'
    ASSERT_EQ(arr.size(), 3);
    ASSERT_EQ(arr.at(0), 1);
    ASSERT_EQ(arr.at(1), 3);
    ASSERT_EQ(arr.at(2), 4);
}
