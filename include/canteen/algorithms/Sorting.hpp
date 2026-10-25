#ifndef CANTEEN_SORTING_HPP
#define CANTEEN_SORTING_HPP

#include "canteen/models/Counter.hpp"
#include "canteen/models/FoodItem.hpp"
#include "canteen/models/Order.hpp"
#include <cstddef>
#include <utility>
#include <functional>

namespace canteen {
namespace sorting {

// ============================================================================
// BUBBLE SORT ALGORITHMS (Time: O(n^2), Auxiliary Space: O(1) In-Place)
// ============================================================================

/**
 * @brief Manually implemented in-place Bubble Sort to sort Counters by estimated wait time.
 * Demonstrates a classic quadratic O(n^2) exchange-based sorting algorithm.
 * 
 * ALGORITHM & CHARACTERISTICS:
 * - Sorts in-place directly within the input array (Auxiliary Space: O(1)).
 * - Adjacent elements are compared and swapped if they are out of order.
 * - Optimized with early termination: if a complete pass occurs without any swaps,
 *   the algorithm terminates immediately (best-case O(n)).
 * - Equal keys maintain stable relative order (using strict inequality '>' for swaps).
 * - Empty arrays and single-element arrays return immediately without error.
 * 
 * TIME COMPLEXITY:
 *   - Best Case:    O(n) (when already sorted, 1 pass with 0 swaps)
 *   - Worst Case:   O(n^2) (when reverse sorted)
 *   - Average Case: O(n^2)
 * SPACE COMPLEXITY:
 *   - Auxiliary Space: O(1) (requires only a few scalar variables for indices and swap flag)
 * 
 * @param arr Array of Counters to sort in-place
 * @param size Number of elements
 * @param ascending If true, sorts lowest wait time first; if false, highest first
 */
inline void bubbleSortCountersByWaitTime(Counter* arr, size_t size, bool ascending = true) {
    if (arr == nullptr || size <= 1) {
        return; // Empty or single element is already sorted
    }

    for (size_t i = 0; i < size - 1; ++i) {
        bool swapped = false;

        for (size_t j = 0; j < size - i - 1; ++j) {
            int waitA = arr[j].getEstimatedWaitTime();
            int waitB = arr[j + 1].getEstimatedWaitTime();

            bool shouldSwap = false;
            if (ascending) {
                // If wait times equal, tie-break by Counter ID ascending
                if (waitA > waitB) {
                    shouldSwap = true;
                } else if (waitA == waitB && arr[j].getId() > arr[j + 1].getId()) {
                    shouldSwap = true;
                }
            } else {
                if (waitA < waitB) {
                    shouldSwap = true;
                } else if (waitA == waitB && arr[j].getId() < arr[j + 1].getId()) {
                    shouldSwap = true;
                }
            }

            if (shouldSwap) {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }

        // Early termination optimization
        if (!swapped) {
            break;
        }
    }
}

/**
 * @brief Generic Bubble Sort with custom comparator.
 * Sorts in-place with O(1) auxiliary memory.
 * 
 * Time Complexity: O(n^2) worst/average, O(n) best
 * Space Complexity: O(1)
 */
template <typename T, typename Comparator>
inline void bubbleSort(T* arr, size_t size, Comparator&& comp) {
    if (arr == nullptr || size <= 1) {
        return;
    }

    for (size_t i = 0; i < size - 1; ++i) {
        bool swapped = false;
        for (size_t j = 0; j < size - i - 1; ++j) {
            // comp(a, b) should return true if a should precede b
            if (comp(arr[j + 1], arr[j])) {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

// ============================================================================
// MERGE SORT ALGORITHMS (Time: O(n log n), Auxiliary Space: O(n))
// ============================================================================

/**
 * @brief Internal helper to merge two sorted contiguous subarrays:
 * arr[left ... mid] and arr[mid + 1 ... right].
 * 
 * Guarantees stability: if arr[leftIdx] is equivalent to arr[rightIdx],
 * arr[leftIdx] is placed first into the merged output.
 * 
 * Auxiliary Space: O(n) dynamically allocated buffer.
 */
template <typename T, typename Comparator>
void mergeSubarrays(T* arr, size_t left, size_t mid, size_t right, Comparator&& comp) {
    size_t leftSize = mid - left + 1;
    size_t rightSize = right - mid;

    // Allocate temporary arrays for merge
    T* leftArr = new T[leftSize];
    T* rightArr = new T[rightSize];

    for (size_t i = 0; i < leftSize; ++i) {
        leftArr[i] = std::move(arr[left + i]);
    }
    for (size_t j = 0; j < rightSize; ++j) {
        rightArr[j] = std::move(arr[mid + 1 + j]);
    }

    size_t i = 0;
    size_t j = 0;
    size_t k = left;

    while (i < leftSize && j < rightSize) {
        // Use !comp(rightArr[j], leftArr[i]) to ensure stability for duplicates
        if (!comp(rightArr[j], leftArr[i])) {
            arr[k++] = std::move(leftArr[i++]);
        } else {
            arr[k++] = std::move(rightArr[j++]);
        }
    }

    while (i < leftSize) {
        arr[k++] = std::move(leftArr[i++]);
    }
    while (j < rightSize) {
        arr[k++] = std::move(rightArr[j++]);
    }

    delete[] leftArr;
    delete[] rightArr;
}

/**
 * @brief Recursive helper function for Merge Sort.
 */
template <typename T, typename Comparator>
void mergeSortInternal(T* arr, size_t left, size_t right, Comparator&& comp) {
    if (left >= right) {
        return; // Base case: 0 or 1 element is inherently sorted
    }

    size_t mid = left + (right - left) / 2;
    mergeSortInternal(arr, left, mid, comp);
    mergeSortInternal(arr, mid + 1, right, comp);
    mergeSubarrays(arr, left, mid, right, comp);
}

/**
 * @brief Manually implemented Merge Sort algorithm.
 * Demonstrates divide-and-conquer strategy, achieving guaranteed O(n log n)
 * time complexity across all cases.
 * 
 * ALGORITHM & CHARACTERISTICS:
 * - Divide: Splits array into two halves at mid = left + (right - left) / 2.
 * - Conquer: Recursively sorts both halves.
 * - Combine: Merges the two sorted halves into a single sorted array.
 * - Stable: Preserves the relative order of duplicate elements.
 * - Space: Creates auxiliary storage during the merge step (Auxiliary Space: O(n)).
 * 
 * TIME COMPLEXITY:
 *   - Best Case:    O(n log n)
 *   - Worst Case:   O(n log n)
 *   - Average Case: O(n log n)
 * SPACE COMPLEXITY:
 *   - Auxiliary Space: O(n) (temporary buffers for merging subarrays)
 * 
 * @param arr Array to sort
 * @param size Number of elements
 * @param comp Comparator function returning true if first argument precedes second
 */
template <typename T, typename Comparator>
inline void mergeSort(T* arr, size_t size, Comparator&& comp) {
    if (arr == nullptr || size <= 1) {
        return;
    }
    mergeSortInternal(arr, 0, size - 1, std::forward<Comparator>(comp));
}

/**
 * @brief Sorts FoodItems by ID ascending using Merge Sort.
 * Useful as a prerequisite before applying Binary Search.
 */
inline void mergeSortFoodItemsById(FoodItem* arr, size_t size) {
    mergeSort(arr, size, [](const FoodItem& a, const FoodItem& b) {
        return a.getId() < b.getId();
    });
}

/**
 * @brief Sorts FoodItems by preparation time ascending.
 */
inline void mergeSortFoodItemsByPrepTime(FoodItem* arr, size_t size) {
    mergeSort(arr, size, [](const FoodItem& a, const FoodItem& b) {
        if (a.getPreparationTimeMinutes() != b.getPreparationTimeMinutes()) {
            return a.getPreparationTimeMinutes() < b.getPreparationTimeMinutes();
        }
        return a.getId() < b.getId();
    });
}

/**
 * @brief Sorts FoodItems by price ascending.
 */
inline void mergeSortFoodItemsByPrice(FoodItem* arr, size_t size) {
    mergeSort(arr, size, [](const FoodItem& a, const FoodItem& b) {
        if (a.getPrice() != b.getPrice()) {
            return a.getPrice() < b.getPrice();
        }
        return a.getId() < b.getId();
    });
}

/**
 * @brief Sorts Orders by preparation time ascending using Merge Sort.
 */
inline void mergeSortOrdersByPrepTime(Order* arr, size_t size) {
    mergeSort(arr, size, [](const Order& a, const Order& b) {
        if (a.getTotalPreparationTimeMinutes() != b.getTotalPreparationTimeMinutes()) {
            return a.getTotalPreparationTimeMinutes() < b.getTotalPreparationTimeMinutes();
        }
        return a.getId() < b.getId();
    });
}

/**
 * @brief Sorts Orders by Order ID ascending using Merge Sort.
 */
inline void mergeSortOrdersById(Order* arr, size_t size) {
    mergeSort(arr, size, [](const Order& a, const Order& b) {
        return a.getId() < b.getId();
    });
}

} // namespace sorting
} // namespace canteen

#endif // CANTEEN_SORTING_HPP
