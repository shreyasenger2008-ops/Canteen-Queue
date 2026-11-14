# Testing Report: DSA, Machine Learning, Database & REST API Test Suite

This report documents the verification results for the **AI-Based College Canteen Queue Optimization System** across all phases: DSA Foundation, Machine Learning Predictor, SQLite Persistence, and C++ REST API. All 68 unit and integration tests were compiled and executed against the C++ test suite binary.

---

## 1. Test Execution Metadata

- **Date of Execution**: 2026-11-02
- **Compiler**: Microsoft C/C++ Optimizing Compiler Version 19.51.36260 for x64
- **C++ Standard**: C++17 (`/std:c++17`)
- **Build System**: CMake 4.3.1-msvc1 / MSBuild 18.10.1
- **Platform**: Windows 11 (x64)
- **Binary Executed**: `build\Debug\canteen_tests.exe`
- **Total Backend Tests Executed**: 68
- **Backend Tests Passed**: 68
- **Backend Tests Failed**: 0
- **Pass Rate**: 100.0%
- **Browser E2E Automated Tests**: 10
- **Browser E2E Passed**: 10 (100.0%)

---

## 2. Test Cases Breakdown

### Category 1: Fixed Array Tests (`tests/test_array.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 1 | `Array_ValidIndexingAndTraversal` | Add 3 integers (10, 20, 30), update index 1 to 99, traverse to compute sum. | Accesses match elements; sum equals 139; size equals 3. | Matched expected values. | **PASS** |
| 2 | `Array_OutOfBoundsProtection` | Access indices 2, 3, 999 on an array with 2 active elements. | `at()` and `update()` throw `std::out_of_range`. | `std::out_of_range` caught as expected. | **PASS** |
| 3 | `Array_CapacityOverflow` | Attempt to add a 3rd element to `FixedArray<T, 2>`. | Throws `std::overflow_error`. | `std::overflow_error` caught. | **PASS** |
| 4 | `Array_CounterRepresentationAndAvailability` | Create 3 counters with active and offline states; toggle availability. | State flags update accurately without side effects. | Verified state changes. | **PASS** |
| 5 | `Array_RemoveAtAndShift` | Remove element at index 1 from `[1, 2, 3, 4]`. | Trailing elements shifted left; resulting array is `[1, 3, 4]`; size is 3. | Elements shifted correctly. | **PASS** |

---

### Category 2: Singly Linked List Tests (`tests/test_linked_list.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 6 | `LinkedList_EmptyListHandling` | Call `front()`, `back()`, `deleteFront()`, `deleteBy()` on empty list. | Throws `std::underflow_error`; `deleteBy()` returns `false`. | Handled cleanly with exceptions/false. | **PASS** |
| 7 | `LinkedList_SingleNodeOperations` | Insert 1 element (42), inspect front and back, delete front. | Front and back equal 42; after deletion, list becomes empty. | Size transitions 0 -> 1 -> 0. | **PASS** |
| 8 | `LinkedList_MultipleNodesInsertFrontAndBack` | `insertBack(20)`, `insertFront(10)`, `insertBack(30)`. | List order: 10 -> 20 -> 30; front is 10; back is 30. | Correct pointer links maintained. | **PASS** |
| 9 | `LinkedList_DeleteFirstMiddleLastNode` | Sequence of deletions on `[10, 20, 30, 40, 50]`: delete 30 (middle), 10 (head), 50 (tail), and 999 (absent). | Correct node unlinked; head/tail pointers updated; absent returns `false`. | All cases handled correctly. | **PASS** |
| 10 | `LinkedList_SearchPresentAndAbsent` | Search for "Banana" (present) and "Mango" (absent) in `["Apple", "Banana", "Cherry"]`. | Returns pointer to "Banana"; returns `nullptr` for "Mango". | Pointer matches element or `nullptr`. | **PASS** |
| 11 | `LinkedList_DeepCopyAndMoveRuleOfFive` | Copy construct, copy assign, move construct, and move assign. | Deep copies are independent; move transfers ownership without memory leaks. | Verified independence and clean transfers. | **PASS** |

---

### Category 3: FIFO Queue Tests (`tests/test_queue.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 12 | `Queue_FIFOOrdering` | Enqueue 101, 102, 103; dequeue sequentially. | Elements returned in exact order: 101, 102, 103. | Strict FIFO order verified. | **PASS** |
| 13 | `Queue_EmptyQueueHandling` | Dequeue or inspect `front()`/`rear()` on empty queue. | Throws `std::underflow_error`. | Threw expected underflow error. | **PASS** |
| 14 | `Queue_RepeatedEnqueueDequeueCycles` | 50 consecutive cycles of `enqueue(x)` and `dequeue()`. | Queue empties cleanly after each cycle; no memory leak or pointer corruption. | Completed all 50 cycles cleanly. | **PASS** |
| 15 | `Queue_FrontAndRearUpdates` | Enqueue A, B, C; inspect front and rear after each enqueue and dequeue. | Front remains A until dequeue; rear updates to latest enqueued element. | Verified front/rear pointers. | **PASS** |
| 16 | `Queue_RemoveIfOrderCancellation` | Enqueue 1, 2, 3, 4; cancel (remove) order 2. | Remaining elements 1, 3, 4 maintain original FIFO arrival sequence. | Order 2 removed, relative sequence preserved. | **PASS** |

---

### Category 4: LIFO Stack Tests (`tests/test_stack.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 17 | `Stack_LIFOBehavior` | Push 10, 20, 30; pop sequentially. | Elements popped in reverse: 30, 20, 10. | LIFO order confirmed. | **PASS** |
| 18 | `Stack_EmptyStackHandling` | Pop or call `top()` on empty stack. | Throws `std::underflow_error`. | Underflow error thrown. | **PASS** |
| 19 | `Stack_DeepCopyAndOrderPreservation` | Copy construct stack; verify operations on clone do not affect original. | Clones are isolated; popping from clone leaves original intact. | Deep copy validated. | **PASS** |
| 20 | `Stack_UndoRestoresPreviousMenuAvailability` | Push admin action (item availability toggle); pop to undo. | Item availability restored to initial value. | Undo action restores previous state. | **PASS** |

---

### Category 5: Searching Tests (`tests/test_searching.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 21 | `Searching_LinearSearchOrderById` | Linear search in singly linked list for present and absent IDs. | Returns pointer to target or `nullptr`. | Matched target order. | **PASS** |
| 22 | `Searching_LinearSearchFoodItemByIdAndName` | Search for food items by ID and substring in name. | Found items returned accurately. | Linear search verified. | **PASS** |
| 23 | `Searching_BinarySearchOnSortedData` | Binary search on sorted array of food items by ID. | Returns correct index for targets; returns -1 for absent items. | $O(\log n)$ lookup confirmed. | **PASS** |
| 24 | `Searching_BinarySearchRequiresSortedValidation` | Pass unsorted array to binary search with validation check. | Throws `std::invalid_argument`. | Precondition check verified. | **PASS** |

---

### Category 6: Sorting Tests (`tests/test_sorting.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 25 | `Sorting_BubbleSortCountersByWaitTime` | Sort counters with workloads `[15, 5, 20, 10]`. | Sorted ascending: `[5, 10, 15, 20]`. | Counters sorted accurately. | **PASS** |
| 26 | `Sorting_BubbleSortTieBreakingOnId` | Sort counters with identical workloads `[10, 10, 10]`, IDs `[3, 1, 2]`. | Sorted by ID: `[1, 2, 3]`. | Stable tie-breaking verified. | **PASS** |
| 27 | `Sorting_MergeSortEmptyAndSingleElement` | Sort empty vector and 1-element vector. | Operates cleanly without crash or infinite recursion. | Handled base cases. | **PASS** |
| 28 | `Sorting_MergeSortFoodItemsByPrepTimeAndDuplicates` | Sort items with duplicate prep times: `[10, 5, 7, 5, 2]`. | Sorted: `[2, 5, 5, 7, 10]`; original relative order of 5s preserved. | Stable merge sort verified. | **PASS** |
| 29 | `Sorting_MergeSortOrdersByPrepTime` | Sort historical orders by total prep duration. | Orders sorted in non-decreasing order of duration. | Order history sorted. | **PASS** |

---

### Category 7: Integration Tests (`tests/test_integration.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 30 | `Integration_OrderPlacementAndAllocation` | Place 2 orders through `CanteenSystem`. | Orders assigned to eligible counters; order IDs auto-incremented. | Assigned to Counter 1 and 2. | **PASS** |
| 31 | `Integration_ValidAndInvalidStatusTransitions` | Test transitions: QUEUED -> PREPARING -> READY -> COLLECTED; test illegal skip. | Valid transitions succeed; illegal transitions throw `invalid_argument`. | State transitions enforced. | **PASS** |
| 32 | `Integration_CompleteOrderWorkflowAndWorkloadUpdate` | Place order, start preparation, complete order. | Workload decremented back to 0; counter served count increments. | Workload updated correctly. | **PASS** |
| 33 | `Integration_CancellationReclaimsWorkloadAndClearsQueue` | Place 2 orders, cancel first order. | Workload decremented; cancelled order removed from counter queue. | Queue and workload updated. | **PASS** |

---

### Category 8: Machine Learning Tests (`tests/test_ml.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 34 | `ML_ModelLoadSuccess` | Load `ml/model.json` into `RandomForestPredictor`. | `isLoaded() == true`; tree count equals 20; feature count equals 5. | Loaded 20 decision trees. | **PASS** |
| 35 | `ML_FinitePositivePredictions` | Test features across various item types and workloads. | Predictions are finite numbers $> 0.0$. | Valid durations returned. | **PASS** |
| 36 | `ML_CompareWithPythonHeldOutPredictions` | Test 5 held-out samples against predictions generated by Python `evaluate_model.py`. | C++ predictions match Python predictions to $< 10^{-5}$ tolerance. | Difference $< 10^{-6}$. | **PASS** |
| 37 | `ML_SafeInputValidationAndErrorHandling` | Pass negative workloads or invalid features. | Predictor clamps values cleanly to valid operational bounds. | Safely handled. | **PASS** |
| 38 | `ML_MissingModelFileFallbackSafety` | Instantiate predictor pointing to non-existent file. | Returns heuristic base sum without throwing unhandled exception. | Safe fallback confirmed. | **PASS** |
| 39 | `ML_PredictionsReachSchedulingEngine` | Place order with ML enabled; inspect counter workload increment. | Workload increment matches model prediction. | Counter received ML workload. | **PASS** |
| 40 | `ML_PreservesFIFOOrderingUnderPredictedAllocations` | Place 3 orders with varying predicted durations to same counter. | Orders served in exact arrival sequence regardless of duration. | Strict FIFO preserved. | **PASS** |

---

### Category 9: SQLite Database Tests (`tests/test_database.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 41 | `DB_InitializeAndSchemaCreation` | Initialize SQLite on temp database; create schema. | All tables and foreign keys provisioned; `isOpen() == true`. | Schema created. | **PASS** |
| 42 | `DB_SyncAndLoadMenuItems` | Synchronize items, update availability and prices, reload. | Values updated and persisted across connections. | Loaded exact records. | **PASS** |
| 43 | `DB_SaveAndLoadOrderWithItems` | Insert order with 2 line items; query `loadOrderById()`. | Order and line items match quantities, prices, and totals. | Loaded order matches. | **PASS** |
| 44 | `DB_IdempotencyKeyDeduplication` | Insert order with idempotency key; attempt duplicate insert. | Key lookup returns existing order; duplicate insert fails constraint. | UNIQUE constraint verified. | **PASS** |
| 45 | `DB_OrderStatusTransitionsAndActiveFilter` | Transition orders through QUEUED, PREPARING, COLLECTED, CANCELLED. | `loadActiveOrders()` returns only QUEUED and PREPARING orders. | Active orders filtered. | **PASS** |
| 46 | `DB_AdminAuditLogging` | Insert audit log record for menu availability modification. | Record stored successfully in `admin_audit_log`. | Audit log saved. | **PASS** |
| 47 | `DB_SystemRecoveryOnRestart` | Place orders, simulate restart with new `CanteenSystem` instance. | Active orders reconstructed into counter queues; completed excluded. | State reconstructed. | **PASS** |
| 48 | `DB_LoadOrdersPaginatedAndFilter` | Query `loadOrdersPaginated()` with limit, offset, and status filter (`QUEUED`). | Returns exact slice matching pagination; total count reflects filter; orders have full item lines. | Parameterized query verified. | **PASS** |
| 49 | `DB_PaymentStatusPersistence` | Save order with default PENDING status, update to PAID, reload via `loadOrderById()`. | Payment status transitions to PAID and is returned upon reload. | Payment status persisted. | **PASS** |
| 50 | `DB_PaymentStatusSurvivesRestart` | Update order to PAID, close database connection, re-initialize new `DatabaseManager` instance. | Re-initialized database loads order with `PAID` status intact. | Survives database restart. | **PASS** |

---

### Category 10: REST API Integration Tests (`tests/test_api.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 51 | `API_HealthEndpoint` | `GET /api/health` via `httplib::Client`. | Returns HTTP 200, status "healthy", db/ml connected flags. | HTTP 200 verified. | **PASS** |
| 52 | `API_MenuEndpoint` | `GET /api/menu`. | Returns HTTP 200, complete menu items array. | Non-empty menu returned. | **PASS** |
| 53 | `API_CountersEndpoint` | `GET /api/counters`. | Returns HTTP 200, all 4 counters with live workloads. | 4 counters returned. | **PASS** |
| 54 | `API_PlaceOrderValidAndLookup` | `POST /api/orders` with valid JSON; `GET /api/orders/{id}`. | Returns HTTP 201 Created; lookup returns HTTP 200. | Order placed & fetched. | **PASS** |
| 55 | `API_PlaceOrderIdempotencyKey` | Send same order request twice with same `idempotency_key`. | First returns HTTP 201; second returns HTTP 200 with same order ID. | Idempotency verified. | **PASS** |
| 56 | `API_PlaceOrderValidationFailures` | Test empty student, missing items, zero quantity, non-existent item. | Each returns HTTP 400 Bad Request with descriptive error. | HTTP 400 on all bad payloads. | **PASS** |
| 57 | `API_OrderLifecycleProgression` | Call `/start` -> `/ready` -> `/collect` on placed order with staff Bearer token. | Transitions proceed 200 OK; invalid jumps return 409 Conflict. | Lifecycle enforced. | **PASS** |
| 58 | `API_OrderCancellationInQueue` | Place order, call `POST /api/orders/{id}/cancel` with staff Bearer token. | Returns HTTP 200; counter workload reclaimed to 0. | Workload reclaimed. | **PASS** |
| 59 | `API_AnalyticsAndCorsHeaders` | `GET /api/analytics`; check headers and `OPTIONS /api/menu`. | HTTP 200 with metrics; CORS headers present on all responses. | CORS & metrics confirmed. | **PASS** |
| 60 | `API_GetOrdersPaginatedAndFilter` | `GET /api/orders?limit=2&offset=1&status=QUEUED`; invalid params validation. | Returns HTTP 200 with paginated orders slice; invalid limit/offset/status return HTTP 400. | Query params validated & paginated. | **PASS** |
| 61 | `API_StaffAuth_ValidPin` | `POST /api/staff/auth` with valid PIN ("1234"). | Returns HTTP 200 with server Bearer token. | Token issued successfully. | **PASS** |
| 62 | `API_StaffAuth_InvalidPin` | `POST /api/staff/auth` with invalid PIN ("wrong"). | Returns HTTP 401 Unauthorized with error message. | Unauthorized rejected. | **PASS** |
| 63 | `API_StaffProtected_NoToken` | Call `POST /api/orders/{id}/start` without `Authorization` header. | Returns HTTP 401 Unauthorized; order status remains QUEUED. | Protected endpoint guarded. | **PASS** |
| 64 | `API_StaffProtected_WithToken` | Call `POST /api/orders/{id}/start` with valid Bearer token header. | Returns HTTP 200 OK; order status updates to PREPARING. | Authorized request succeeds. | **PASS** |
| 65 | `API_PaymentPersistence` | Place order (PENDING), call `POST /api/orders/{id}/pay` with Bearer token. | Returns HTTP 200 with `payment_status: PAID`; GET order confirms PAID. | Payment persisted. | **PASS** |
| 66 | `API_PaymentCancelledOrder` | Place and cancel order, attempt `POST /api/orders/{id}/pay` with Bearer token. | Returns HTTP 409 Conflict; payment rejected on cancelled order. | Conflict enforced. | **PASS** |
| 67 | `API_AcceptOrder_Valid` | Place order, call `POST /api/orders/{id}/accept` with Bearer token. | Returns HTTP 200, status QUEUED preserved, `acknowledged: true`. | Honest acknowledgement logged. | **PASS** |
| 68 | `API_AcceptOrder_NoToken` | Call `POST /api/orders/{id}/accept` without token. | Returns HTTP 401 Unauthorized. | Protected from unauthenticated access. | **PASS** |

---

### Category 11: End-to-End Browser QA Automation (`scratch/qa_browser_e2e.mjs`)

Executed in headless Google Chrome via Chrome DevTools Protocol against the live C++ REST API (`localhost:8080`) and Vite production preview (`localhost:5173`).

| Flow | Flow Description | Verification Mechanism | Observed Result | Status |
|---|---|---|---|---|
| Flow 1 | Dashboard loads menu items from backend API | Query DOM for rendered food cards, prices, and categories | 8 menu items loaded dynamically with accurate prices | **PASS** |
| Flow 2 | Search and category filters work properly | Click "Beverages" category pill; enter "Burger" in search | Isolated 2 beverages; search isolated "Veg Cheese Burger" | **PASS** |
| Flow 3 | Cart quantities and total price update accurately | Increment item count to 2; verify drawer total | Cart updated to 2 items; live total calculation ₹75.00 | **PASS** |
| Flow 4 | Invalid checkout inputs rejected with clear messages | Submit empty checkout form; check constraint validation | HTML5 validation enforced: Student ID required | **PASS** |
| Flow 5 | Valid order placement produces real confirmation | Submit valid student credentials; inspect confirmation modal | Order confirmed, assigned to counter, prep time displayed | **PASS** |
| Flow 6 | Order tracking displays actual backend status | Navigate to Track Order; inspect status and stepper | Order tracked live: Status=QUEUED, steps active | **PASS** |
| Flow 7 | Queue dashboard reflects real active counts & refreshes | Open Live Queues; inspect 4 counters and workload metrics | 4 counters rendered with workload telemetry; refresh successful | **PASS** |
| Flow 8 | Admin dashboard lists persisted orders from SQLite | Open Admin Ops; inspect `GET /api/orders` table and filter | Table loaded orders; filter pills active; pagination verified | **PASS** |
| Flow 9 | Valid lifecycle actions update status; invalid actions show errors | Transition order: QUEUED -> PREPARING -> READY -> COLLECTED | Lifecycle completed; re-start rejected with HTTP 409 Conflict | **PASS** |
| Flow 10 | Mobile and desktop layouts remain usable | Emulate iPhone (375x812) and Desktop (1280x800) | Mobile layout wraps cleanly without overflow; desktop contained | **PASS** |

---

## 3. Raw Test Runner Output

```text
==============================================================================
               RUNNING CANTEEN DSA FOUNDATION TEST SUITE                      
==============================================================================
  [PASS] Array_ValidIndexingAndTraversal
  [PASS] Array_OutOfBoundsProtection
  [PASS] Array_CapacityOverflow
  [PASS] Array_CounterRepresentationAndAvailability
  [PASS] Array_RemoveAtAndShift
  [PASS] LinkedList_EmptyListHandling
  [PASS] LinkedList_SingleNodeOperations
  [PASS] LinkedList_MultipleNodesInsertFrontAndBack
  [PASS] LinkedList_DeleteFirstMiddleLastNode
  [PASS] LinkedList_SearchPresentAndAbsent
  [PASS] LinkedList_DeepCopyAndMoveRuleOfFive
  [PASS] Queue_FIFOOrdering
  [PASS] Queue_EmptyQueueHandling
  [PASS] Queue_RepeatedEnqueueDequeueCycles
  [PASS] Queue_FrontAndRearUpdates
  [PASS] Queue_RemoveIfOrderCancellation
  [PASS] Stack_LIFOBehavior
  [PASS] Stack_EmptyStackHandling
  [PASS] Stack_DeepCopyAndOrderPreservation
  [PASS] Stack_UndoRestoresPreviousMenuAvailability
  [PASS] Searching_LinearSearchOrderById
  [PASS] Searching_LinearSearchFoodItemByIdAndName
  [PASS] Searching_BinarySearchOnSortedData
  [PASS] Searching_BinarySearchRequiresSortedValidation
  [PASS] Sorting_BubbleSortCountersByWaitTime
  [PASS] Sorting_BubbleSortTieBreakingOnId
  [PASS] Sorting_MergeSortEmptyAndSingleElement
  [PASS] Sorting_MergeSortFoodItemsByPrepTimeAndDuplicates
  [PASS] Sorting_MergeSortOrdersByPrepTime
[SUCCESS] Order #101 placed successfully for Student One (S101). Assigned to Counter #1 (Predicted Prep: 7 mins via ML Model).
[SUCCESS] Order #102 placed successfully for Student Two (S102). Assigned to Counter #2 (Predicted Prep: 3 mins via ML Model).
  [PASS] Integration_OrderPlacementAndAllocation
  [PASS] Integration_ValidAndInvalidStatusTransitions
[SUCCESS] Order #101 placed successfully for User1 (S1). Assigned to Counter #1 (Predicted Prep: 7 mins via Base Sum).
[STATUS] Front order at Counter #1 is now PREPARING.
[SUCCESS] Order #101 marked as READY & COLLECTED from Counter #1 by User1.
  [PASS] Integration_CompleteOrderWorkflowAndWorkloadUpdate
[SUCCESS] Order #101 placed successfully for User1 (S1). Assigned to Counter #1 (Predicted Prep: 7 mins via Base Sum).
[SUCCESS] Order #102 placed successfully for User2 (S2). Assigned to Counter #1 (Predicted Prep: 8 mins via Base Sum).
[SUCCESS] Order #101 has been CANCELLED and removed from queue.
[ERROR] Order #101 was not found in any pending queue.
  [PASS] Integration_CancellationReclaimsWorkloadAndClearsQueue
  [PASS] ML_ModelLoadSuccess
  [PASS] ML_FinitePositivePredictions
  [PASS] ML_CompareWithPythonHeldOutPredictions
  [PASS] ML_SafeInputValidationAndErrorHandling
  [PASS] ML_MissingModelFileFallbackSafety
[SUCCESS] Order #101 placed successfully for ML Test Student (STU99). Assigned to Counter #1 (Predicted Prep: 7 mins via ML Model).
  [PASS] ML_PredictionsReachSchedulingEngine
[SUCCESS] Order #101 placed successfully for Alice (STU_A). Assigned to Counter #1 (Predicted Prep: 7 mins via ML Model).
[SUCCESS] Order #102 placed successfully for Bob (STU_B). Assigned to Counter #1 (Predicted Prep: 3 mins via ML Model).
[SUCCESS] Order #103 placed successfully for Carol (STU_C). Assigned to Counter #1 (Predicted Prep: 3 mins via ML Model).
  [PASS] ML_PreservesFIFOOrderingUnderPredictedAllocations
  [PASS] DB_InitializeAndSchemaCreation
  [PASS] DB_SyncAndLoadMenuItems
  [PASS] DB_SaveAndLoadOrderWithItems
  [PASS] DB_IdempotencyKeyDeduplication
  [PASS] DB_OrderStatusTransitionsAndActiveFilter
  [PASS] DB_AdminAuditLogging
[SUCCESS] Order #101 placed successfully for Alice (S1). Assigned to Counter #1 (Predicted Prep: 7 mins via Base Sum).
[SUCCESS] Order #102 placed successfully for Bob (S2). Assigned to Counter #1 (Predicted Prep: 8 mins via Base Sum).
[SUCCESS] Order #103 placed successfully for Charlie (S3). Assigned to Counter #3 (Predicted Prep: 3 mins via Base Sum).
[STATUS] Front order at Counter #1 is now PREPARING.
[SUCCESS] Order #103 marked as READY & COLLECTED from Counter #3 by Charlie.
  [PASS] DB_SystemRecoveryOnRestart
  [PASS] DB_LoadOrdersPaginatedAndFilter
  [PASS] DB_PaymentStatusPersistence
  [PASS] DB_PaymentStatusSurvivesRestart
[REST API SERVER] Listening on http://127.0.0.1:18089
[REST API SERVER] Stopped.
  [PASS] API_HealthEndpoint
[REST API SERVER] Listening on http://127.0.0.1:18089
[REST API SERVER] Stopped.
  [PASS] API_MenuEndpoint
[REST API SERVER] Listening on http://127.0.0.1:18089
[REST API SERVER] Stopped.
  [PASS] API_CountersEndpoint
[REST API SERVER] Listening on http://127.0.0.1:18089
[SUCCESS] Order #101 placed successfully for Rahul Sharma (STU101). Assigned to Counter #1 (Predicted Prep: 17 mins via Base Sum).
[REST API SERVER] Stopped.
  [PASS] API_PlaceOrderValidAndLookup
[REST API SERVER] Listening on http://127.0.0.1:18089
[SUCCESS] Order #101 placed successfully for Priya Patel (STU202). Assigned to Counter #1 (Predicted Prep: 3 mins via Base Sum).
[REST API SERVER] Stopped.
  [PASS] API_PlaceOrderIdempotencyKey
[REST API SERVER] Listening on http://127.0.0.1:18089
[REST API SERVER] Stopped.
  [PASS] API_PlaceOrderValidationFailures
[REST API SERVER] Listening on http://127.0.0.1:18089
[SUCCESS] Order #101 placed successfully for Vikram Singh (STU303). Assigned to Counter #2 (Predicted Prep: 8 mins via Base Sum).
[STATUS] Front order at Counter #2 is now PREPARING.
[REST API SERVER] Stopped.
  [PASS] API_OrderLifecycleProgression
[REST API SERVER] Listening on http://127.0.0.1:18089
[SUCCESS] Order #101 placed successfully for Ananya (STU404). Assigned to Counter #1 (Predicted Prep: 7 mins via Base Sum).
[SUCCESS] Order #101 has been CANCELLED and removed from queue.
[REST API SERVER] Stopped.
  [PASS] API_OrderCancellationInQueue
[REST API SERVER] Listening on http://127.0.0.1:18089
[REST API SERVER] Stopped.
  [PASS] API_AnalyticsAndCorsHeaders
[REST API SERVER] Listening on http://127.0.0.1:18089
[SUCCESS] Order #101 placed successfully for Student 1 (STU_1). Assigned to Counter #1 (Predicted Prep: 7 mins via Base Sum).
[SUCCESS] Order #102 placed successfully for Student 2 (STU_2). Assigned to Counter #2 (Predicted Prep: 7 mins via Base Sum).
[SUCCESS] Order #103 placed successfully for Student 3 (STU_3). Assigned to Counter #3 (Predicted Prep: 7 mins via Base Sum).
[REST API SERVER] Stopped.
  [PASS] API_GetOrdersPaginatedAndFilter
[REST API SERVER] Listening on http://127.0.0.1:18089
[REST API SERVER] Stopped.
  [PASS] API_StaffAuth_ValidPin
[REST API SERVER] Listening on http://127.0.0.1:18089
[REST API SERVER] Stopped.
  [PASS] API_StaffAuth_InvalidPin
[REST API SERVER] Listening on http://127.0.0.1:18089
[SUCCESS] Order #101 placed successfully for Test (STU99). Assigned to Counter #1 (Predicted Prep: 7 mins via Base Sum).
[REST API SERVER] Stopped.
  [PASS] API_StaffProtected_NoToken
[REST API SERVER] Listening on http://127.0.0.1:18089
[SUCCESS] Order #101 placed successfully for Authorized (STU88). Assigned to Counter #1 (Predicted Prep: 7 mins via Base Sum).
[STATUS] Front order at Counter #1 is now PREPARING.
[REST API SERVER] Stopped.
  [PASS] API_StaffProtected_WithToken
[REST API SERVER] Listening on http://127.0.0.1:18089
[SUCCESS] Order #101 placed successfully for Pay Test (STU77). Assigned to Counter #1 (Predicted Prep: 7 mins via Base Sum).
[REST API SERVER] Stopped.
  [PASS] API_PaymentPersistence
[REST API SERVER] Listening on http://127.0.0.1:18089
[SUCCESS] Order #101 placed successfully for Cancel Test (STU66). Assigned to Counter #1 (Predicted Prep: 7 mins via Base Sum).
[SUCCESS] Order #101 has been CANCELLED and removed from queue.
[REST API SERVER] Stopped.
  [PASS] API_PaymentCancelledOrder
[REST API SERVER] Listening on http://127.0.0.1:18089
[SUCCESS] Order #101 placed successfully for Accept Test (STU55). Assigned to Counter #1 (Predicted Prep: 7 mins via Base Sum).
[REST API SERVER] Stopped.
  [PASS] API_AcceptOrder_Valid
[REST API SERVER] Listening on http://127.0.0.1:18089
[SUCCESS] Order #101 placed successfully for No Auth (STU44). Assigned to Counter #1 (Predicted Prep: 7 mins via Base Sum).
[REST API SERVER] Stopped.
  [PASS] API_AcceptOrder_NoToken
==============================================================================
TEST SUMMARY: 68 PASSED, 0 FAILED (Total: 68)
ALL TESTS PASSED SUCCESSFULLY!
==============================================================================
```

```
