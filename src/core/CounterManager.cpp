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

Counter* CounterManager::findBestCounterForOrder(const Order& order,
                                                 const ml::IPrepTimePredictor* predictor,
                                                 double timeOfDayHour,
                                                 double* outPredictedPrepTime) {
    if (counters.isEmpty()) {
        return nullptr;
    }

    Counter* bestCounter = nullptr;
    double minProjectedCompletion = std::numeric_limits<double>::max();
    double bestPredictedPrep = static_cast<double>(order.getTotalPreparationTimeMinutes());

    // Traversal across fixed array
    for (size_t i = 0; i < counters.size(); ++i) {
        Counter& candidate = counters[i];

        // Only allocate to available counters
        if (!candidate.getIsAvailable()) {
            continue;
        }

        double candidateWorkload = static_cast<double>(candidate.getCurrentWorkloadMinutes());
        double candidatePrepTime = static_cast<double>(order.getTotalPreparationTimeMinutes());

        // Use ML predictor if available and loaded
        if (predictor != nullptr && predictor->isLoaded()) {
            try {
                auto feats = order.extractPredictionFeatures(timeOfDayHour, candidateWorkload);
                candidatePrepTime = predictor->predict(feats);
                // Validate prediction (must be finite and positive)
                if (std::isnan(candidatePrepTime) || std::isinf(candidatePrepTime) || candidatePrepTime <= 0.0) {
                    candidatePrepTime = static_cast<double>(order.getTotalPreparationTimeMinutes());
                }
            } catch (...) {
                // Safe fallback to baseline sum
                candidatePrepTime = static_cast<double>(order.getTotalPreparationTimeMinutes());
            }
        }

        double projectedCompletion = candidateWorkload + candidatePrepTime;

        if (bestCounter == nullptr) {
            bestCounter = &candidate;
            minProjectedCompletion = projectedCompletion;
            bestPredictedPrep = candidatePrepTime;
        } else if (projectedCompletion < minProjectedCompletion) {
            bestCounter = &candidate;
            minProjectedCompletion = projectedCompletion;
            bestPredictedPrep = candidatePrepTime;
        } else if (std::abs(projectedCompletion - minProjectedCompletion) < 1e-6) {
            // Tie-break consistently using Counter ID
            if (candidate.getId() < bestCounter->getId()) {
                bestCounter = &candidate;
                minProjectedCompletion = projectedCompletion;
                bestPredictedPrep = candidatePrepTime;
            }
        }
    }

    if (outPredictedPrepTime != nullptr) {
        *outPredictedPrepTime = bestPredictedPrep;
    }
    return bestCounter;
}

bool CounterManager::enqueueOrder(Order order, int preferredCounterId, int* assignedCounterIdOut,
                                  const ml::IPrepTimePredictor* predictor, double timeOfDayHour) {
    Counter* targetCounter = nullptr;
    double predictedPrepTime = static_cast<double>(order.getTotalPreparationTimeMinutes());

    if (preferredCounterId != -1) {
        targetCounter = getCounterById(preferredCounterId);
        if (targetCounter != nullptr && !targetCounter->getIsAvailable()) {
            targetCounter = nullptr; // Target is offline, fall back to optimal
        }
        if (targetCounter != nullptr && predictor != nullptr && predictor->isLoaded()) {
            try {
                auto feats = order.extractPredictionFeatures(timeOfDayHour, targetCounter->getCurrentWorkloadMinutes());
                predictedPrepTime = predictor->predict(feats);
            } catch (...) {
                predictedPrepTime = static_cast<double>(order.getTotalPreparationTimeMinutes());
            }
        }
    }

    if (targetCounter == nullptr) {
        targetCounter = findBestCounterForOrder(order, predictor, timeOfDayHour, &predictedPrepTime);
    }

    if (targetCounter == nullptr) {
        return false; // No available counter in the system
    }

    if (assignedCounterIdOut != nullptr) {
        *assignedCounterIdOut = targetCounter->getId();
    }

    // Update order with the validated ML prediction
    int roundedPrepMins = std::max(1, static_cast<int>(std::round(predictedPrepTime)));
    order.setTotalPreparationTimeMinutes(roundedPrepMins);

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
