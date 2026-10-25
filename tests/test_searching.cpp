#include "test_framework.hpp"
#include "canteen/algorithms/Searching.hpp"
#include "canteen/models/FoodItem.hpp"
#include "canteen/models/Order.hpp"
#include <vector>

using namespace canteen;

TEST_CASE(Searching_LinearSearchOrderById) {
    std::vector<Order> orders;
    orders.emplace_back(101, "S1", "Alice");
    orders.emplace_back(102, "S2", "Bob");
    orders.emplace_back(103, "S3", "Charlie");
    orders.emplace_back(104, "S4", "Diana");

    // First element
    ASSERT_EQ(searching::linearSearchOrderById(orders.data(), orders.size(), 101), 0);
    // Middle element
    ASSERT_EQ(searching::linearSearchOrderById(orders.data(), orders.size(), 103), 2);
    // Last element
    ASSERT_EQ(searching::linearSearchOrderById(orders.data(), orders.size(), 104), 3);
    // Missing element
    ASSERT_EQ(searching::linearSearchOrderById(orders.data(), orders.size(), 999), -1);
}

TEST_CASE(Searching_LinearSearchFoodItemByIdAndName) {
    std::vector<FoodItem> items;
    items.emplace_back(1, "Veg Burger", "Fast Food", 70.0, 5);
    items.emplace_back(2, "Paneer Wrap", "Fast Food", 90.0, 6);
    items.emplace_back(3, "Masala Chai", "Beverages", 20.0, 2);
    items.emplace_back(4, "Cold Coffee", "Beverages", 45.0, 3);

    // By ID
    ASSERT_EQ(searching::linearSearchFoodItemById(items.data(), items.size(), 1), 0);
    ASSERT_EQ(searching::linearSearchFoodItemById(items.data(), items.size(), 3), 2);
    ASSERT_EQ(searching::linearSearchFoodItemById(items.data(), items.size(), 4), 3);
    ASSERT_EQ(searching::linearSearchFoodItemById(items.data(), items.size(), 99), -1);

    // By Name (substring, case-insensitive)
    ASSERT_EQ(searching::linearSearchFoodItemByName(items.data(), items.size(), "chai"), 2);
    ASSERT_EQ(searching::linearSearchFoodItemByName(items.data(), items.size(), "WRAP"), 1);
    ASSERT_EQ(searching::linearSearchFoodItemByName(items.data(), items.size(), "Burger"), 0);
    ASSERT_EQ(searching::linearSearchFoodItemByName(items.data(), items.size(), "Pizza"), -1);
}

TEST_CASE(Searching_BinarySearchOnSortedData) {
    std::vector<FoodItem> sortedItems;
    sortedItems.emplace_back(10, "Item10", "Cat", 10.0, 1);
    sortedItems.emplace_back(20, "Item20", "Cat", 20.0, 2);
    sortedItems.emplace_back(30, "Item30", "Cat", 30.0, 3);
    sortedItems.emplace_back(40, "Item40", "Cat", 40.0, 4);
    sortedItems.emplace_back(50, "Item50", "Cat", 50.0, 5);

    // First element
    ASSERT_EQ(searching::binarySearchFoodItemById(sortedItems.data(), sortedItems.size(), 10), 0);
    // Middle element
    ASSERT_EQ(searching::binarySearchFoodItemById(sortedItems.data(), sortedItems.size(), 30), 2);
    // Last element
    ASSERT_EQ(searching::binarySearchFoodItemById(sortedItems.data(), sortedItems.size(), 50), 4);
    // Missing elements
    ASSERT_EQ(searching::binarySearchFoodItemById(sortedItems.data(), sortedItems.size(), 5), -1);
    ASSERT_EQ(searching::binarySearchFoodItemById(sortedItems.data(), sortedItems.size(), 25), -1);
    ASSERT_EQ(searching::binarySearchFoodItemById(sortedItems.data(), sortedItems.size(), 60), -1);
}

TEST_CASE(Searching_BinarySearchRequiresSortedValidation) {
    std::vector<FoodItem> unsortedItems;
    unsortedItems.emplace_back(30, "Item30", "Cat", 30.0, 3);
    unsortedItems.emplace_back(10, "Item10", "Cat", 10.0, 1); // Unsorted inversion
    unsortedItems.emplace_back(20, "Item20", "Cat", 20.0, 2);

    ASSERT_FALSE(searching::isSortedByFoodItemId(unsortedItems.data(), unsortedItems.size()));

    // Must throw std::invalid_argument because data is unsorted
    ASSERT_THROWS(
        searching::binarySearchFoodItemById(unsortedItems.data(), unsortedItems.size(), 10),
        std::invalid_argument
    );
}
