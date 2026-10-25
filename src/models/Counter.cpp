#include "canteen/models/Counter.hpp"
#include <iomanip>
#include <sstream>
#include <iostream>

namespace canteen {

Counter::Counter(int id, std::string name, std::string counterType, bool available)
    : id(id), name(std::move(name)), counterType(std::move(counterType)),
      isAvailable(available), currentWorkloadMinutes(0), totalOrdersServed(0) {}

void Counter::enqueueOrder(Order order) {
    order.setAssignedCounterId(id);
    currentWorkloadMinutes += order.getTotalPreparationTimeMinutes();
    pendingQueue.enqueue(std::move(order));
}

bool Counter::startPreparingFront() {
    if (pendingQueue.isEmpty()) {
        return false;
    }
    Order& frontOrder = pendingQueue.front();
    if (frontOrder.getStatus() == OrderStatus::QUEUED) {
        frontOrder.transitionTo(OrderStatus::PREPARING);
        return true;
    }
    return false;
}

Order Counter::completeFrontOrder() {
    if (pendingQueue.isEmpty()) {
        throw std::underflow_error("No orders in counter queue to complete.");
    }
    Order completedOrder = pendingQueue.dequeue();
    
    // Decrement workload
    if (currentWorkloadMinutes >= completedOrder.getTotalPreparationTimeMinutes()) {
        currentWorkloadMinutes -= completedOrder.getTotalPreparationTimeMinutes();
    } else {
        currentWorkloadMinutes = 0;
    }
    
    // Transition status to READY (progressing through PREPARING if still QUEUED)
    if (completedOrder.getStatus() == OrderStatus::QUEUED) {
        completedOrder.transitionTo(OrderStatus::PREPARING);
    }
    if (completedOrder.getStatus() == OrderStatus::PREPARING) {
        completedOrder.transitionTo(OrderStatus::READY);
    }
    
    ++totalOrdersServed;
    return completedOrder;
}

bool Counter::cancelOrder(int orderId, Order& cancelledOrderOut) {
    if (pendingQueue.isEmpty()) {
        return false;
    }

    // First find the order
    const Order* found = pendingQueue.find([orderId](const Order& o) {
        return o.getId() == orderId;
    });

    if (found == nullptr) {
        return false;
    }

    // Copy order before removing
    cancelledOrderOut = *found;
    int prepTime = cancelledOrderOut.getTotalPreparationTimeMinutes();

    // Adjust workload
    if (currentWorkloadMinutes >= prepTime) {
        currentWorkloadMinutes -= prepTime;
    } else {
        currentWorkloadMinutes = 0;
    }

    cancelledOrderOut.transitionTo(OrderStatus::CANCELLED);

    // Remove from FIFO queue
    pendingQueue.removeIf([orderId](const Order& o) {
        return o.getId() == orderId;
    });

    return true;
}

std::string Counter::toString() const {
    std::ostringstream oss;
    oss << "[Counter #" << id << "] " << std::left << std::setw(22) << name
        << " | Type: " << std::setw(16) << counterType
        << " | Status: " << (isAvailable ? "ACTIVE  " : "OFFLINE ")
        << " | Pending: " << std::setw(2) << pendingQueue.size()
        << " | Wait: " << std::setw(3) << currentWorkloadMinutes << " mins"
        << " | Served: " << totalOrdersServed;
    return oss.str();
}

void Counter::displaySummary() const {
    std::cout << toString() << "\n";
}

void Counter::displayQueue() const {
    std::cout << "\n=== Pending Orders for Counter #" << id << " (" << name << ") ===\n";
    pendingQueue.displayQueue();
}

} // namespace canteen
