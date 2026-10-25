#include "canteen/models/Order.hpp"
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace canteen {

Order::Order(int id, std::string studentId, std::string studentName, long long timestamp)
    : id(id), studentId(std::move(studentId)), studentName(std::move(studentName)),
      status(OrderStatus::QUEUED), assignedCounterId(-1), orderTimestamp(timestamp) {}

void Order::recalculateTotals() {
    totalPreparationTimeMinutes = 0;
    totalAmount = 0.0;
    for (const auto& line : items) {
        // Preparation time is determined by cooking items, with a base overhead
        totalPreparationTimeMinutes += line.item.getPreparationTimeMinutes() * line.quantity;
        totalAmount += line.item.getPrice() * line.quantity;
    }
}

void Order::addItem(const FoodItem& item, int quantity) {
    if (quantity <= 0) {
        throw std::invalid_argument("Order item quantity must be positive.");
    }
    items.emplace_back(item, quantity);
    recalculateTotals();
}

bool Order::transitionTo(OrderStatus newStatus) {
    if (!canTransition(status, newStatus)) {
        throw std::invalid_argument("Invalid status transition from " +
                                    orderStatusToString(status) + " to " +
                                    orderStatusToString(newStatus));
    }
    status = newStatus;
    return true;
}

std::string Order::toString() const {
    std::ostringstream oss;
    oss << "[Order #" << id << "] " << studentName << " (" << studentId << ")"
        << " | Items: " << items.size()
        << " | Total: Rs." << std::fixed << std::setprecision(2) << totalAmount
        << " | PrepTime: " << totalPreparationTimeMinutes << "m"
        << " | Status: " << orderStatusToString(status)
        << " | Counter: " << (assignedCounterId == -1 ? "None" : std::to_string(assignedCounterId));
    return oss.str();
}

void Order::display() const {
    std::cout << toString() << "\n";
    for (const auto& line : items) {
        std::cout << "    - " << line.item.getName() << " x" << line.quantity
                  << " (@ Rs." << std::fixed << std::setprecision(2) << line.item.getPrice() << " each)\n";
    }
}

} // namespace canteen
