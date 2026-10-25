#include "canteen/core/CounterManager.hpp"
#include "canteen/algorithms/Sorting.hpp"
#include <iostream>
#include <iomanip>
#include <limits>

namespace canteen {

void CounterManager::addCounter(const Counter& counter) {
    counters.add(counter);
}

Counter& CounterManager::getCounterByIndex(size_t index) {
    // Uses FixedArray bounds checked at() method
    return counters.at(index);
}

const Counter& CounterManager::getCounterByIndex(size_t index) const {
    return counters.at(index);
}

Counter* CounterManager::getCounterById(int id) {
    for (size_t i = 0; i < counters.size(); ++i) {
        if (counters[i].getId() == id) {
            return &counters[i];
        }
    }
    return nullptr;
}

const Counter* CounterManager::getCounterById(int id) const {
    for (size_t i = 0; i < counters.size(); ++i) {
        if (counters[i].getId() == id) {
            return &counters[i];
        }
    }
    return nullptr;
}

Counter* CounterManager::findBestCounterForOrder(const Order& /*order*/) {
    if (counters.isEmpty()) {
        return nullptr;
    }

    Counter* bestCounter = nullptr;
    int minWorkload = std::numeric_limits<int>::max();

    // Traversal across fixed array
    for (size_t i = 0; i < counters.size(); ++i) {
        Counter& candidate = counters[i];

        // Only allocate to available counters
        if (!candidate.getIsAvailable()) {
            continue;
        }

        int candidateWorkload = candidate.getCurrentWorkloadMinutes();

        if (bestCounter == nullptr) {
            bestCounter = &candidate;
            minWorkload = candidateWorkload;
        } else if (candidateWorkload < minWorkload) {
            bestCounter = &candidate;
            minWorkload = candidateWorkload;
        } else if (candidateWorkload == minWorkload) {
            // Tie-break consistently using Counter ID
            if (candidate.getId() < bestCounter->getId()) {
                bestCounter = &candidate;
                minWorkload = candidateWorkload;
            }
        }
    }

    return bestCounter;
}

bool CounterManager::enqueueOrder(Order order, int preferredCounterId, int* assignedCounterIdOut) {
    Counter* targetCounter = nullptr;

    if (preferredCounterId != -1) {
        targetCounter = getCounterById(preferredCounterId);
        if (targetCounter != nullptr && !targetCounter->getIsAvailable()) {
            targetCounter = nullptr; // Target is offline, fall back to optimal
        }
    }

    if (targetCounter == nullptr) {
        targetCounter = findBestCounterForOrder(order);
    }

    if (targetCounter == nullptr) {
        return false; // No available counter in the system
    }

    if (assignedCounterIdOut != nullptr) {
        *assignedCounterIdOut = targetCounter->getId();
    }

    targetCounter->enqueueOrder(std::move(order));
    return true;
}

bool CounterManager::startPreparingFront(int counterId) {
    Counter* counter = getCounterById(counterId);
    if (counter == nullptr) {
        return false;
    }
    return counter->startPreparingFront();
}

Order CounterManager::completeFrontOrder(int counterId) {
    Counter* counter = getCounterById(counterId);
    if (counter == nullptr) {
        throw std::invalid_argument("Counter with ID " + std::to_string(counterId) + " not found.");
    }
    return counter->completeFrontOrder();
}

bool CounterManager::cancelOrder(int orderId, Order& cancelledOrderOut) {
    // Check all counters to find and cancel the order
    for (size_t i = 0; i < counters.size(); ++i) {
        if (counters[i].cancelOrder(orderId, cancelledOrderOut)) {
            return true;
        }
    }
    return false;
}

void CounterManager::sortCountersByWaitTime(bool ascending) {
    sorting::bubbleSortCountersByWaitTime(counters.data(), counters.size(), ascending);
}

void CounterManager::displayCounters() const {
    std::cout << "\n============================== CANTEEN COUNTERS ==============================\n";
    std::cout << std::left << std::setw(6) << "ID"
              << std::setw(24) << "Counter Name"
              << std::setw(18) << "Specialty"
              << std::setw(12) << "Status"
              << std::setw(10) << "Queue"
              << std::setw(12) << "Wait Time"
              << "Served\n";
    std::cout << "------------------------------------------------------------------------------\n";
    counters.traverse([](const Counter& c, size_t /*idx*/) {
        std::cout << "#" << std::left << std::setw(5) << c.getId()
                  << std::setw(24) << c.getName()
                  << std::setw(18) << c.getCounterType()
                  << std::setw(12) << (c.getIsAvailable() ? "ACTIVE" : "OFFLINE")
                  << std::setw(10) << c.getPendingOrderCount()
                  << std::setw(3) << c.getCurrentWorkloadMinutes() << " mins    "
                  << c.getTotalOrdersServed() << "\n";
    });
    std::cout << "==============================================================================\n";
}

void CounterManager::displayCounterQueue(int counterId) const {
    const Counter* counter = getCounterById(counterId);
    if (counter == nullptr) {
        std::cout << "Counter #" << counterId << " not found.\n";
        return;
    }
    counter->displayQueue();
}

} // namespace canteen
