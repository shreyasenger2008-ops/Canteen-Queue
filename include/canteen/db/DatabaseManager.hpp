#ifndef CANTEEN_DATABASE_MANAGER_HPP
#define CANTEEN_DATABASE_MANAGER_HPP

#include "sqlite3.h"
#include "canteen/models/FoodItem.hpp"
#include "canteen/models/Order.hpp"
#include "canteen/models/Counter.hpp"
#include "canteen/models/AdminAction.hpp"
#include "canteen/models/OrderStatus.hpp"
#include "canteen/core/MenuManager.hpp"
#include "canteen/core/CounterManager.hpp"
#include "canteen/core/OrderManager.hpp"

#include <string>
#include <vector>
#include <optional>
#include <mutex>

namespace canteen {
namespace db {

/**
 * @brief Manages SQLite relational persistence for the Canteen Queue Optimization System.
 * Guarantees atomic transactions, foreign-key enforcement, idempotency, and restart state recovery.
 */
class DatabaseManager {
private:
    sqlite3* db{nullptr};
    std::string dbFilePath;
    mutable std::recursive_mutex dbMutex;

    void createSchema();

public:
    DatabaseManager() = default;
    ~DatabaseManager();

    // Database connection lifecycle
    bool initialize(const std::string& path = "canteen.db");
    void close();
    bool isOpen() const { return db != nullptr; }
    const std::string& getDbPath() const { return dbFilePath; }

    // Transaction management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Menu operations
    bool syncMenuItems(const std::vector<FoodItem>& items);
    bool updateMenuItemAvailability(int itemId, bool available);
    bool updateMenuItemPrice(int itemId, double newPrice);
    std::vector<FoodItem> loadMenuItems();

    // Counter operations
    bool syncCounters(const CounterManager& counterManager);
    bool updateCounterAvailability(int counterId, bool available);

    // Order operations
    bool saveNewOrder(const Order& order, const std::string& idempotencyKey = "");
    bool updateOrderStatus(int orderId, OrderStatus newStatus);
    bool updateOrderPaymentStatus(int orderId, const std::string& paymentStatus);
    bool deleteOrderById(int orderId);
    std::optional<Order> loadOrderById(int orderId);
    std::optional<Order> findOrderByIdempotencyKey(const std::string& key);
    std::vector<Order> loadAllOrders();
    std::vector<Order> loadActiveOrders(); // Orders in QUEUED or PREPARING status

    // Admin audit logging
    bool logAdminAction(const AdminAction& action);

    // Paginated orders query for admin dashboard
    struct PaginatedOrdersResult {
        std::vector<Order> orders;
        int totalCount{0};
        int limit{20};
        int offset{0};
    };
    PaginatedOrdersResult loadOrdersPaginated(int limit = 20, int offset = 0, const std::string& statusFilter = "");

    // State recovery on application start
    bool recoverSystemState(MenuManager& menu, CounterManager& counters, OrderManager& orders);
};

} // namespace db
} // namespace canteen

#endif // CANTEEN_DATABASE_MANAGER_HPP
