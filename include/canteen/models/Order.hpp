#ifndef CANTEEN_ORDER_HPP
#define CANTEEN_ORDER_HPP

#include "canteen/models/FoodItem.hpp"
#include "canteen/models/OrderStatus.hpp"
#include "canteen/ml/IPrepTimePredictor.hpp"
#include <vector>
#include <string>

namespace canteen {

/**
 * @brief Represents an individual line item in an order.
 */
struct OrderLineItem {
    FoodItem item;
    int quantity{1};

    OrderLineItem() = default;
    OrderLineItem(FoodItem it, int qty) : item(std::move(it)), quantity(qty) {}
};

/**
 * @brief Represents a customer order in the college canteen system.
 */
class Order {
private:
    int id{0};
    std::string studentId;
    std::string studentName;
    std::vector<OrderLineItem> items;
    int totalPreparationTimeMinutes{0};
    double totalAmount{0.0};
    OrderStatus status{OrderStatus::QUEUED};
    int assignedCounterId{-1};
    long long orderTimestamp{0};

    void recalculateTotals();

public:
    Order() = default;
    Order(int id, std::string studentId, std::string studentName, long long timestamp = 0);

    // Items management
    void addItem(const FoodItem& item, int quantity = 1);
    const std::vector<OrderLineItem>& getItems() const { return items; }

    // State transition with strict validation
    bool transitionTo(OrderStatus newStatus);

    // Getters
    int getId() const { return id; }
    const std::string& getStudentId() const { return studentId; }
    const std::string& getStudentName() const { return studentName; }
    int getTotalPreparationTimeMinutes() const { return totalPreparationTimeMinutes; }
    double getTotalAmount() const { return totalAmount; }
    OrderStatus getStatus() const { return status; }
    int getAssignedCounterId() const { return assignedCounterId; }
    long long getOrderTimestamp() const { return orderTimestamp; }

    // Setters
    void setId(int newId) { id = newId; }
    void setAssignedCounterId(int counterId) { assignedCounterId = counterId; }
    void setOrderTimestamp(long long ts) { orderTimestamp = ts; }
    void setTotalPreparationTimeMinutes(int mins) { if (mins > 0) totalPreparationTimeMinutes = mins; }

    // Feature extraction for ML inference
    ml::OrderPredictionFeatures extractPredictionFeatures(double timeOfDayHour = 12.5, double counterWorkload = 0.0) const;

    // Helpers
    std::string toString() const;
    void display() const;
};

} // namespace canteen

#endif // CANTEEN_ORDER_HPP
