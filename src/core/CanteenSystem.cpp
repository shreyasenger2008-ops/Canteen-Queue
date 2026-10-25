#include "canteen/core/CanteenSystem.hpp"
#include <iostream>
#include <iomanip>
#include <limits>
#include <chrono>

namespace canteen {

CanteenSystem::CanteenSystem() {
    seedDefaultData();
}

void CanteenSystem::seedDefaultData() {
    // 1. Seed Menu Items
    menuManager.addItem(FoodItem(101, "Veg Cheese Burger", "Fast Food", 75.0, 7, true));
    menuManager.addItem(FoodItem(102, "Paneer Kathi Roll", "Fast Food", 90.0, 8, true));
    menuManager.addItem(FoodItem(103, "Masala Dosa", "South Indian", 60.0, 5, true));
    menuManager.addItem(FoodItem(104, "Veg Fried Rice", "Chinese", 110.0, 10, true));
    menuManager.addItem(FoodItem(105, "Cold Coffee", "Beverages", 45.0, 3, true));
    menuManager.addItem(FoodItem(106, "Masala Chai", "Beverages", 20.0, 2, true));
    menuManager.addItem(FoodItem(107, "Chocolate Pastry", "Bakery", 50.0, 1, true));
    menuManager.addItem(FoodItem(108, "Samosa (2 pcs)", "Snacks", 30.0, 3, true));

    // 2. Seed Fixed Counter Collection
    counterManager.addCounter(Counter(1, "Express Fast Food #1", "Fast Food & Snacks", true));
    counterManager.addCounter(Counter(2, "Main Meals #2", "Rice & Meals", true));
    counterManager.addCounter(Counter(3, "Beverages & Cafe #3", "Drinks & Bakery", true));
    counterManager.addCounter(Counter(4, "Special Counter #4", "South Indian & Special", true));
}

int CanteenSystem::placeOrder(const std::string& studentId, const std::string& studentName,
                             const std::vector<std::pair<int, int>>& itemRequests,
                             int preferredCounterId) {
    if (itemRequests.empty()) {
        std::cout << "[ERROR] Cannot place order with 0 items.\n";
        return -1;
    }

    int orderId = orderManager.generateNextOrderId();
    auto now = std::chrono::system_clock::now().time_since_epoch();
    long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(now).count();

    Order newOrder(orderId, studentId, studentName, timestamp);

    for (const auto& req : itemRequests) {
        int itemId = req.first;
        int qty = req.second;

        const FoodItem* item = menuManager.getItemById(itemId);
        if (item == nullptr) {
            std::cout << "[ERROR] Item #" << itemId << " does not exist. Order aborted.\n";
            return -1;
        }
        if (!item->getIsAvailable()) {
            std::cout << "[ERROR] Item '" << item->getName() << "' is currently OUT OF STOCK. Order aborted.\n";
            return -1;
        }
        newOrder.addItem(*item, qty);
    }

    int assignedCounterId = -1;
    const ml::IPrepTimePredictor* activePredictor = (useMLPredictor && predictor.isLoaded()) ? &predictor : nullptr;
    bool success = counterManager.enqueueOrder(newOrder, preferredCounterId, &assignedCounterId, activePredictor, timeOfDayHour);
    if (!success) {
        std::cout << "[ERROR] No available counter to accept Order #" << orderId << ".\n";
        return -1;
    }

    newOrder.setAssignedCounterId(assignedCounterId);
    orderManager.recordOrderPlaced(newOrder);

    std::cout << "[SUCCESS] Order #" << orderId << " placed successfully for "
              << studentName << " (" << studentId << "). Assigned to Counter #"
              << assignedCounterId << " (Predicted Prep: " << newOrder.getTotalPreparationTimeMinutes()
              << " mins" << (activePredictor ? " via ML Model" : " via Base Sum") << ").\n";

    return orderId;
}

double CanteenSystem::predictOrderPrepTime(const Order& order, int counterId) const {
    double workload = 0.0;
    if (counterId != -1) {
        const Counter* c = counterManager.getCounterById(counterId);
        if (c != nullptr) {
            workload = static_cast<double>(c->getCurrentWorkloadMinutes());
        }
    }
    if (useMLPredictor && predictor.isLoaded()) {
        auto feats = order.extractPredictionFeatures(timeOfDayHour, workload);
        return predictor.predict(feats);
    }
    return static_cast<double>(order.getTotalPreparationTimeMinutes());
}

bool CanteenSystem::startPreparingOrder(int counterId) {
    bool ok = counterManager.startPreparingFront(counterId);
    if (ok) {
        std::cout << "[STATUS] Front order at Counter #" << counterId << " is now PREPARING.\n";
    } else {
        std::cout << "[WARNING] Counter #" << counterId << " has no pending orders waiting in QUEUED state.\n";
    }
    return ok;
}

bool CanteenSystem::completeAndCollectOrder(int counterId, Order* completedOrderOut) {
    try {
        Order completed = counterManager.completeFrontOrder(counterId);
        completed.transitionTo(OrderStatus::COLLECTED);
        orderManager.recordOrderCompleted(completed);

        std::cout << "[SUCCESS] Order #" << completed.getId() << " marked as READY & COLLECTED from Counter #"
                  << counterId << " by " << completed.getStudentName() << ".\n";

        if (completedOrderOut != nullptr) {
            *completedOrderOut = completed;
        }
        return true;
    } catch (const std::exception& ex) {
        std::cout << "[ERROR] Failed to complete front order: " << ex.what() << "\n";
        return false;
    }
}

bool CanteenSystem::cancelOrder(int orderId) {
    Order cancelledOrder;
    bool removed = counterManager.cancelOrder(orderId, cancelledOrder);
    if (removed) {
        orderManager.recordOrderCancelled(cancelledOrder);
        std::cout << "[SUCCESS] Order #" << orderId << " has been CANCELLED and removed from queue.\n";
        return true;
    }
    std::cout << "[ERROR] Order #" << orderId << " was not found in any pending queue.\n";
    return false;
}

bool CanteenSystem::searchOrderById(int orderId, Order& orderOut) const {
    return orderManager.findOrderById(orderId, orderOut, counterManager);
}

int CanteenSystem::searchFoodItemLinear(int itemId) const {
    return menuManager.linearSearchById(itemId);
}

int CanteenSystem::searchFoodItemBinary(int itemId) const {
    return menuManager.binarySearchById(itemId);
}

void CanteenSystem::sortCountersByWaitTime(bool ascending) {
    counterManager.sortCountersByWaitTime(ascending);
}

void CanteenSystem::sortMenuByPrepTime() {
    menuManager.sortItemsByPrepTime();
}

void CanteenSystem::sortMenuByPrice() {
    menuManager.sortItemsByPrice();
}

bool CanteenSystem::toggleItemAvailability(int itemId, bool available) {
    return menuManager.updateItemAvailability(itemId, available);
}

bool CanteenSystem::undoLastAdminChange(std::string& messageOut) {
    return menuManager.undoLastAction(messageOut);
}

void CanteenSystem::displayDashboard() const {
    std::cout << "\n==============================================================================\n";
    std::cout << "                 AI-BASED COLLEGE CANTEEN QUEUE SYSTEM DASHBOARD             \n";
    std::cout << "==============================================================================\n";
    displayCounters();
    std::cout << "\nMenu summary: " << menuManager.getItemCount() << " items cataloged | "
              << "Total order history count: " << orderManager.getHistoryCount() << " | "
              << "Admin Undo Stack: " << menuManager.getUndoStackSize() << " action(s)\n";
}

void CanteenSystem::displayMenu() const {
    menuManager.displayMenu();
}

void CanteenSystem::displayCounters() const {
    counterManager.displayCounters();
}

void CanteenSystem::displayCounterQueue(int counterId) const {
    counterManager.displayCounterQueue(counterId);
}

void CanteenSystem::displayOrderHistory() const {
    orderManager.displayOrderHistory();
}

void CanteenSystem::runDemonstrationSimulation() {
    std::cout << "\n==============================================================================\n";
    std::cout << "             RUNNING COMPREHENSIVE DSA FOUNDATION DEMONSTRATION               \n";
    std::cout << "==============================================================================\n";

    // 1. Arrays Demonstration
    std::cout << "\n--- [TOPIC 1: ARRAYS] ---\n";
    std::cout << "Managing fixed collection of " << counterManager.getCounterCount() << " counters in FixedArray.\n";
    std::cout << "Indexed access at index 0: " << counterManager.getCounterByIndex(0).getName() << " (O(1))\n";
    std::cout << "Indexed access at index 2: " << counterManager.getCounterByIndex(2).getName() << " (O(1))\n";
    try {
        std::cout << "Attempting invalid access at index 10 (Bounds checking):\n";
        counterManager.getCounterByIndex(10);
    } catch (const std::out_of_range& e) {
        std::cout << "[CAUGHT EXPECTED EXCEPTION]: " << e.what() << "\n";
    }

    // 2. Queues & Linked Lists Demonstration
    std::cout << "\n--- [TOPICS 2 & 3: LINKED LISTS & QUEUES] ---\n";
    std::cout << "Placing 4 sample orders with optimal counter allocation...\n";
    int o1 = placeOrder("STU01", "Aarav Sharma", {{101, 2}, {105, 1}}); // Fast food (prep: 17 mins) -> Counter 1
    int o2 = placeOrder("STU02", "Diya Patel",   {{103, 1}, {106, 1}}); // South Indian (prep: 7 mins) -> Counter 4
    int o3 = placeOrder("STU03", "Rohan Mehta",  {{104, 1}});          // Meals (prep: 10 mins) -> Counter 2
    int o4 = placeOrder("STU04", "Ananya Iyer",  {{107, 2}, {105, 1}}); // Bakery & Drink (prep: 5 mins) -> Counter 3

    std::cout << "\nAll counters after order allocations:\n";
    displayCounters();

    std::cout << "\nPlacing 5th order to test lowest workload allocation...\n";
    // Counter 3 has 5 mins workload (lowest), so order 5 should allocate to Counter 3
    int o5 = placeOrder("STU05", "Kabir Singh", {{108, 1}}); // Samosa (prep: 3 mins)
    std::cout << "Assigned order " << o5 << " to lowest workload counter.\n";

    std::cout << "\n--- [TOPIC 4: STACKS & ADMIN UNDO] ---\n";
    std::cout << "Admin updating availability: Marking Masala Dosa (Item #103) as OUT OF STOCK.\n";
    toggleItemAvailability(103, false);
    const FoodItem* item103 = menuManager.getItemById(103);
    std::cout << "Item #103 status: " << (item103->getIsAvailable() ? "AVAILABLE" : "OUT OF STOCK") << "\n";
    std::cout << "Attempting to order Masala Dosa now (should fail):\n";
    placeOrder("STU06", "Test Student", {{103, 1}});

    std::cout << "\nPerforming Admin Undo via LIFO Stack:\n";
    std::string undoMsg;
    undoLastAdminChange(undoMsg);
    std::cout << undoMsg << "\n";
    std::cout << "Item #103 status after Undo: " << (item103->getIsAvailable() ? "AVAILABLE" : "OUT OF STOCK") << "\n";

    std::cout << "\n--- [TOPIC 5: SEARCHING ALGORITHMS] ---\n";
    std::cout << "1. Linear Search for Order #" << o1 << ":\n";
    Order foundOrder;
    if (searchOrderById(o1, foundOrder)) {
        std::cout << "   Found: " << foundOrder.toString() << "\n";
    }
    std::cout << "2. Linear Search for Food Item by name 'coffee':\n";
    int coffeeIdx = menuManager.linearSearchByName("coffee");
    if (coffeeIdx != -1) {
        std::cout << "   Found at index " << coffeeIdx << ": "
                  << menuManager.getItems()[coffeeIdx].getName() << "\n";
    }
    std::cout << "3. Binary Search for Food Item #105 (Cold Coffee):\n";
    menuManager.sortItemsById(); // Guarantee sorted precondition
    int binIdx = menuManager.binarySearchById(105);
    std::cout << "   Binary Search Result index: " << binIdx << " -> "
              << menuManager.getItems()[binIdx].getName() << " (O(log n))\n";

    std::cout << "\n--- [TOPIC 6: SORTING ALGORITHMS] ---\n";
    std::cout << "1. Sorting counters by estimated wait time using Bubble Sort O(n^2):\n";
    sortCountersByWaitTime(true);
    displayCounters();

    std::cout << "\n2. Sorting menu items by preparation time using Merge Sort O(n log n):\n";
    sortMenuByPrepTime();
    displayMenu();

    std::cout << "\n--- [LIFECYCLE STATE PROGRESSION] ---\n";
    std::cout << "Starting preparation at Counter #3:\n";
    startPreparingOrder(3);
    displayCounterQueue(3);
    std::cout << "Completing order at Counter #3:\n";
    completeAndCollectOrder(3);
    displayCounterQueue(3);

    std::cout << "\n--- [ORDER CANCELLATION] ---\n";
    std::cout << "Cancelling Order #" << o5 << " from queue:\n";
    cancelOrder(o5);

    std::cout << "\n--- [FINAL AUDIT TRAIL] ---\n";
    displayOrderHistory();

    std::cout << "\n==============================================================================\n";
    std::cout << "                 DEMONSTRATION COMPLETED SUCCESSFULLY                        \n";
    std::cout << "==============================================================================\n";
}

void CanteenSystem::runInteractiveCLI() {
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n==============================================================================\n";
        std::cout << "         AI-BASED COLLEGE CANTEEN QUEUE OPTIMIZATION SYSTEM (CLI)             \n";
        std::cout << "==============================================================================\n";
        std::cout << " 1. View Food Menu\n";
        std::cout << " 2. View All Counters & Workloads (Array Traversal)\n";
        std::cout << " 3. Place New Order (Auto-Allocation to Lowest Workload Counter)\n";
        std::cout << " 4. View Selected Counter's Pending Queue (FIFO Queue Inspection)\n";
        std::cout << " 5. Start Preparing Front Order at Counter (QUEUED -> PREPARING)\n";
        std::cout << " 6. Mark Order as Ready & Collected (PREPARING -> READY -> COLLECTED)\n";
        std::cout << " 7. Cancel an Order (Queue Removal & Workload Reclaim)\n";
        std::cout << " 8. Search Order by ID (Linear Search)\n";
        std::cout << " 9. Search Food Item by ID (Binary Search on Sorted Array)\n";
        std::cout << "10. Search Food Item by Name (Linear Search)\n";
        std::cout << "11. Sort Counters by Estimated Wait Time (Bubble Sort O(n^2))\n";
        std::cout << "12. Sort Menu Items by Prep Time or Price (Merge Sort O(n log n))\n";
        std::cout << "13. Admin: Update Menu Item Availability (Push Action to Stack)\n";
        std::cout << "14. Admin: Undo Last Menu Change (Pop Action from Stack)\n";
        std::cout << "15. View Full Order History (Singly Linked List Traversal)\n";
        std::cout << "16. Run Full Automated Demonstration Simulation\n";
        std::cout << "17. View ML Model Status & Test Feature Prediction\n";
        std::cout << "18. Toggle ML Model vs Baseline Sum Mode (Current: " << (useMLPredictor ? "ML ACTIVE" : "BASELINE") << ")\n";
        std::cout << " 0. Exit\n";
        std::cout << "==============================================================================\n";
        std::cout << "Enter your choice [0-18]: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a valid number.\n";
            continue;
        }

        switch (choice) {
            case 1:
                displayMenu();
                break;
            case 2:
                displayCounters();
                break;
            case 3: {
                std::string sId, sName;
                std::cout << "Enter Student ID: ";
                std::cin >> sId;
                std::cout << "Enter Student Name: ";
                std::cin.ignore();
                std::getline(std::cin, sName);

                displayMenu();
                std::cout << "Enter Item ID to order: ";
                int itemId, qty;
                std::cin >> itemId;
                std::cout << "Enter Quantity: ";
                std::cin >> qty;

                placeOrder(sId, sName, {{itemId, qty}});
                break;
            }
            case 4: {
                int cId;
                std::cout << "Enter Counter ID (1-" << counterManager.getCounterCount() << "): ";
                std::cin >> cId;
                displayCounterQueue(cId);
                break;
            }
            case 5: {
                int cId;
                std::cout << "Enter Counter ID to start preparing: ";
                std::cin >> cId;
                startPreparingOrder(cId);
                break;
            }
            case 6: {
                int cId;
                std::cout << "Enter Counter ID to complete front order: ";
                std::cin >> cId;
                completeAndCollectOrder(cId);
                break;
            }
            case 7: {
                int oId;
                std::cout << "Enter Order ID to cancel: ";
                std::cin >> oId;
                cancelOrder(oId);
                break;
            }
            case 8: {
                int oId;
                std::cout << "Enter Order ID to search (Linear Search): ";
                std::cin >> oId;
                Order found;
                if (searchOrderById(oId, found)) {
                    std::cout << "\n[FOUND via Linear Search]:\n";
                    found.display();
                } else {
                    std::cout << "\nOrder #" << oId << " not found.\n";
                }
                break;
            }
            case 9: {
                int itId;
                std::cout << "Enter Food Item ID to search (Binary Search): ";
                std::cin >> itId;
                try {
                    menuManager.sortItemsById();
                    int idx = searchFoodItemBinary(itId);
                    if (idx != -1) {
                        std::cout << "\n[FOUND via Binary Search at index " << idx << "]:\n";
                        menuManager.getItems()[idx].display();
                    } else {
                        std::cout << "\nFood Item #" << itId << " not found in menu.\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "[ERROR]: " << e.what() << "\n";
                }
                break;
            }
            case 10: {
                std::string q;
                std::cout << "Enter Food Item Name to search (Linear Search): ";
                std::cin.ignore();
                std::getline(std::cin, q);
                int idx = menuManager.linearSearchByName(q, false);
                if (idx != -1) {
                    std::cout << "\n[FOUND via Linear Search at index " << idx << "]:\n";
                    menuManager.getItems()[idx].display();
                } else {
                    std::cout << "\nNo food item matching '" << q << "' found.\n";
                }
                break;
            }
            case 11: {
                sortCountersByWaitTime(true);
                std::cout << "\nCounters sorted by estimated waiting time (Bubble Sort):\n";
                displayCounters();
                break;
            }
            case 12: {
                int sub;
                std::cout << "Sort menu by: 1. Prep Time  2. Price: ";
                std::cin >> sub;
                if (sub == 1) {
                    sortMenuByPrepTime();
                    std::cout << "\nMenu sorted by preparation time (Merge Sort):\n";
                } else {
                    sortMenuByPrice();
                    std::cout << "\nMenu sorted by price (Merge Sort):\n";
                }
                displayMenu();
                break;
            }
            case 13: {
                int itId, avail;
                std::cout << "Enter Food Item ID: ";
                std::cin >> itId;
                std::cout << "Set status: 1. Available  0. Out of Stock: ";
                std::cin >> avail;
                if (toggleItemAvailability(itId, avail == 1)) {
                    std::cout << "Item status updated. Action pushed to Undo Stack.\n";
                } else {
                    std::cout << "Item not found.\n";
                }
                break;
            }
            case 14: {
                std::string msg;
                undoLastAdminChange(msg);
                std::cout << msg << "\n";
                break;
            }
            case 15:
                displayOrderHistory();
                break;
            case 16:
                runDemonstrationSimulation();
                break;
            case 17: {
                std::cout << "\n============================== ML MODEL STATUS ==============================\n";
                std::cout << "Model Status     : " << (predictor.isLoaded() ? "LOADED & OPERATIONAL" : "FALLBACK") << "\n";
                std::cout << "Model Source     : " << predictor.getModelName() << "\n";
                std::cout << "Ensemble Trees   : " << predictor.getEstimatorCount() << "\n";
                std::cout << "Time of Day      : " << timeOfDayHour << " hrs\n";
                std::cout << "Scheduler Mode   : " << (useMLPredictor ? "ML RANDOM FOREST PREDICTOR" : "STATIC MENU BASELINE") << "\n";
                std::cout << "------------------------------------------------------------------------------\n";
                std::cout << "Test Custom Feature Vector:\n";
                int cat, qty, comp;
                double wl;
                std::cout << "Enter Category (0: FastFood, 1: SouthIndian, 2: Meals, 3: Beverage, 4: Bakery): ";
                std::cin >> cat;
                std::cout << "Enter Quantity (1-5): ";
                std::cin >> qty;
                std::cout << "Enter Complexity (1-5): ";
                std::cin >> comp;
                std::cout << "Enter Counter Workload (minutes): ";
                std::cin >> wl;

                ml::OrderPredictionFeatures f;
                f.foodCategory = cat;
                f.orderQuantity = qty;
                f.preparationComplexity = comp;
                f.timeOfDayHour = timeOfDayHour;
                f.currentKitchenWorkload = wl;

                double pred = predictor.predict(f);
                std::cout << "[ML INFERENCE RESULT]: Predicted Preparation Time = "
                          << std::fixed << std::setprecision(2) << pred << " minutes\n";
                std::cout << "==============================================================================\n";
                break;
            }
            case 18: {
                useMLPredictor = !useMLPredictor;
                std::cout << "[CONFIG UPDATED]: Scheduler prediction mode changed to: "
                          << (useMLPredictor ? "ML RANDOM FOREST PREDICTOR" : "STATIC MENU BASELINE") << "\n";
                break;
            }
            case 0:
                std::cout << "Exiting College Canteen Queue Optimization System. Goodbye!\n";
                break;
            default:
                std::cout << "Invalid choice. Please select from 0 to 18.\n";
                break;
        }
    }
}

} // namespace canteen
