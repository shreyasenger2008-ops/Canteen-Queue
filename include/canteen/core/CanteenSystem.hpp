#ifndef CANTEEN_SYSTEM_HPP
#define CANTEEN_SYSTEM_HPP

#include "canteen/core/MenuManager.hpp"
#include "canteen/core/CounterManager.hpp"
#include "canteen/core/OrderManager.hpp"
#include <vector>
#include <utility>

namespace canteen {

/**
 * @brief High-level facade for the AI-Based College Canteen Queue Optimization System.
 * Connects all core DSA data structures, algorithms, and models into a cohesive
 * console simulation and testing environment.
 */
class CanteenSystem {
private:
    MenuManager menuManager;
    CounterManager counterManager;
    OrderManager orderManager;

    void seedDefaultData();

public:
    CanteenSystem();

    // Accessors to managers
    MenuManager& getMenuManager() { return menuManager; }
    const MenuManager& getMenuManager() const { return menuManager; }

    CounterManager& getCounterManager() { return counterManager; }
    const CounterManager& getCounterManager() const { return counterManager; }

    OrderManager& getOrderManager() { return orderManager; }
    const OrderManager& getOrderManager() const { return orderManager; }

    // Core Business Workflows
    int placeOrder(const std::string& studentId, const std::string& studentName,
                   const std::vector<std::pair<int, int>>& itemRequests,
                   int preferredCounterId = -1);

    bool startPreparingOrder(int counterId);
    bool completeAndCollectOrder(int counterId, Order* completedOrderOut = nullptr);
    bool cancelOrder(int orderId);

    // Searching operations
    bool searchOrderById(int orderId, Order& orderOut) const;
    int searchFoodItemLinear(int itemId) const;
    int searchFoodItemBinary(int itemId) const;

    // Sorting operations
    void sortCountersByWaitTime(bool ascending = true);
    void sortMenuByPrepTime();
    void sortMenuByPrice();

    // Stack Undo Operations
    bool toggleItemAvailability(int itemId, bool available);
    bool undoLastAdminChange(std::string& messageOut);

    // Display helpers
    void displayDashboard() const;
    void displayMenu() const;
    void displayCounters() const;
    void displayCounterQueue(int counterId) const;
    void displayOrderHistory() const;

    // Interactive CLI and Demo
    void runInteractiveCLI();
    void runDemonstrationSimulation();
};

} // namespace canteen

#endif // CANTEEN_SYSTEM_HPP
