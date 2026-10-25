#include "canteen/core/OrderManager.hpp"
#include "canteen/algorithms/Searching.hpp"
#include "canteen/algorithms/Sorting.hpp"
#include <iostream>
#include <iomanip>

namespace canteen {

void OrderManager::recordOrderPlaced(const Order& order) {
    // Add to linked list history
    orderHistory.insertBack(order);
}

void OrderManager::recordOrderCompleted(const Order& order) {
    // Update or append status in history
    Order* existing = orderHistory.find([&order](const Order& o) {
        return o.getId() == order.getId();
    });

    if (existing != nullptr) {
        *existing = order;
    } else {
        orderHistory.insertBack(order);
    }
}

void OrderManager::recordOrderCancelled(const Order& order) {
    Order* existing = orderHistory.find([&order](const Order& o) {
        return o.getId() == order.getId();
    });

    if (existing != nullptr) {
        *existing = order;
    } else {
        orderHistory.insertBack(order);
    }
}

bool OrderManager::findOrderById(int orderId, Order& orderOut, const CounterManager& counterManager) const {
    // 1. First, search through active counter queues
    const auto& counters = counterManager.getCountersArray();
    for (size_t i = 0; i < counters.size(); ++i) {
        const Order* foundInQueue = counters[i].getPendingQueue().find([orderId](const Order& o) {
            return o.getId() == orderId;
        });
        if (foundInQueue != nullptr) {
            orderOut = *foundInQueue;
            return true;
        }
    }

    // 2. Next, search linked list order history
    const Order* foundInHistory = orderHistory.find([orderId](const Order& o) {
        return o.getId() == orderId;
    });

    if (foundInHistory != nullptr) {
        orderOut = *foundInHistory;
        return true;
    }

    return false;
}

void OrderManager::displayOrderHistory() const {
    std::cout << "\n============================== ORDER HISTORY ==============================\n";
    if (orderHistory.isEmpty()) {
        std::cout << "No orders recorded yet.\n";
        std::cout << "===========================================================================\n";
        return;
    }

    std::cout << std::left << std::setw(8) << "ID"
              << std::setw(20) << "Student Name"
              << std::setw(12) << "Student ID"
              << std::setw(14) << "Status"
              << std::setw(10) << "Counter"
              << std::setw(12) << "Prep Time"
              << "Total (Rs)\n";
    std::cout << "---------------------------------------------------------------------------\n";

    orderHistory.traverse([](const Order& o) {
        std::cout << "#" << std::left << std::setw(7) << o.getId()
                  << std::setw(20) << o.getStudentName()
                  << std::setw(12) << o.getStudentId()
                  << std::setw(14) << orderStatusToString(o.getStatus())
                  << std::setw(10) << (o.getAssignedCounterId() == -1 ? "N/A" : std::to_string(o.getAssignedCounterId()))
                  << std::setw(2) << o.getTotalPreparationTimeMinutes() << " mins    "
                  << std::fixed << std::setprecision(2) << o.getTotalAmount() << "\n";
    });
    std::cout << "===========================================================================\n";
}

std::vector<Order> OrderManager::getHistorySortedByPrepTime() const {
    std::vector<Order> vec;
    orderHistory.traverse([&vec](const Order& o) {
        vec.push_back(o);
    });
    if (!vec.empty()) {
        sorting::mergeSortOrdersByPrepTime(vec.data(), vec.size());
    }
    return vec;
}

std::vector<Order> OrderManager::getHistorySortedById() const {
    std::vector<Order> vec;
    orderHistory.traverse([&vec](const Order& o) {
        vec.push_back(o);
    });
    if (!vec.empty()) {
        sorting::mergeSortOrdersById(vec.data(), vec.size());
    }
    return vec;
}

} // namespace canteen
