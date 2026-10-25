# AI-Based College Canteen Queue Optimization System

An advanced C++ Data Structures & Algorithms (DSA) and Machine Learning project engineered to optimize college canteen queues, predict dynamic food preparation durations using a trained `RandomForestRegressor`, balance kitchen workloads across counters, and track complete order lifecycles.

---

## 📋 Course & Syllabus Alignment

All 7 core DSA topics required by the syllabus are implemented from scratch with zero reliance on standard-library container adapters (`std::queue`, `std::stack`, `std::list`):

| # | DSA Syllabus Topic | Custom Implementation Component | Purpose & Meaningful Application |
|---|---|---|---|
| 1 | **Arrays** | [`FixedArray<T, Capacity>`](include/canteen/ds/FixedArray.hpp) | Fixed collection of canteen service counters; $O(1)$ bounds-checked indexed access, array traversal, and workload updates. |
| 2 | **Linked Lists** | [`SinglyLinkedList<T>`](include/canteen/ds/SinglyLinkedList.hpp) | Raw pointer singly linked list with Rule of Five compliance; serves as the underlying dynamic storage for order queues and history tracking. |
| 3 | **Queues** | [`Queue<T>`](include/canteen/ds/Queue.hpp) | Custom FIFO queue with $O(1)$ front and rear operations; maintains independent queue per counter and enforces first-in, first-out order processing. |
| 4 | **Stacks** | [`Stack<T>`](include/canteen/ds/Stack.hpp) | Custom LIFO stack using linked nodes; manages admin action history and enables $O(1)$ undo functionality for menu availability and pricing changes. |
| 5 | **Searching** | [`Searching.hpp`](include/canteen/algorithms/Searching.hpp) | **Linear Search** ($O(n)$) for orders by ID and items by name/ID; **Binary Search** ($O(\log n)$) on sorted item IDs with strict precondition sorting validation. |
| 6 | **Sorting** | [`Sorting.hpp`](include/canteen/algorithms/Sorting.hpp) | **Bubble Sort** ($O(n^2)$ in-place) to sort counters by estimated wait times; **Merge Sort** ($O(n \log n)$ stable) to sort menu items and orders by prep time or price. |
| 7 | **Basic DSA Concepts** | System Architecture | Classes, encapsulation, raw pointers, dynamic memory management (`new`/`delete`), Rule of Five (copy/move), state machines, and Big-O complexity. |

---

## 🤖 Machine Learning Integration (Phase 2)

A `RandomForestRegressor` (20 estimators, maximum depth 6) was trained using Python (`scikit-learn`, `pandas`, `numpy`) on a reproducible synthetic dataset ($N = 2,500$ samples) to predict preparation times based on:
1. `food_category` (0: Fast Food, 1: South Indian, 2: Meals/Chinese, 3: Beverages, 4: Bakery/Snacks)
2. `order_quantity` (1 to 5 items)
3. `preparation_complexity` (1 to 5)
4. `time_of_day_hour` (8.0 to 20.0 decimal hours)
5. `current_kitchen_workload` (0 to 30 minutes pending at the evaluated counter)

### Evaluation Metrics on Held-Out Test Set (500 samples)
- **Baseline Model (Training Set Mean Predictor)**:
  - MAE: **4.5003 minutes**
  - RMSE: **5.5477 minutes**
- **Trained RandomForestRegressor**:
  - MAE: **0.8139 minutes** (81.91% reduction over baseline)
  - RMSE: **1.0411 minutes** (81.23% reduction over baseline)
  - $R^2$ Score: **0.9648**
- **C++ Inference Engine**:
  - Direct tree traversal in `RandomForestPredictor.cpp`.
  - Zero external DLL dependencies.
  - Matches Python predictions down to $< 10^{-6}$ error on held-out test points.
  - Automatic fallback to compiled constants (`EmbeddedModelData.hpp`) or baseline sum if `model.json` is missing.

---

## 🏗️ Architecture & Project Structure

```
c:\SY\FODS CP FINAL\
├── CMakeLists.txt                         # Modern CMake build configuration
├── build.bat                              # Automated Windows build & test runner
├── README.md                              # Project documentation and quickstart
├── ml/                                    # Machine learning training & export pipeline
│   ├── requirements.txt                   # scikit-learn, pandas, numpy
│   ├── generate_dataset.py                # Synthetic dataset generator (reproducible seed 42)
│   ├── train_model.py                     # RandomForestRegressor training script
│   ├── evaluate_model.py                  # Evaluation vs baseline generating metrics.json
│   ├── export_model.py                    # Exports trees to model.json & EmbeddedModelData.hpp
│   ├── canteen_prep_data.csv              # Synthetic dataset (2,500 rows)
│   ├── model.json                         # Serialized 20-tree Random Forest
│   ├── metrics.json                       # Evaluated MAE, RMSE, R^2, and feature importances
│   └── test_predictions.json             # Held-out samples for C++ vs Python cross-verification
├── docs/
│   ├── algorithms.md                      # Data structures & predictive scheduling specifications
│   ├── ml-model.md                        # Machine learning model documentation & metrics
│   ├── complexity-analysis.md             # Asymptotic time and space complexity analysis
│   └── testing-report.md                  # Verification report covering all 40 unit & integration tests
├── include/
│   └── canteen/
│       ├── ml/
│       │   ├── IPrepTimePredictor.hpp     # Abstract prediction interface and feature vector
│       │   └── RandomForestPredictor.hpp  # In-memory Random Forest inference engine
│       ├── models/
│       │   ├── FoodItem.hpp               # FoodItem model (ID, price, prep time, complexity)
│       │   ├── Order.hpp                  # Order model with status lifecycle validation
│       │   ├── OrderStatus.hpp            # State machine: QUEUED -> PREPARING -> READY -> COLLECTED
│       │   ├── Counter.hpp                # Service counter holding its dedicated FIFO OrderQueue
│       │   ├── AdminAction.hpp            # Encapsulates administrative actions for stack undo
│       │   └── EmbeddedModelData.hpp      # Auto-generated zero-dependency compiled tree fallback
│       ├── ds/
│       │   ├── FixedArray.hpp             # Bounds-checked fixed array implementation
│       │   ├── SinglyLinkedList.hpp       # Memory-safe singly linked list with head/tail pointers
│       │   ├── Queue.hpp                  # FIFO Queue built on SinglyLinkedList
│       │   └── Stack.hpp                  # LIFO Stack for action audit and rollback
│       ├── algorithms/
│       │   ├── Searching.hpp              # Linear Search and Binary Search
│       │   └── Sorting.hpp                # In-place Bubble Sort and Divide-and-Conquer Merge Sort
│       └── core/
│           ├── MenuManager.hpp            # Catalog management, search/sort, and undo stack
│           ├── CounterManager.hpp         # Counter workload optimization and allocation heuristic
│           ├── OrderManager.hpp           # Order placement, audit trail, and cross-queue search
│           └── CanteenSystem.hpp          # Unified facade and simulation coordinator
├── src/
│   ├── ml/
│   │   └── RandomForestPredictor.cpp      # Fast C++ tree traversal and JSON model loader
│   ├── models/
│   │   ├── FoodItem.cpp
│   │   ├── Order.cpp
│   │   └── Counter.cpp
│   ├── core/
│   │   ├── MenuManager.cpp
│   │   ├── CounterManager.cpp
│   │   ├── OrderManager.cpp
│   │   └── CanteenSystem.cpp
│   └── main.cpp                           # CLI entry point (interactive CLI + `--demo` flag)
└── tests/
    ├── test_framework.hpp                 # Lightweight standalone test runner
    ├── test_main.cpp                      # Test registry runner
    ├── test_array.cpp                     # 5 Unit tests for FixedArray
    ├── test_linked_list.cpp               # 6 Unit tests for SinglyLinkedList
    ├── test_queue.cpp                     # 5 Unit tests for FIFO Queue
    ├── test_stack.cpp                     # 4 Unit tests for LIFO Stack & Undo
    ├── test_searching.cpp                 # 4 Unit tests for Linear and Binary Search
    ├── test_sorting.cpp                   # 5 Unit tests for Bubble Sort and Merge Sort
    ├── test_integration.cpp               # 4 Integration tests for allocation & workload
    └── test_ml.cpp                        # 7 Unit tests for ML inference & cross-language verification
```

---

## ⚙️ Prerequisites & Toolchain

- **C++ Compiler**: C++17 or C++20 compliant compiler:
  - MSVC (Visual Studio 2019 / 2022 / 2026 Developer Command Prompt)
  - GCC / MinGW (g++ 7.0+)
  - Clang (clang++ 6.0+)
- **Build System**: CMake (version 3.15 or later)
- **Python**: Python 3.9+ with `scikit-learn`, `pandas`, `numpy` (for training pipeline)
- **Platform**: Windows, Linux, or macOS

---

## 🚀 Build and Run Instructions

### 1. Build and Run All Tests via `build.bat` (Windows)

```cmd
build.bat
```
This builds `canteen_core.lib`, `canteen_app.exe`, and `canteen_tests.exe`, and executes all 40 tests.

### 2. Retraining the ML Model (Optional)

```cmd
python ml/generate_dataset.py
python ml/train_model.py
python ml/evaluate_model.py
python ml/export_model.py
```

### 3. Running the Live Console Application

```cmd
.\build\Debug\canteen_app.exe
```
Or run the non-interactive automated viva demonstration:
```cmd
.\build\Debug\canteen_app.exe --demo
```

---

## 🧪 Test Results Summary

**Command executed:**
```cmd
build\Debug\canteen_tests.exe
```
**Output:**
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
  [PASS] Integration_OrderPlacementAndAllocation
  [PASS] Integration_ValidAndInvalidStatusTransitions
  [PASS] Integration_CompleteOrderWorkflowAndWorkloadUpdate
  [PASS] Integration_CancellationReclaimsWorkloadAndClearsQueue
  [PASS] ML_ModelLoadSuccess
  [PASS] ML_FinitePositivePredictions
  [PASS] ML_CompareWithPythonHeldOutPredictions
  [PASS] ML_SafeInputValidationAndErrorHandling
  [PASS] ML_MissingModelFileFallbackSafety
  [PASS] ML_PredictionsReachSchedulingEngine
  [PASS] ML_PreservesFIFOOrderingUnderPredictedAllocations
==============================================================================
TEST SUMMARY: 40 PASSED, 0 FAILED (Total: 40)
ALL TESTS PASSED SUCCESSFULLY!
==============================================================================
```
