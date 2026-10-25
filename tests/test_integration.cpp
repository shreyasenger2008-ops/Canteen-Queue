#include "test_framework.hpp"
#include "canteen/core/CanteenSystem.hpp"
#include "canteen/models/OrderStatus.hpp"

using namespace canteen;

TEST_CASE(Integration_OrderPlacementAndAllocation) {
    CanteenSystem system;

    // Initially, all counters have 0 workload.
    // Order 1 should allocate to Counter #1 (tie-break rule: lowest counter ID among equal workloads)
    int o1 = system.placeOrder("S101", "Student One", {{101, 1}}); // 7 mins
    ASSERT_EQ(o1, 101);

    Order order1;
    ASSERT_TRUE(system.searchOrderById(o1, order1));
    ASSERT_EQ(order1.getAssignedCounterId(), 1);
    ASSERT_EQ(order1.getStatus(), OrderStatus::QUEUED);

    // Counter #1 now has 7 mins workload.
    // Order 2 should allocate to Counter #2 (0 workload < 7)
    int o2 = system.placeOrder("S102", "Student Two", {{103, 1}}); // 5 mins
    Order order2;
    ASSERT_TRUE(system.searchOrderById(o2, order2));
    ASSERT_EQ(order2.getAssignedCounterId(), 2);
}

TEST_CASE(Integration_ValidAndInvalidStatusTransitions) {
    Order order(1, "S1", "Alice");
    ASSERT_EQ(order.getStatus(), OrderStatus::QUEUED);

    // Valid transition: QUEUED -> PREPARING
    ASSERT_TRUE(order.transitionTo(OrderStatus::PREPARING));
    ASSERT_EQ(order.getStatus(), OrderStatus::PREPARING);

    // Valid transition: PREPARING -> READY
    ASSERT_TRUE(order.transitionTo(OrderStatus::READY));
    ASSERT_EQ(order.getStatus(), OrderStatus::READY);

    // Valid transition: READY -> COLLECTED
    ASSERT_TRUE(order.transitionTo(OrderStatus::COLLECTED));
    ASSERT_EQ(order.getStatus(), OrderStatus::COLLECTED);

    // Invalid transition: COLLECTED is terminal, cannot transition back to QUEUED
    ASSERT_THROWS(order.transitionTo(OrderStatus::QUEUED), std::invalid_argument);

    // Order cancellation test
    Order orderCan(2, "S2", "Bob");
    ASSERT_TRUE(orderCan.transitionTo(OrderStatus::CANCELLED));
    ASSERT_EQ(orderCan.getStatus(), OrderStatus::CANCELLED);

    // Invalid transition: CANCELLED is terminal, cannot transition to READY
    ASSERT_THROWS(orderCan.transitionTo(OrderStatus::READY), std::invalid_argument);
}

TEST_CASE(Integration_CompleteOrderWorkflowAndWorkloadUpdate) {
    CanteenSystem system;
    system.setUseMLPredictor(false); // Test deterministic baseline workload accounting
    Counter* c1 = system.getCounterManager().getCounterById(1);
    ASSERT_TRUE(c1 != nullptr);
    int initialWorkload = c1->getCurrentWorkloadMinutes();
    ASSERT_EQ(initialWorkload, 0);

    // Place order explicitly at Counter 1
    int oId = system.placeOrder("S1", "User1", {{101, 1}}, 1); // 7 mins
    ASSERT_EQ(c1->getCurrentWorkloadMinutes(), 7);
    ASSERT_EQ(c1->getPendingOrderCount(), 1);

    // Start preparing front order
    ASSERT_TRUE(system.startPreparingOrder(1));
    ASSERT_EQ(c1->getPendingQueue().front().getStatus(), OrderStatus::PREPARING);

    // Complete order
    Order completed;
    ASSERT_TRUE(system.completeAndCollectOrder(1, &completed));
    ASSERT_EQ(completed.getId(), oId);
    ASSERT_EQ(completed.getStatus(), OrderStatus::COLLECTED);

    // Workload must be deducted back to 0
    ASSERT_EQ(c1->getCurrentWorkloadMinutes(), 0);
    ASSERT_EQ(c1->getPendingOrderCount(), 0);
    ASSERT_EQ(c1->getTotalOrdersServed(), 1);
}

TEST_CASE(Integration_CancellationReclaimsWorkloadAndClearsQueue) {
    CanteenSystem system;
    system.setUseMLPredictor(false); // Test deterministic baseline workload accounting
    Counter* c1 = system.getCounterManager().getCounterById(1);
    int o1 = system.placeOrder("S1", "User1", {{101, 1}}, 1); // 7 mins
    int o2 = system.placeOrder("S2", "User2", {{102, 1}}, 1); // 8 mins

    ASSERT_EQ(c1->getCurrentWorkloadMinutes(), 15);
    ASSERT_EQ(c1->getPendingOrderCount(), 2);

    // Cancel first order
    ASSERT_TRUE(system.cancelOrder(o1));

    // Counter workload should decrease by 7 mins to 8 mins
    ASSERT_EQ(c1->getCurrentWorkloadMinutes(), 8);
    ASSERT_EQ(c1->getPendingOrderCount(), 1);

    // Remaining order must be o2
    ASSERT_EQ(c1->getPendingQueue().front().getId(), o2);

    // Cancelling already cancelled or nonexistent order returns false
    ASSERT_FALSE(system.cancelOrder(o1));
}
