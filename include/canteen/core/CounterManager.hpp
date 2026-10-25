#ifndef CANTEEN_COUNTER_MANAGER_HPP
#define CANTEEN_COUNTER_MANAGER_HPP

#include "canteen/ds/FixedArray.hpp"
#include "canteen/models/Counter.hpp"
#include <cstddef>
#include <string>

namespace canteen {

constexpr size_t MAX_CANTEEN_COUNTERS = 5;

/**
 * @brief Manages the canteen's fixed collection of service counters using
 * a fixed-size array data structure. Implements counter allocation heuristics
 * and sorting algorithms.
 */
class CounterManager {
private:
    FixedArray<Counter, MAX_CANTEEN_COUNTERS> counters;

public:
    CounterManager() = default;

    // Fixed array management
    void addCounter(const Counter& counter);
    size_t getCounterCount() const { return counters.size(); }
    constexpr size_t getMaxCapacity() const { return MAX_CANTEEN_COUNTERS; }

    // O(1) indexed access (bounds checked)
    Counter& getCounterByIndex(size_t index);
    const Counter& getCounterByIndex(size_t index) const;

    // Lookup by ID
    Counter* getCounterById(int id);
    const Counter* getCounterById(int id) const;

    // Baseline Counter Allocation Strategy (Smallest Workload, Tie-break on Counter ID)
    Counter* findBestCounterForOrder(const Order& order);

    // Queue actions on counters
    bool enqueueOrder(Order order, int preferredCounterId = -1, int* assignedCounterIdOut = nullptr);
    bool startPreparingFront(int counterId);
    Order completeFrontOrder(int counterId);
    bool cancelOrder(int orderId, Order& cancelledOrderOut);

    // Sorting algorithm (Bubble Sort O(n^2))
    void sortCountersByWaitTime(bool ascending = true);

    // Array traversal and display
    void displayCounters() const;
    void displayCounterQueue(int counterId) const;

    // Access to underlying array
    FixedArray<Counter, MAX_CANTEEN_COUNTERS>& getCountersArray() { return counters; }
    const FixedArray<Counter, MAX_CANTEEN_COUNTERS>& getCountersArray() const { return counters; }
};

} // namespace canteen

#endif // CANTEEN_COUNTER_MANAGER_HPP
