# AI-Based College Canteen Queue Optimization System (Phase 1: DSA Foundation)

An advanced C++ Data Structures & Algorithms (DSA) project engineered to optimize college canteen queues, balance kitchen workloads across counters, track complete order lifecycles, and provide an extensible core for future AI/ML predictive scheduling and graphical dashboards.

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

## 🏗️ Architecture & Project Structure

```
c:\SY\FODS CP FINAL\
├── CMakeLists.txt                    # Modern CMake build configuration
├── build.bat                         # Automated Windows build & test runner
├── README.md                         # Project documentation and quickstart
├── docs/
│   ├── algorithms.md                 # Detailed data structure & algorithm specifications
│   ├── complexity-analysis.md        # Comprehensive time and space complexity analysis
│   └── testing-report.md             # Verification results for all 33 unit & integration tests
├── include/
│   └── canteen/
│       ├── models/
│       │   ├── FoodItem.hpp          # FoodItem model (ID, price, prep time, availability)
│       │   ├── Order.hpp             # Order model with status lifecycle validation
│       │   ├── OrderStatus.hpp       # State machine: QUEUED -> PREPARING -> READY -> COLLECTED
│       │   ├── Counter.hpp           # Service counter holding its dedicated FIFO OrderQueue
│       │   └── AdminAction.hpp       # Encapsulates administrative actions for stack undo
│       ├── ds/
│       │   ├── FixedArray.hpp        # Bounds-checked fixed array implementation
│       │   ├── SinglyLinkedList.hpp  # Memory-safe singly linked list with head/tail pointers
│       │   ├── Queue.hpp             # FIFO Queue built on SinglyLinkedList
│       │   └── Stack.hpp             # LIFO Stack for action audit and rollback
│       ├── algorithms/
│       │   ├── Searching.hpp         # Linear Search and Binary Search
│       │   └── Sorting.hpp           # In-place Bubble Sort and Divide-and-Conquer Merge Sort
│       └── core/
│           ├── MenuManager.hpp       # Catalog management, search/sort, and undo stack
│           ├── CounterManager.hpp    # Counter workload optimization and allocation heuristic
│           ├── OrderManager.hpp      # Order placement, audit trail, and cross-queue search
│           └── CanteenSystem.hpp     # Unified facade and simulation coordinator
├── src/
│   ├── models/
│   │   ├── FoodItem.cpp
│   │   ├── Order.cpp
│   │   └── Counter.cpp
│   ├── core/
│   │   ├── MenuManager.cpp
│   │   ├── CounterManager.cpp
│   │   ├── OrderManager.cpp
│   │   └── CanteenSystem.cpp
│   └── main.cpp                      # CLI entry point (supports interactive and --demo modes)
└── tests/
    ├── test_framework.hpp            # Lightweight standalone test runner with zero dependencies
    ├── test_main.cpp                 # Test registry runner
    ├── test_array.cpp                # FixedArray indexing, bounds checking, and overflow tests
    ├── test_linked_list.cpp          # SinglyLinkedList edge cases, delete, search, Rule of 5
    ├── test_queue.cpp                # FIFO order, enqueue/dequeue cycles, cancellation
    ├── test_stack.cpp                # LIFO order, deep copy, admin undo verification
    ├── test_searching.cpp            # Linear & Binary search, sortedness validation
    ├── test_sorting.cpp              # Bubble Sort & Merge Sort (empty, sorted, reverse, duplicates)
    └── test_integration.cpp          # End-to-end allocation, order lifecycle, workload updates
```

---

## ⚙️ Prerequisites & Toolchain

- **C++ Compiler**: C++17 or C++20 compliant compiler:
  - MSVC (Visual Studio 2019 / 2022 / 2026 Developer Command Prompt)
  - GCC / MinGW (g++ 7.0+)
  - Clang (clang++ 6.0+)
- **Build System**: CMake (version 3.15 or later)
- **Platform**: Windows, Linux, or macOS

---

## 🚀 Build Instructions

### Option 1: Quick Build via `build.bat` (Windows)

Simply run:
```cmd
build.bat
```
This automatically configures the project with CMake, compiles both `canteen_app.exe` and `canteen_tests.exe`, and executes all unit tests.

### Option 2: Standard CMake Build

```bash
# 1. Configure the build directory
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Build the core library, CLI application, and tests
cmake --build build --config Release

# 3. Execute the full test suite
ctest --test-dir build --output-on-failure
# Or run tests executable directly:
./build/canteen_tests          # Linux / macOS
.\build\Release\canteen_tests   # Windows (MSVC)
```

---

## 💻 Running the Application

### 1. Interactive Console Menu

Run without arguments to start the interactive management console:
```cmd
.\build\Debug\canteen_app.exe
```
Menu options include:
- Viewing Menu and Counters
- Placing sample orders with auto-allocation to the counter with minimum workload
- Stepping through order preparation and collection
- Cancelling pending orders
- Searching orders and menu items (Linear and Binary Search)
- Sorting counters and menu items (Bubble and Merge Sort)
- Updating menu availability and performing Undo operations

### 2. Automated Demonstration Simulation

To run a non-interactive, automated live viva demonstration covering all 7 DSA topics sequentially:
```cmd
.\build\Debug\canteen_app.exe --demo
```

---

## 🧪 Test Results Summary

The project includes 33 comprehensive unit and integration tests covering all critical boundaries, exceptional conditions, and memory safety invariants.

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
==============================================================================
TEST SUMMARY: 33 PASSED, 0 FAILED (Total: 33)
ALL TESTS PASSED SUCCESSFULLY!
==============================================================================
```
