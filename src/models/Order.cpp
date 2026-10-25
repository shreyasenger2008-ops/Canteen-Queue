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

ml::OrderPredictionFeatures Order::extractPredictionFeatures(double timeOfDayHour, double counterWorkload) const {
    ml::OrderPredictionFeatures feats;
    feats.timeOfDayHour = timeOfDayHour;
    feats.currentKitchenWorkload = counterWorkload;

    if (items.empty()) {
        feats.foodCategory = 0;
        feats.orderQuantity = 1;
        feats.preparationComplexity = 2;
        return feats;
    }

    int totalQty = 0;
    int maxComplexity = 1;
    int dominantCategory = items[0].item.getCategoryCode();
    int maxQtyForCategory = 0;

    // Track category quantities to pick dominant category in order
    int categoryQuantities[5] = {0, 0, 0, 0, 0};

    for (const auto& line : items) {
        totalQty += line.quantity;
        int cat = line.item.getCategoryCode();
        if (cat >= 0 && cat < 5) {
            categoryQuantities[cat] += line.quantity;
            if (categoryQuantities[cat] > maxQtyForCategory) {
                maxQtyForCategory = categoryQuantities[cat];
                dominantCategory = cat;
            }
        }
        if (line.item.getComplexity() > maxComplexity) {
            maxComplexity = line.item.getComplexity();
        }
    }

    feats.foodCategory = dominantCategory;
    feats.orderQuantity = std::max(1, totalQty);
    feats.preparationComplexity = std::min(5, std::max(1, maxComplexity));
    return feats;
}

} // namespace canteen
