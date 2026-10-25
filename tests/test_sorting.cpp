#include "test_framework.hpp"
#include "canteen/algorithms/Sorting.hpp"
#include "canteen/models/Counter.hpp"
#include "canteen/models/FoodItem.hpp"
#include "canteen/models/Order.hpp"
#include <vector>

using namespace canteen;

TEST_CASE(Sorting_BubbleSortCountersByWaitTime) {
    std::vector<Counter> counters;
    counters.emplace_back(1, "Counter 1", "Type A"); // wait 0
    counters.emplace_back(2, "Counter 2", "Type B"); // wait 0
    counters.emplace_back(3, "Counter 3", "Type C"); // wait 0

    // Enqueue orders to establish workloads
    Order oA(1, "S1", "N1");
    oA.addItem(FoodItem(1, "Item1", "Cat", 10.0, 15)); // 15 mins
    counters[0].enqueueOrder(oA); // Counter 1 -> 15 mins

    Order oB(2, "S2", "N2");
    oB.addItem(FoodItem(2, "Item2", "Cat", 10.0, 5)); // 5 mins
    counters[1].enqueueOrder(oB); // Counter 2 -> 5 mins

    Order oC(3, "S3", "N3");
    oC.addItem(FoodItem(3, "Item3", "Cat", 10.0, 10)); // 10 mins
    counters[2].enqueueOrder(oC); // Counter 3 -> 10 mins

    // Before sort: Counter 1 (15m), Counter 2 (5m), Counter 3 (10m)
    sorting::bubbleSortCountersByWaitTime(counters.data(), counters.size(), true);

    // After sort ascending: Counter 2 (5m), Counter 3 (10m), Counter 1 (15m)
    ASSERT_EQ(counters[0].getId(), 2);
    ASSERT_EQ(counters[0].getEstimatedWaitTime(), 5);

    ASSERT_EQ(counters[1].getId(), 3);
    ASSERT_EQ(counters[1].getEstimatedWaitTime(), 10);

    ASSERT_EQ(counters[2].getId(), 1);
    ASSERT_EQ(counters[2].getEstimatedWaitTime(), 15);
}

TEST_CASE(Sorting_BubbleSortTieBreakingOnId) {
    std::vector<Counter> counters;
    // Both counters have 0 wait time
    counters.emplace_back(5, "Counter 5", "Type A");
    counters.emplace_back(2, "Counter 2", "Type B");
    counters.emplace_back(4, "Counter 4", "Type C");

    sorting::bubbleSortCountersByWaitTime(counters.data(), counters.size(), true);

    // Tie breaker on ID ascending: 2, 4, 5
    ASSERT_EQ(counters[0].getId(), 2);
    ASSERT_EQ(counters[1].getId(), 4);
    ASSERT_EQ(counters[2].getId(), 5);
}

TEST_CASE(Sorting_MergeSortEmptyAndSingleElement) {
    std::vector<FoodItem> emptyArr;
    ASSERT_NO_THROW(sorting::mergeSortFoodItemsById(emptyArr.data(), emptyArr.size()));

    std::vector<FoodItem> singleItem;
    singleItem.emplace_back(42, "Item42", "Cat", 50.0, 5);
    sorting::mergeSortFoodItemsById(singleItem.data(), singleItem.size());
    ASSERT_EQ(singleItem.size(), 1);
    ASSERT_EQ(singleItem[0].getId(), 42);
}

TEST_CASE(Sorting_MergeSortFoodItemsByPrepTimeAndDuplicates) {
    std::vector<FoodItem> items;
    items.emplace_back(1, "Fast Food A", "Cat", 50.0, 10);
    items.emplace_back(2, "Fast Food B", "Cat", 50.0, 3);
    items.emplace_back(3, "Fast Food C", "Cat", 50.0, 7);
    items.emplace_back(4, "Fast Food D", "Cat", 50.0, 3); // Duplicate prep time 3
    items.emplace_back(5, "Fast Food E", "Cat", 50.0, 1);

    sorting::mergeSortFoodItemsByPrepTime(items.data(), items.size());

    ASSERT_EQ(items[0].getPreparationTimeMinutes(), 1);
    ASSERT_EQ(items[1].getPreparationTimeMinutes(), 3);
    ASSERT_EQ(items[2].getPreparationTimeMinutes(), 3);
    ASSERT_EQ(items[3].getPreparationTimeMinutes(), 7);
    ASSERT_EQ(items[4].getPreparationTimeMinutes(), 10);
}

TEST_CASE(Sorting_MergeSortOrdersByPrepTime) {
    std::vector<Order> orders;
    Order o1(101, "S1", "Alice");
    o1.addItem(FoodItem(1, "Item1", "Cat", 10.0, 20));

    Order o2(102, "S2", "Bob");
    o2.addItem(FoodItem(2, "Item2", "Cat", 10.0, 5));

    Order o3(103, "S3", "Charlie");
    o3.addItem(FoodItem(3, "Item3", "Cat", 10.0, 12));

    orders.push_back(o1);
    orders.push_back(o2);
    orders.push_back(o3);

    sorting::mergeSortOrdersByPrepTime(orders.data(), orders.size());

    ASSERT_EQ(orders[0].getId(), 102); // 5 mins
    ASSERT_EQ(orders[1].getId(), 103); // 12 mins
    ASSERT_EQ(orders[2].getId(), 101); // 20 mins
}
