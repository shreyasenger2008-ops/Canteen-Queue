#ifndef CANTEEN_ORDER_MANAGER_HPP
#define CANTEEN_ORDER_MANAGER_HPP

#include "canteen/models/Order.hpp"
#include "canteen/ds/SinglyLinkedList.hpp"
#include "canteen/core/CounterManager.hpp"
#include <vector>

namespace canteen {

/**
 * @brief Manages lifecycle of all canteen orders, maintains historical order log
 * using custom singly linked list, and provides linear search across all orders.
 */
class OrderManager {
private:
    int nextOrderId{101};
    SinglyLinkedList<Order> orderHistory; // Custom Singly Linked List for audit trail

public:
    OrderManager() = default;

    int generateNextOrderId() { return nextOrderId++; }
    void setNextOrderId(int next) { if (next > nextOrderId) nextOrderId = next; }

    // Order tracking
    void recordOrderPlaced(const Order& order);
    void recordOrderCompleted(const Order& order);
    void recordOrderCancelled(const Order& order);
    bool removeOrderFromHistory(int orderId);

    // Linear search for an order across counters and history (DSA requirement)
    bool findOrderById(int orderId, Order& orderOut, const CounterManager& counterManager) const;

    // History access & traversal
    const SinglyLinkedList<Order>& getOrderHistory() const { return orderHistory; }
    size_t getHistoryCount() const { return orderHistory.size(); }
    void displayOrderHistory() const;

    // Sorting history using Merge Sort
    std::vector<Order> getHistorySortedByPrepTime() const;
    std::vector<Order> getHistorySortedById() const;
};

} // namespace canteen

#endif // CANTEEN_ORDER_MANAGER_HPP
