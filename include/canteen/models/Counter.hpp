#ifndef CANTEEN_COUNTER_HPP
#define CANTEEN_COUNTER_HPP

#include "canteen/ds/Queue.hpp"
#include "canteen/models/Order.hpp"
#include <string>

namespace canteen {

/**
 * @brief Represents a service counter in the college canteen.
 * Maintains its own FIFO queue of orders, tracks kitchen workload,
 * and handles order lifecycle progression.
 */
class Counter {
private:
    int id{0};
    std::string name;
    std::string counterType;
    bool isAvailable{true};
    int currentWorkloadMinutes{0};
    int totalOrdersServed{0};
    Queue<Order> pendingQueue;

public:
    Counter() = default;
    Counter(int id, std::string name, std::string counterType, bool available = true);

    // Getters
    int getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getCounterType() const { return counterType; }
    bool getIsAvailable() const { return isAvailable; }
    int getCurrentWorkloadMinutes() const { return currentWorkloadMinutes; }
    int getTotalOrdersServed() const { return totalOrdersServed; }
    size_t getPendingOrderCount() const { return pendingQueue.size(); }
    int getEstimatedWaitTime() const { return currentWorkloadMinutes; }
    const Queue<Order>& getPendingQueue() const { return pendingQueue; }
    Queue<Order>& getPendingQueue() { return pendingQueue; }

    // Setters
    void setId(int newId) { id = newId; }
    void setName(const std::string& newName) { name = newName; }
    void setCounterType(const std::string& type) { counterType = type; }
    void setIsAvailable(bool available) { isAvailable = available; }

    // Queue & order lifecycle operations
    void enqueueOrder(Order order);
    bool startPreparingFront();
    Order completeFrontOrder();
    bool cancelOrder(int orderId, Order& cancelledOrderOut);

    // Display helpers
    void displaySummary() const;
    void displayQueue() const;
    std::string toString() const;
};

} // namespace canteen

#endif // CANTEEN_COUNTER_HPP
