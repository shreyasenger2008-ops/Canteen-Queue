# Testing Report: DSA & Machine Learning Integration Test Suite

This report documents the verification results for the **AI-Based College Canteen Queue Optimization System** (Phase 2: ML Integration). All 40 unit and integration tests were compiled and executed against the C++ test suite binary.

---

## 1. Test Execution Metadata

- **Date of Execution**: 2026-10-25
- **Compiler**: Microsoft C/C++ Optimizing Compiler Version 19.51.36260 for x64
- **C++ Standard**: C++17 (`/std:c++17`)
- **Build System**: CMake 4.3.1-msvc1 / MSBuild 18.10.1
- **Platform**: Windows 11 (x64)
- **Binary Executed**: `build\Debug\canteen_tests.exe`
- **Total Tests Executed**: 40
- **Passed**: 40
- **Failed**: 0
- **Pass Rate**: 100.0%

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

### Category 4: Stack Tests (`tests/test_stack.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 17 | `Stack_LIFOBehavior` | Push 10, 20, 30; pop sequentially. | Elements returned in reverse order: 30, 20, 10. | Strict LIFO order verified. | **PASS** |
| 18 | `Stack_EmptyStackHandling` | Pop or peek into empty stack. | Throws `std::underflow_error`. | Underflow error thrown. | **PASS** |
| 19 | `Stack_DeepCopyAndOrderPreservation` | Copy stack containing 100, 200, 300. | Copy pops 300, 200, 100; original remains intact. | LIFO order maintained in copy. | **PASS** |
| 20 | `Stack_UndoRestoresPreviousMenuAvailability` | Change item #1 availability from `true` to `false`; invoke undo. | State restored to `true`; undo stack size returns to 0. | Availability restored cleanly. | **PASS** |

---

### Category 5: Searching Tests (`tests/test_searching.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 21 | `Searching_LinearSearchOrderById` | Search for order IDs 101 (first), 103 (middle), 104 (last), and 999 (absent). | Returns indices 0, 2, 3, and -1 respectively. | Indices matched exactly. | **PASS** |
| 22 | `Searching_LinearSearchFoodItemByIdAndName` | Search food item by ID and case-insensitive substring name ("chai", "WRAP", "Burger", "Pizza"). | Returns correct indices; -1 for "Pizza". | All queries resolved accurately. | **PASS** |
| 23 | `Searching_BinarySearchOnSortedData` | Search for IDs 10 (first), 30 (mid), 50 (last), and missing IDs (5, 25, 60) in sorted array. | Returns indices 0, 2, 4, and -1 for missing values. | Verified $O(\log n)$ search results. | **PASS** |
| 24 | `Searching_BinarySearchRequiresSortedValidation` | Run binary search on unsorted food item array `[30, 10, 20]`. | `isSortedByFoodItemId` returns `false`; binary search throws `std::invalid_argument`. | Exception thrown as required. | **PASS** |

---

### Category 6: Sorting Tests (`tests/test_sorting.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 25 | `Sorting_BubbleSortCountersByWaitTime` | Counters with wait times 15m (C1), 5m (C2), 10m (C3). | Sorted ascending: C2 (5m), C3 (10m), C1 (15m). | In-place sort verified. | **PASS** |
| 26 | `Sorting_BubbleSortTieBreakingOnId` | Three counters with equal wait time (0m) and IDs [5, 2, 4]. | Sorted ascending by ID: C2, C4, C5. | Tie-break rule verified. | **PASS** |
| 27 | `Sorting_MergeSortEmptyAndSingleElement` | Merge sort empty array and 1-element array. | Does not crash; array remains size 0 or 1. | Handled boundary cases cleanly. | **PASS** |
| 28 | `Sorting_MergeSortFoodItemsByPrepTimeAndDuplicates` | Items with prep times: `[10, 3, 7, 3, 1]`. | Sorted: `[1, 3, 3, 7, 10]`; stable relative order for duplicates. | Stability and order confirmed. | **PASS** |
| 29 | `Sorting_MergeSortOrdersByPrepTime` | Orders with prep times 20m, 5m, 12m. | Sorted: Order 102 (5m), Order 103 (12m), Order 101 (20m). | Sorted accurately. | **PASS** |

---

### Category 7: System Integration Tests (`tests/test_integration.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 30 | `Integration_OrderPlacementAndAllocation` | Place 2 orders when counters are initially empty under ML scheduling. | Order 1 assigned to Counter 1 (tie-breaker); Order 2 assigned to Counter 2 (workload 0 < 7). | Optimal allocation heuristic verified. | **PASS** |
| 31 | `Integration_ValidAndInvalidStatusTransitions` | Step order through `QUEUED -> PREPARING -> READY -> COLLECTED`; attempt invalid transitions. | Valid transitions succeed; invalid transitions throw `std::invalid_argument`. | State machine validated. | **PASS** |
| 32 | `Integration_CompleteOrderWorkflowAndWorkloadUpdate` | Place order (7m), start preparing, complete and collect (baseline mode). | Workload increases to 7m, then decreases back to 0m; counter served count increments to 1. | Workload tracking verified. | **PASS** |
| 33 | `Integration_CancellationReclaimsWorkloadAndClearsQueue` | Place 2 orders (7m + 8m = 15m); cancel first order (baseline mode). | Workload reclaimed to 8m; canceled order removed from queue; remaining order undisturbed. | Queue sanitized and workload adjusted. | **PASS** |

---

### Category 8: Machine Learning Model Tests (`tests/test_ml.cpp`)

| # | Test Identifier | Input / Scenario | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|---|
| 34 | `ML_ModelLoadSuccess` | Instantiate `RandomForestPredictor`. | Loads 20 decision trees from `ml/model.json`; `isLoaded()` is true. | 20 trees loaded successfully. | **PASS** |
| 35 | `ML_FinitePositivePredictions` | Generate feature vectors across all 5 culinary categories. | Predictions are non-NaN, non-infinite, and positive ($> 0.0$ and $< 45.0$). | All outputs finite and positive. | **PASS** |
| 36 | `ML_CompareWithPythonHeldOutPredictions` | Compare C++ predictions against Python predictions on 5 held-out test points. | Difference $|y_{\text{cpp}} - y_{\text{py}}| < 10^{-4}$ (agreement within 0.006s). | Max difference was $< 3.0 \times 10^{-7}$. | **PASS** |
| 37 | `ML_SafeInputValidationAndErrorHandling` | Pass incomplete features ($< 5$), `NaN`, or `Infinity`. | Throws `std::invalid_argument`; prevents model corruption. | Exceptions thrown safely. | **PASS** |
| 38 | `ML_MissingModelFileFallbackSafety` | Instantiate predictor pointing to nonexistent JSON file path. | Automatically falls back to embedded compiled tree; does not crash; produces valid prediction. | Fallback initialized and functional. | **PASS** |
| 39 | `ML_PredictionsReachSchedulingEngine` | Place an order through `CanteenSystem` with ML predictor active. | Order prep time set by ML model; counter workload incremented by predicted time. | Dynamic ML allocation verified. | **PASS** |
| 40 | `ML_PreservesFIFOOrderingUnderPredictedAllocations` | Enqueue 3 orders at Counter 1; complete front orders sequentially. | Dequeued orders match arrival sequence (Order 1, Order 2, Order 3); workloads return to 0. | Strict FIFO queue preserved. | **PASS** |

---

## 3. Verbatim Test Suite Console Output

```
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
[SUCCESS] Order #102 placed successfully for Student Two (S102). Assigned to Counter #2 (Predicted Prep: 5 mins via ML Model).
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
[SUCCESS] Order #103 placed successfully for Carol (STU_C). Assigned to Counter #1 (Predicted Prep: 1 mins via ML Model).
  [PASS] ML_PreservesFIFOOrderingUnderPredictedAllocations
==============================================================================
TEST SUMMARY: 40 PASSED, 0 FAILED (Total: 40)
ALL TESTS PASSED SUCCESSFULLY!
==============================================================================
```
