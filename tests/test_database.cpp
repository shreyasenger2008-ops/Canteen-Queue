#include "test_framework.hpp"
#include "canteen/db/DatabaseManager.hpp"
#include "canteen/core/CanteenSystem.hpp"
#include <cstdio>
#include <fstream>

using namespace canteen;

TEST(DB_InitializeAndSchemaCreation) {
    const std::string testDb = "test_init.db";
    std::remove(testDb.c_str());

    db::DatabaseManager db;
    ASSERT_TRUE(db.initialize(testDb));
    ASSERT_TRUE(db.isOpen());
    ASSERT_EQ(db.getDbPath(), testDb);

    db.close();
    ASSERT_FALSE(db.isOpen());
    std::remove(testDb.c_str());
}

TEST(DB_SyncAndLoadMenuItems) {
    const std::string testDb = "test_menu.db";
    std::remove(testDb.c_str());

    db::DatabaseManager db;
    ASSERT_TRUE(db.initialize(testDb));

    std::vector<FoodItem> items = {
        FoodItem(1, "Test Burger", "Fast Food", 50.0, 5, true, 2),
        FoodItem(2, "Test Tea", "Beverages", 15.0, 2, true, 1)
    };

    ASSERT_TRUE(db.syncMenuItems(items));
    auto loaded = db.loadMenuItems();
    ASSERT_EQ(loaded.size(), size_t(2));
    ASSERT_EQ(loaded[0].getName(), std::string("Test Burger"));
    ASSERT_EQ(loaded[1].getPrice(), 15.0);

    // Update availability
    ASSERT_TRUE(db.updateMenuItemAvailability(1, false));
    // Update price
    ASSERT_TRUE(db.updateMenuItemPrice(2, 20.0));

    auto updated = db.loadMenuItems();
    ASSERT_FALSE(updated[0].getIsAvailable());
    ASSERT_EQ(updated[1].getPrice(), 20.0);

    db.close();
    std::remove(testDb.c_str());
}

TEST(DB_SaveAndLoadOrderWithItems) {
    const std::string testDb = "test_order.db";
    std::remove(testDb.c_str());

    db::DatabaseManager db;
    ASSERT_TRUE(db.initialize(testDb));

    // Must have menu items and counters for foreign key integrity
    std::vector<FoodItem> items = {
        FoodItem(10, "Wrap", "Snacks", 40.0, 4, true, 2),
        FoodItem(20, "Juice", "Beverages", 30.0, 2, true, 1)
    };
    ASSERT_TRUE(db.syncMenuItems(items));

    CounterManager cm;
    cm.addCounter(Counter(1, "Counter 1", "Fast Food", true));
    cm.addCounter(Counter(2, "Counter 2", "Beverages", true));
    ASSERT_TRUE(db.syncCounters(cm));

    Order o(501, "STU01", "Alice", 1700000000);
    o.addItem(items[0], 2); // 80.0
    o.addItem(items[1], 1); // 30.0
    o.setAssignedCounterId(2);

    ASSERT_TRUE(db.saveNewOrder(o, "idemp-key-501"));

    auto loaded = db.loadOrderById(501);
    ASSERT_TRUE(loaded.has_value());
    ASSERT_EQ(loaded->getId(), 501);
    ASSERT_EQ(loaded->getStudentId(), std::string("STU01"));
    ASSERT_EQ(loaded->getStudentName(), std::string("Alice"));
    ASSERT_EQ(loaded->getAssignedCounterId(), 2);
    ASSERT_EQ(loaded->getStatus(), OrderStatus::QUEUED);
    ASSERT_EQ(loaded->getTotalAmount(), 110.0);
    ASSERT_EQ(loaded->getItems().size(), size_t(2));
    ASSERT_EQ(loaded->getItems()[0].quantity, 2);
    ASSERT_EQ(loaded->getItems()[1].quantity, 1);

    db.close();
    std::remove(testDb.c_str());
}

TEST(DB_IdempotencyKeyDeduplication) {
    const std::string testDb = "test_idemp.db";
    std::remove(testDb.c_str());

    db::DatabaseManager db;
    ASSERT_TRUE(db.initialize(testDb));

    std::vector<FoodItem> items = { FoodItem(1, "Item1", "Cat", 10.0, 2, true, 1) };
    db.syncMenuItems(items);

    CounterManager cm;
    cm.addCounter(Counter(1, "Counter 1", "Express", true));
    db.syncCounters(cm);

    Order o1(101, "STU1", "Bob", 1700000010);
    o1.addItem(items[0], 1);
    o1.setAssignedCounterId(1);

    ASSERT_TRUE(db.saveNewOrder(o1, "txn-unique-12345"));

    // Find by key
    auto found = db.findOrderByIdempotencyKey("txn-unique-12345");
    ASSERT_TRUE(found.has_value());
    ASSERT_EQ(found->getId(), 101);

    // Try inserting another order with duplicate key - should fail due to UNIQUE constraint
    Order o2(102, "STU2", "Charlie", 1700000020);
    o2.addItem(items[0], 1);
    o2.setAssignedCounterId(1);
    ASSERT_FALSE(db.saveNewOrder(o2, "txn-unique-12345"));

    db.close();
    std::remove(testDb.c_str());
}

TEST(DB_OrderStatusTransitionsAndActiveFilter) {
    const std::string testDb = "test_status.db";
    std::remove(testDb.c_str());

    db::DatabaseManager db;
    ASSERT_TRUE(db.initialize(testDb));

    std::vector<FoodItem> items = { FoodItem(1, "Item1", "Cat", 10.0, 2, true, 1) };
    db.syncMenuItems(items);

    CounterManager cm;
    cm.addCounter(Counter(1, "Counter 1", "Express", true));
    db.syncCounters(cm);

    Order o1(201, "S1", "User1", 100);
    o1.addItem(items[0], 1);
    o1.setAssignedCounterId(1);
    ASSERT_TRUE(db.saveNewOrder(o1));

    Order o2(202, "S2", "User2", 200);
    o2.addItem(items[0], 1);
    o2.setAssignedCounterId(1);
    ASSERT_TRUE(db.saveNewOrder(o2));

    Order o3(203, "S3", "User3", 300);
    o3.addItem(items[0], 1);
    o3.setAssignedCounterId(1);
    ASSERT_TRUE(db.saveNewOrder(o3));

    // Transition o1 -> PREPARING
    ASSERT_TRUE(db.updateOrderStatus(201, OrderStatus::PREPARING));
    // Transition o3 -> CANCELLED
    ASSERT_TRUE(db.updateOrderStatus(203, OrderStatus::CANCELLED));

    // Active orders should be only o1 (PREPARING) and o2 (QUEUED), NOT o3 (CANCELLED)
    auto active = db.loadActiveOrders();
    ASSERT_EQ(active.size(), size_t(2));
    ASSERT_EQ(active[0].getId(), 201);
    ASSERT_EQ(active[0].getStatus(), OrderStatus::PREPARING);
    ASSERT_EQ(active[1].getId(), 202);
    ASSERT_EQ(active[1].getStatus(), OrderStatus::QUEUED);

    // Transition o1 -> COLLECTED
    ASSERT_TRUE(db.updateOrderStatus(201, OrderStatus::COLLECTED));
    active = db.loadActiveOrders();
    ASSERT_EQ(active.size(), size_t(1));
    ASSERT_EQ(active[0].getId(), 202);

    db.close();
    std::remove(testDb.c_str());
}

TEST(DB_AdminAuditLogging) {
    const std::string testDb = "test_audit.db";
    std::remove(testDb.c_str());

    db::DatabaseManager db;
    ASSERT_TRUE(db.initialize(testDb));

    AdminAction act;
    act.type = ActionType::UPDATE_ITEM_AVAILABILITY;
    act.targetId = 101;
    act.targetName = "Cheese Burger";
    act.prevBoolValue = true;
    act.newBoolValue = false;
    act.description = "Marked out of stock due to shortage";
    act.timestamp = 1700000000;

    ASSERT_TRUE(db.logAdminAction(act));

    db.close();
    std::remove(testDb.c_str());
}

TEST(DB_SystemRecoveryOnRestart) {
    const std::string testDb = "test_recovery.db";
    std::remove(testDb.c_str());

    int order1Id = -1;
    int order2Id = -1;
    int order3Id = -1;

    // Session 1: Create system, place orders, modify statuses
    {
        CanteenSystem sys1(testDb);
        sys1.setUseMLPredictor(false); // deterministic prep time

        order1Id = sys1.placeOrder("S1", "Alice", {{101, 1}}, 1); // Counter 1
        order2Id = sys1.placeOrder("S2", "Bob", {{102, 1}}, 1);   // Counter 1
        order3Id = sys1.placeOrder("S3", "Charlie", {{105, 1}}, 3); // Counter 3

        ASSERT_TRUE(order1Id > 0);
        ASSERT_TRUE(order2Id > 0);
        ASSERT_TRUE(order3Id > 0);

        // Transition order 1 to PREPARING
        ASSERT_TRUE(sys1.startPreparingOrder(1));

        // Complete & collect order 3 at Counter 3
        Order completed;
        ASSERT_TRUE(sys1.completeAndCollectOrder(3, &completed));
        ASSERT_EQ(completed.getId(), order3Id);
        ASSERT_EQ(completed.getStatus(), OrderStatus::COLLECTED);
    } // sys1 destructs, simulated restart

    // Session 2: Instantiate new CanteenSystem with the same database
    {
        CanteenSystem sys2(testDb);
        sys2.setUseMLPredictor(false);

        // Counter 1 must have restored:
        // - order 1 (PREPARING) at the front of queue
        // - order 2 (QUEUED) behind it
        const Counter* c1 = sys2.getCounterManager().getCounterById(1);
        ASSERT_TRUE(c1 != nullptr);
        ASSERT_EQ(c1->getPendingOrderCount(), size_t(2));
        ASSERT_EQ(c1->getPendingQueue().front().getId(), order1Id);
        ASSERT_EQ(c1->getPendingQueue().front().getStatus(), OrderStatus::PREPARING);

        // Counter 3 had order 3 COLLECTED, so its queue must be empty!
        const Counter* c3 = sys2.getCounterManager().getCounterById(3);
        ASSERT_TRUE(c3 != nullptr);
        ASSERT_EQ(c3->getPendingOrderCount(), size_t(0));
        ASSERT_EQ(c3->getCurrentWorkloadMinutes(), 0);

        // Search for all 3 orders via sys2 OrderManager
        Order recovered1, recovered2, recovered3;
        ASSERT_TRUE(sys2.searchOrderById(order1Id, recovered1));
        ASSERT_TRUE(sys2.searchOrderById(order2Id, recovered2));
        ASSERT_TRUE(sys2.searchOrderById(order3Id, recovered3));

        ASSERT_EQ(recovered1.getStatus(), OrderStatus::PREPARING);
        ASSERT_EQ(recovered2.getStatus(), OrderStatus::QUEUED);
        ASSERT_EQ(recovered3.getStatus(), OrderStatus::COLLECTED);
    }

    std::remove(testDb.c_str());
}

TEST(DB_LoadOrdersPaginatedAndFilter) {
    const std::string testDb = "test_paginated.db";
    std::remove(testDb.c_str());

    db::DatabaseManager db;
    ASSERT_TRUE(db.initialize(testDb));

    std::vector<FoodItem> items = { FoodItem(1, "Item1", "Cat", 10.0, 2, true, 1) };
    db.syncMenuItems(items);

    CounterManager cm;
    cm.addCounter(Counter(1, "Counter 1", "Express", true));
    db.syncCounters(cm);

    // Insert 5 orders with different statuses and timestamps
    for (int i = 1; i <= 5; ++i) {
        Order o(100 + i, "STU_" + std::to_string(i), "Student " + std::to_string(i), 1000 + i * 10);
        o.addItem(items[0], 1);
        o.setAssignedCounterId(1);
        ASSERT_TRUE(db.saveNewOrder(o));
    }

    // Update statuses: 101: QUEUED, 102: PREPARING, 103: READY, 104: COLLECTED, 105: CANCELLED
    db.updateOrderStatus(102, OrderStatus::PREPARING);
    db.updateOrderStatus(103, OrderStatus::READY);
    db.updateOrderStatus(104, OrderStatus::COLLECTED);
    db.updateOrderStatus(105, OrderStatus::CANCELLED);

    // 1. Paginated slice (page 1: limit 2, offset 0) -> newest first (105, 104)
    auto page1 = db.loadOrdersPaginated(2, 0, "");
    ASSERT_EQ(page1.totalCount, 5);
    ASSERT_EQ(page1.orders.size(), size_t(2));
    ASSERT_EQ(page1.orders[0].getId(), 105);
    ASSERT_EQ(page1.orders[1].getId(), 104);

    // 2. Paginated slice (page 2: limit 2, offset 2) -> (103, 102)
    auto page2 = db.loadOrdersPaginated(2, 2, "");
    ASSERT_EQ(page2.totalCount, 5);
    ASSERT_EQ(page2.orders.size(), size_t(2));
    ASSERT_EQ(page2.orders[0].getId(), 103);
    ASSERT_EQ(page2.orders[1].getId(), 102);

    // 3. Status filter: PREPARING -> only 102
    auto prepPage = db.loadOrdersPaginated(10, 0, "PREPARING");
    ASSERT_EQ(prepPage.totalCount, 1);
    ASSERT_EQ(prepPage.orders.size(), size_t(1));
    ASSERT_EQ(prepPage.orders[0].getId(), 102);
    ASSERT_EQ(prepPage.orders[0].getStatus(), OrderStatus::PREPARING);

    // 4. Non-matching status filter -> empty result
    auto emptyPage = db.loadOrdersPaginated(10, 0, "NON_EXISTENT");
    ASSERT_EQ(emptyPage.totalCount, 0);
    ASSERT_TRUE(emptyPage.orders.empty());

    // 5. Offset beyond total count -> empty slice
    auto beyondPage = db.loadOrdersPaginated(10, 100, "");
    ASSERT_EQ(beyondPage.totalCount, 5);
    ASSERT_TRUE(beyondPage.orders.empty());

    db.close();
    std::remove(testDb.c_str());
}

TEST(DB_PaymentStatusPersistence) {
    const std::string testDb = "test_payment_persist.db";
    std::remove(testDb.c_str());

    db::DatabaseManager db;
    ASSERT_TRUE(db.initialize(testDb));

    std::vector<FoodItem> items = {FoodItem(1, "Poha", "Breakfast", 30.0, 7, true, 2)};
    ASSERT_TRUE(db.syncMenuItems(items));
    CounterManager cm;
    cm.addCounter(Counter(1, "Counter 1", "Breakfast", true));
    ASSERT_TRUE(db.syncCounters(cm));

    Order o(900, "STU900", "Payment Test", 1700000100);
    o.addItem(items[0], 1);
    o.setAssignedCounterId(1);
    ASSERT_TRUE(db.saveNewOrder(o));

    // Default payment status should be PENDING
    auto loaded = db.loadOrderById(900);
    ASSERT_TRUE(loaded.has_value());
    ASSERT_EQ(loaded->getPaymentStatus(), std::string("PENDING"));

    // Update payment status
    ASSERT_TRUE(db.updateOrderPaymentStatus(900, "PAID"));

    // Reload and verify persistence
    auto reloaded = db.loadOrderById(900);
    ASSERT_TRUE(reloaded.has_value());
    ASSERT_EQ(reloaded->getPaymentStatus(), std::string("PAID"));

    db.close();
    std::remove(testDb.c_str());
}

TEST(DB_PaymentStatusSurvivesRestart) {
    const std::string testDb = "test_payment_restart.db";
    std::remove(testDb.c_str());

    {
        db::DatabaseManager db;
        ASSERT_TRUE(db.initialize(testDb));
        std::vector<FoodItem> items = {FoodItem(1, "Samosa", "Snacks", 20.0, 5, true, 1)};
        ASSERT_TRUE(db.syncMenuItems(items));
        CounterManager cm;
        cm.addCounter(Counter(1, "Counter 1", "General", true));
        ASSERT_TRUE(db.syncCounters(cm));
        Order o(901, "STU901", "Restart Test", 1700000200);
        o.addItem(items[0], 2);
        o.setAssignedCounterId(1);
        ASSERT_TRUE(db.saveNewOrder(o));
        ASSERT_TRUE(db.updateOrderPaymentStatus(901, "PAID"));
    } // db closes here

    {
        db::DatabaseManager db2;
        ASSERT_TRUE(db2.initialize(testDb));
        auto loaded = db2.loadOrderById(901);
        ASSERT_TRUE(loaded.has_value());
        ASSERT_EQ(loaded->getPaymentStatus(), std::string("PAID"));
    }

    std::remove(testDb.c_str());
}

