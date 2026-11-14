#include "canteen/db/DatabaseManager.hpp"
#include <iostream>
#include <stdexcept>
#include <algorithm>

namespace canteen {
namespace db {

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::initialize(const std::string& path) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    close();

    dbFilePath = path;
    int rc = sqlite3_open(path.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB ERROR] Cannot open database " << path << ": "
                  << sqlite3_errmsg(db) << "\n";
        db = nullptr;
        return false;
    }

    createSchema();
    return true;
}

void DatabaseManager::close() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
    }
}

void DatabaseManager::createSchema() {
    if (db == nullptr) return;

    const char* schemaSql = R"(
        PRAGMA foreign_keys = ON;

        CREATE TABLE IF NOT EXISTS menu_items (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL,
            category TEXT NOT NULL,
            price REAL NOT NULL,
            prep_time_minutes INTEGER NOT NULL,
            is_available INTEGER NOT NULL DEFAULT 1,
            complexity INTEGER NOT NULL DEFAULT 2
        );

        CREATE TABLE IF NOT EXISTS counters (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL,
            counter_type TEXT NOT NULL,
            is_available INTEGER NOT NULL DEFAULT 1
        );

        CREATE TABLE IF NOT EXISTS orders (
            id INTEGER PRIMARY KEY,
            student_id TEXT NOT NULL,
            student_name TEXT NOT NULL,
            status TEXT NOT NULL,
            payment_status TEXT NOT NULL DEFAULT 'PENDING',
            assigned_counter_id INTEGER NOT NULL,
            total_prep_time INTEGER NOT NULL,
            total_amount REAL NOT NULL,
            order_timestamp INTEGER NOT NULL,
            idempotency_key TEXT UNIQUE,
            FOREIGN KEY(assigned_counter_id) REFERENCES counters(id)
        );

        CREATE TABLE IF NOT EXISTS order_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            order_id INTEGER NOT NULL,
            item_id INTEGER NOT NULL,
            item_name TEXT NOT NULL,
            item_price REAL NOT NULL,
            quantity INTEGER NOT NULL,
            FOREIGN KEY(order_id) REFERENCES orders(id) ON DELETE CASCADE,
            FOREIGN KEY(item_id) REFERENCES menu_items(id)
        );

        CREATE TABLE IF NOT EXISTS admin_audit_log (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            action_type TEXT NOT NULL,
            target_id INTEGER NOT NULL,
            target_name TEXT NOT NULL,
            prev_bool INTEGER,
            new_bool INTEGER,
            prev_double REAL,
            new_double REAL,
            description TEXT NOT NULL,
            timestamp INTEGER NOT NULL
        );
    )";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, schemaSql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "Unknown SQL error";
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to initialize database schema: " + err);
    }

    // Migration for existing databases that lack the payment_status column
    sqlite3_exec(db, "ALTER TABLE orders ADD COLUMN payment_status TEXT NOT NULL DEFAULT 'PENDING';", nullptr, nullptr, nullptr);
}

bool DatabaseManager::beginTransaction() {
    if (db == nullptr) return false;
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::commitTransaction() {
    if (db == nullptr) return false;
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::rollbackTransaction() {
    if (db == nullptr) return false;
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::syncMenuItems(const std::vector<FoodItem>& items) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr) return false;

    beginTransaction();
    const char* sql = R"(
        INSERT INTO menu_items (id, name, category, price, prep_time_minutes, is_available, complexity)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
            name = excluded.name,
            category = excluded.category,
            price = excluded.price,
            prep_time_minutes = excluded.prep_time_minutes,
            is_available = excluded.is_available,
            complexity = excluded.complexity;
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        rollbackTransaction();
        return false;
    }

    for (const auto& item : items) {
        sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, item.getId());
        sqlite3_bind_text(stmt, 2, item.getName().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, item.getCategory().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, item.getPrice());
        sqlite3_bind_int(stmt, 5, item.getPreparationTimeMinutes());
        sqlite3_bind_int(stmt, 6, item.getIsAvailable() ? 1 : 0);
        sqlite3_bind_int(stmt, 7, item.getComplexity());

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            rollbackTransaction();
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return commitTransaction();
}

bool DatabaseManager::updateMenuItemAvailability(int itemId, bool available) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr) return false;

    const char* sql = "UPDATE menu_items SET is_available = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, available ? 1 : 0);
    sqlite3_bind_int(stmt, 2, itemId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::updateMenuItemPrice(int itemId, double newPrice) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr) return false;

    const char* sql = "UPDATE menu_items SET price = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_double(stmt, 1, newPrice);
    sqlite3_bind_int(stmt, 2, itemId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

std::vector<FoodItem> DatabaseManager::loadMenuItems() {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    std::vector<FoodItem> list;
    if (db == nullptr) return list;

    const char* sql = "SELECT id, name, category, price, prep_time_minutes, is_available, complexity FROM menu_items ORDER BY id ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        double price = sqlite3_column_double(stmt, 3);
        int prepTime = sqlite3_column_int(stmt, 4);
        bool available = (sqlite3_column_int(stmt, 5) != 0);
        int comp = sqlite3_column_int(stmt, 6);

        list.emplace_back(id, name, category, price, prepTime, available, comp);
    }

    sqlite3_finalize(stmt);
    return list;
}

bool DatabaseManager::syncCounters(const CounterManager& counterManager) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr) return false;

    beginTransaction();
    const char* sql = R"(
        INSERT INTO counters (id, name, counter_type, is_available)
        VALUES (?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
            name = excluded.name,
            counter_type = excluded.counter_type,
            is_available = excluded.is_available;
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        rollbackTransaction();
        return false;
    }

    const auto& countersArr = counterManager.getCountersArray();
    for (size_t i = 0; i < countersArr.size(); ++i) {
        const auto& c = countersArr[i];
        sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, c.getId());
        sqlite3_bind_text(stmt, 2, c.getName().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, c.getCounterType().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, c.getIsAvailable() ? 1 : 0);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            rollbackTransaction();
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return commitTransaction();
}

bool DatabaseManager::updateCounterAvailability(int counterId, bool available) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr) return false;

    const char* sql = "UPDATE counters SET is_available = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, available ? 1 : 0);
    sqlite3_bind_int(stmt, 2, counterId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::saveNewOrder(const Order& order, const std::string& idempotencyKey) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr) return false;

    beginTransaction();

    // 1. Insert Order
    const char* orderSql = R"(
        INSERT INTO orders (id, student_id, student_name, status, payment_status, assigned_counter_id, total_prep_time, total_amount, order_timestamp, idempotency_key)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* orderStmt = nullptr;
    if (sqlite3_prepare_v2(db, orderSql, -1, &orderStmt, nullptr) != SQLITE_OK) {
        rollbackTransaction();
        return false;
    }

    sqlite3_bind_int(orderStmt, 1, order.getId());
    sqlite3_bind_text(orderStmt, 2, order.getStudentId().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(orderStmt, 3, order.getStudentName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(orderStmt, 4, orderStatusToString(order.getStatus()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(orderStmt, 5, order.getPaymentStatus().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(orderStmt, 6, order.getAssignedCounterId());
    sqlite3_bind_int(orderStmt, 7, order.getTotalPreparationTimeMinutes());
    sqlite3_bind_double(orderStmt, 8, order.getTotalAmount());
    sqlite3_bind_int64(orderStmt, 9, order.getOrderTimestamp());

    if (!idempotencyKey.empty()) {
        sqlite3_bind_text(orderStmt, 10, idempotencyKey.c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(orderStmt, 10);
    }

    if (sqlite3_step(orderStmt) != SQLITE_DONE) {
        sqlite3_finalize(orderStmt);
        rollbackTransaction();
        return false;
    }
    sqlite3_finalize(orderStmt);

    // 2. Insert Order Items
    const char* itemSql = R"(
        INSERT INTO order_items (order_id, item_id, item_name, item_price, quantity)
        VALUES (?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* itemStmt = nullptr;
    if (sqlite3_prepare_v2(db, itemSql, -1, &itemStmt, nullptr) != SQLITE_OK) {
        rollbackTransaction();
        return false;
    }

    for (const auto& line : order.getItems()) {
        sqlite3_reset(itemStmt);
        sqlite3_bind_int(itemStmt, 1, order.getId());
        sqlite3_bind_int(itemStmt, 2, line.item.getId());
        sqlite3_bind_text(itemStmt, 3, line.item.getName().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(itemStmt, 4, line.item.getPrice());
        sqlite3_bind_int(itemStmt, 5, line.quantity);

        if (sqlite3_step(itemStmt) != SQLITE_DONE) {
            sqlite3_finalize(itemStmt);
            rollbackTransaction();
            return false;
        }
    }
    sqlite3_finalize(itemStmt);

    return commitTransaction();
}

bool DatabaseManager::updateOrderStatus(int orderId, OrderStatus newStatus) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr) return false;

    const char* sql = "UPDATE orders SET status = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, orderStatusToString(newStatus).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, orderId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::updateOrderPaymentStatus(int orderId, const std::string& paymentStatus) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr) return false;

    const char* sql = "UPDATE orders SET payment_status = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, paymentStatus.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, orderId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::deleteOrderById(int orderId) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr) return false;

    if (!beginTransaction()) return false;

    const char* delItemsSql = "DELETE FROM order_items WHERE order_id = ?;";
    sqlite3_stmt* itemStmt = nullptr;
    if (sqlite3_prepare_v2(db, delItemsSql, -1, &itemStmt, nullptr) != SQLITE_OK) {
        rollbackTransaction();
        return false;
    }
    sqlite3_bind_int(itemStmt, 1, orderId);
    if (sqlite3_step(itemStmt) != SQLITE_DONE) {
        sqlite3_finalize(itemStmt);
        rollbackTransaction();
        return false;
    }
    sqlite3_finalize(itemStmt);

    const char* delOrderSql = "DELETE FROM orders WHERE id = ?;";
    sqlite3_stmt* orderStmt = nullptr;
    if (sqlite3_prepare_v2(db, delOrderSql, -1, &orderStmt, nullptr) != SQLITE_OK) {
        rollbackTransaction();
        return false;
    }
    sqlite3_bind_int(orderStmt, 1, orderId);
    if (sqlite3_step(orderStmt) != SQLITE_DONE) {
        sqlite3_finalize(orderStmt);
        rollbackTransaction();
        return false;
    }
    sqlite3_finalize(orderStmt);

    int affected = sqlite3_changes(db);
    if (!commitTransaction()) {
        rollbackTransaction();
        return false;
    }
    return affected > 0;
}

static OrderStatus parseStatus(const std::string& str) {
    if (str == "PREPARING") return OrderStatus::PREPARING;
    if (str == "READY")     return OrderStatus::READY;
    if (str == "COLLECTED") return OrderStatus::COLLECTED;
    if (str == "CANCELLED") return OrderStatus::CANCELLED;
    return OrderStatus::QUEUED;
}

std::optional<Order> DatabaseManager::loadOrderById(int orderId) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr) return std::nullopt;

    const char* orderSql = "SELECT id, student_id, student_name, status, payment_status, assigned_counter_id, total_prep_time, total_amount, order_timestamp FROM orders WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, orderSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, orderId);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    int id = sqlite3_column_int(stmt, 0);
    std::string sId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string sName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    std::string statusStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    std::string paymentStatusStr = "PENDING";
    if (sqlite3_column_text(stmt, 4) != nullptr) {
        paymentStatusStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    }
    int counterId = sqlite3_column_int(stmt, 5);
    int prepTime = sqlite3_column_int(stmt, 6);
    long long timestamp = sqlite3_column_int64(stmt, 8);
    sqlite3_finalize(stmt);

    Order order(id, sId, sName, timestamp);
    order.setAssignedCounterId(counterId);
    order.setPaymentStatus(paymentStatusStr);

    // Load line items
    const char* itemSql = "SELECT item_id, item_name, item_price, quantity FROM order_items WHERE order_id = ? ORDER BY id ASC;";
    sqlite3_stmt* itemStmt = nullptr;
    if (sqlite3_prepare_v2(db, itemSql, -1, &itemStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(itemStmt, 1, orderId);
        while (sqlite3_step(itemStmt) == SQLITE_ROW) {
            int itemId = sqlite3_column_int(itemStmt, 0);
            std::string itemName = reinterpret_cast<const char*>(sqlite3_column_text(itemStmt, 1));
            double itemPrice = sqlite3_column_double(itemStmt, 2);
            int qty = sqlite3_column_int(itemStmt, 3);

            FoodItem fi(itemId, itemName, "General", itemPrice, 0, true);
            order.addItem(fi, qty);
        }
        sqlite3_finalize(itemStmt);
    }

    order.setTotalPreparationTimeMinutes(prepTime);
    order.setStatus(parseStatus(statusStr));
    return order;
}

std::optional<Order> DatabaseManager::findOrderByIdempotencyKey(const std::string& key) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr || key.empty()) return std::nullopt;

    const char* sql = "SELECT id FROM orders WHERE idempotency_key = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int orderId = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return loadOrderById(orderId);
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Order> DatabaseManager::loadAllOrders() {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    std::vector<Order> orders;
    if (db == nullptr) return orders;

    const char* sql = "SELECT id FROM orders ORDER BY order_timestamp ASC, id ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return orders;
    }

    std::vector<int> ids;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ids.push_back(sqlite3_column_int(stmt, 0));
    }
    sqlite3_finalize(stmt);

    for (int orderId : ids) {
        auto optOrder = loadOrderById(orderId);
        if (optOrder.has_value()) {
            orders.push_back(std::move(*optOrder));
        }
    }
    return orders;
}

std::vector<Order> DatabaseManager::loadActiveOrders() {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    std::vector<Order> active;
    if (db == nullptr) return active;

    const char* sql = "SELECT id FROM orders WHERE status IN ('QUEUED', 'PREPARING') ORDER BY order_timestamp ASC, id ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return active;
    }

    std::vector<int> ids;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ids.push_back(sqlite3_column_int(stmt, 0));
    }
    sqlite3_finalize(stmt);

    for (int orderId : ids) {
        auto optOrder = loadOrderById(orderId);
        if (optOrder.has_value()) {
            active.push_back(std::move(*optOrder));
        }
    }
    return active;
}

DatabaseManager::PaginatedOrdersResult DatabaseManager::loadOrdersPaginated(int limit, int offset, const std::string& statusFilter) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    PaginatedOrdersResult result;
    result.limit = limit;
    result.offset = offset;
    if (db == nullptr) return result;

    // 1. Count total matching orders
    std::string countSql;
    if (!statusFilter.empty()) {
        countSql = "SELECT COUNT(*) FROM orders WHERE status = ?;";
    } else {
        countSql = "SELECT COUNT(*) FROM orders;";
    }

    sqlite3_stmt* countStmt = nullptr;
    if (sqlite3_prepare_v2(db, countSql.c_str(), -1, &countStmt, nullptr) == SQLITE_OK) {
        if (!statusFilter.empty()) {
            sqlite3_bind_text(countStmt, 1, statusFilter.c_str(), -1, SQLITE_TRANSIENT);
        }
        if (sqlite3_step(countStmt) == SQLITE_ROW) {
            result.totalCount = sqlite3_column_int(countStmt, 0);
        }
        sqlite3_finalize(countStmt);
    }

    if (result.totalCount == 0 || offset >= result.totalCount) {
        return result;
    }

    // 2. Fetch paginated slice of IDs
    // FIFO: active-queue statuses (QUEUED, PREPARING) sort oldest-first so staff
    // always see the earliest-placed order at the top. Completed/historical
    // statuses (COLLECTED, CANCELLED) and the unfiltered "ALL" view sort newest-
    // first for convenience.
    std::string selectSql;
    if (!statusFilter.empty()) {
        const bool isFifoStatus = (statusFilter == "QUEUED" || statusFilter == "PREPARING");
        const std::string orderDir = isFifoStatus ? "ASC" : "DESC";
        selectSql = "SELECT id FROM orders WHERE status = ? ORDER BY order_timestamp " + orderDir + ", id " + orderDir + " LIMIT ? OFFSET ?;";
    } else {
        selectSql = "SELECT id FROM orders ORDER BY order_timestamp DESC, id DESC LIMIT ? OFFSET ?;";
    }

    sqlite3_stmt* selectStmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &selectStmt, nullptr) != SQLITE_OK) {
        return result;
    }

    if (!statusFilter.empty()) {
        sqlite3_bind_text(selectStmt, 1, statusFilter.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(selectStmt, 2, limit);
        sqlite3_bind_int(selectStmt, 3, offset);
    } else {
        sqlite3_bind_int(selectStmt, 1, limit);
        sqlite3_bind_int(selectStmt, 2, offset);
    }

    std::vector<int> ids;
    while (sqlite3_step(selectStmt) == SQLITE_ROW) {
        ids.push_back(sqlite3_column_int(selectStmt, 0));
    }
    sqlite3_finalize(selectStmt);

    // 3. Reconstruct full orders with line items
    for (int orderId : ids) {
        auto optOrder = loadOrderById(orderId);
        if (optOrder.has_value()) {
            result.orders.push_back(std::move(*optOrder));
        }
    }

    return result;
}

bool DatabaseManager::logAdminAction(const AdminAction& action) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    if (db == nullptr) return false;

    const char* sql = R"(
        INSERT INTO admin_audit_log (action_type, target_id, target_name, prev_bool, new_bool, prev_double, new_double, description, timestamp)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string actType = !action.actionType.empty() ? action.actionType : actionTypeToString(action.type);
    sqlite3_bind_text(stmt, 1, actType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, action.targetId);
    sqlite3_bind_text(stmt, 3, action.targetName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, action.prevBoolValue ? 1 : 0);
    sqlite3_bind_int(stmt, 5, action.newBoolValue ? 1 : 0);
    sqlite3_bind_double(stmt, 6, action.prevDoubleValue);
    sqlite3_bind_double(stmt, 7, action.newDoubleValue);
    sqlite3_bind_text(stmt, 8, action.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 9, action.timestamp);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::recoverSystemState(MenuManager& menu, CounterManager& counters, OrderManager& orders) {
    if (db == nullptr) return false;

    // 1. Recover Menu Items
    auto dbItems = loadMenuItems();
    if (!dbItems.empty()) {
        for (const auto& it : dbItems) {
            FoodItem* existing = menu.getItemById(it.getId());
            if (existing != nullptr) {
                existing->setIsAvailable(it.getIsAvailable());
                existing->setPrice(it.getPrice());
            } else {
                menu.addItem(it);
            }
        }
    }

    // 2. Recover All Orders for Audit Trail & Order ID Sequence
    auto allOrders = loadAllOrders();
    int maxId = 100;
    for (const auto& ord : allOrders) {
        if (ord.getId() > maxId) {
            maxId = ord.getId();
        }
        orders.recordOrderPlaced(ord);
        if (ord.getStatus() == OrderStatus::COLLECTED || ord.getStatus() == OrderStatus::READY) {
            orders.recordOrderCompleted(ord);
        } else if (ord.getStatus() == OrderStatus::CANCELLED) {
            orders.recordOrderCancelled(ord);
        }
    }
    orders.setNextOrderId(maxId + 1);

    // 3. Recover Active Queues & Workloads for Counters
    // Preserves exact FIFO queue ordering without reassigning or duplicating orders
    auto activeOrders = loadActiveOrders();
    for (const auto& ord : activeOrders) {
        int cId = ord.getAssignedCounterId();
        Counter* counter = counters.getCounterById(cId);
        if (counter != nullptr) {
            counter->enqueueOrder(ord);
            if (ord.getStatus() == OrderStatus::PREPARING) {
                counter->startPreparingFront();
            }
        }
    }

    return true;
}

} // namespace db
} // namespace canteen
