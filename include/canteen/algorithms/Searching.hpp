#ifndef CANTEEN_SEARCHING_HPP
#define CANTEEN_SEARCHING_HPP

#include "canteen/models/FoodItem.hpp"
#include "canteen/models/Order.hpp"
#include <cstddef>
#include <string>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace canteen {

/**
 * @brief Namespace containing manually implemented searching algorithms
 * for the canteen optimization system.
 */
namespace searching {

/**
 * @brief Helper function to convert a string to lowercase for case-insensitive matching.
 */
inline std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

// ============================================================================
// LINEAR SEARCH ALGORITHMS (Time: O(n), Space: O(1))
// ============================================================================

/**
 * @brief Linear Search for an order by order ID.
 * Examines elements sequentially from index 0 to size - 1.
 * 
 * Time Complexity:
 *   - Best Case: O(1) (target is at index 0)
 *   - Worst Case: O(n) (target is at the last index or absent)
 *   - Average Case: O(n) (target located near the middle)
 * Space Complexity:
 *   - Auxiliary Space: O(1) (in-place iterative traversal)
 * 
 * @param arr Array of Orders
 * @param size Number of elements in array
 * @param targetId Order ID to find
 * @return Index of matching order, or -1 if not found
 */
inline int linearSearchOrderById(const Order* arr, size_t size, int targetId) {
    if (arr == nullptr || size == 0) {
        return -1;
    }
    for (size_t i = 0; i < size; ++i) {
        if (arr[i].getId() == targetId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

/**
 * @brief Linear Search for a food item by item ID.
 * 
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 * 
 * @param arr Array of FoodItems
 * @param size Number of elements
 * @param targetId FoodItem ID to locate
 * @return Index if found, -1 otherwise
 */
inline int linearSearchFoodItemById(const FoodItem* arr, size_t size, int targetId) {
    if (arr == nullptr || size == 0) {
        return -1;
    }
    for (size_t i = 0; i < size; ++i) {
        if (arr[i].getId() == targetId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

/**
 * @brief Linear Search for a food item by name (case-insensitive).
 * Supports both exact matching and substring matching.
 * 
 * Time Complexity: O(n * m) where m is string length
 * Space Complexity: O(1) auxiliary
 * 
 * @param arr Array of FoodItems
 * @param size Number of elements
 * @param targetName Name to search for
 * @param exactMatch If true, matches entire name; if false, matches substring
 * @return Index of first matching item, or -1 if not found
 */
inline int linearSearchFoodItemByName(const FoodItem* arr, size_t size,
                                     const std::string& targetName,
                                     bool exactMatch = false) {
    if (arr == nullptr || size == 0 || targetName.empty()) {
        return -1;
    }
    std::string needle = toLower(targetName);
    for (size_t i = 0; i < size; ++i) {
        std::string haystack = toLower(arr[i].getName());
        if (exactMatch) {
            if (haystack == needle) {
                return static_cast<int>(i);
            }
        } else {
            if (haystack.find(needle) != std::string::npos) {
                return static_cast<int>(i);
            }
        }
    }
    return -1;
}

/**
 * @brief Generic Linear Search using a custom predicate.
 * 
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */
template <typename T, typename Predicate>
inline int linearSearch(const T* arr, size_t size, Predicate&& pred) {
    if (arr == nullptr || size == 0) {
        return -1;
    }
    for (size_t i = 0; i < size; ++i) {
        if (pred(arr[i])) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ============================================================================
// BINARY SEARCH ALGORITHMS (Time: O(log n), Space: O(1))
// ============================================================================

/**
 * @brief Validates whether an array of FoodItems is sorted in ascending order of ID.
 * Required before applying Binary Search.
 * 
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */
inline bool isSortedByFoodItemId(const FoodItem* arr, size_t size) {
    if (arr == nullptr || size <= 1) {
        return true;
    }
    for (size_t i = 1; i < size; ++i) {
        if (arr[i - 1].getId() > arr[i].getId()) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Binary Search for a FoodItem by ID in a sorted array.
 * 
 * WHY BINARY SEARCH REQUIRES SORTED DATA:
 * ---------------------------------------
 * Binary search relies on the monotonicity principle:
 * In a sorted sequence A[0 ... n-1], if target > A[mid], then for all k <= mid,
 * target > A[k]. Thus, the entire left half [low ... mid] can be eliminated
 * from consideration in O(1) operations.
 * Conversely, if target < A[mid], the entire right half [mid ... high] is eliminated.
 * 
 * If the input array is NOT sorted, this monotonicity invariant is violated;
 * elements smaller or larger than A[mid] could reside anywhere, leading to false
 * negatives where elements exist in the skipped half.
 * 
 * Time Complexity:
 *   - Best Case: O(1) (target is at the initial middle index)
 *   - Worst Case: O(log n) (repeatedly halving until interval size is 0)
 *   - Average Case: O(log n)
 * Space Complexity:
 *   - Auxiliary Space: O(1) (iterative halving with low, high, mid pointers)
 * 
 * @param arr Array of FoodItems (must be sorted ascending by ID)
 * @param size Number of elements
 * @param targetId FoodItem ID to find
 * @return Index of matching item, or -1 if not found
 * @throws std::invalid_argument if the array is not sorted by ID
 */
inline int binarySearchFoodItemById(const FoodItem* arr, size_t size, int targetId) {
    if (arr == nullptr || size == 0) {
        return -1;
    }

    // Explicit validation requirement: ensure input array is sorted
    if (!isSortedByFoodItemId(arr, size)) {
        throw std::invalid_argument(
            "Binary Search Precondition Failure: Array must be sorted in ascending order of ID.");
    }

    int low = 0;
    int high = static_cast<int>(size) - 1;

    while (low <= high) {
        // Prevent integer overflow compared to (low + high) / 2
        int mid = low + (high - low) / 2;

        if (arr[mid].getId() == targetId) {
            return mid; // Element found
        }
        if (arr[mid].getId() < targetId) {
            low = mid + 1; // Target must be in right half
        } else {
            high = mid - 1; // Target must be in left half
        }
    }

    return -1; // Element not present
}

/**
 * @brief Generic Binary Search on a sorted array with key extractor and comparator.
 * 
 * Time Complexity: O(log n)
 * Space Complexity: O(1)
 */
template <typename T, typename Key, typename KeyExtractor>
inline int binarySearchGeneric(const T* arr, size_t size, const Key& targetKey, KeyExtractor&& extractor) {
    if (arr == nullptr || size == 0) {
        return -1;
    }

    // Check sorted order
    for (size_t i = 1; i < size; ++i) {
        if (extractor(arr[i - 1]) > extractor(arr[i])) {
            throw std::invalid_argument("Binary Search Precondition Failure: Generic array is not sorted.");
        }
    }

    int low = 0;
    int high = static_cast<int>(size) - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        auto currentKey = extractor(arr[mid]);

        if (currentKey == targetKey) {
            return mid;
        }
        if (currentKey < targetKey) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return -1;
}

} // namespace searching
} // namespace canteen

#endif // CANTEEN_SEARCHING_HPP
