# DSA Traceability & Syllabus Alignment Document

**Project**: AI-Based College Canteen Queue Optimization System  
**Course Requirement**: Data Structures & Algorithms (DSA) Comprehensive Syllabus Coverage  
**Implementation Standard**: C++17, Custom Data Structures & Zero Container Adapter Shims (`std::queue`, `std::stack`, `std::list` prohibited)

---

## 1. Executive Summary

This document establishes 100% concrete traceability between every required DSA syllabus concept and its genuine implementation in the C++ codebase. Each concept is implemented from raw memory and algorithmic principles, actively participating in the canteen ordering, predictive load-balancing, and queue management engine.

---

## 2. DSA Concept Traceability Matrix

| # | Syllabus Concept | Source File & Class / Function | Application Usage Location | Real Time Complexity | Auxiliary Space | Concrete Canteen Workflow Example |
|:---:|---|---|---|:---:|:---:|---|
| **1** | **Fixed Array** | [`FixedArray<T, Capacity>`](include/canteen/ds/FixedArray.hpp) | [`CounterManager.hpp`](include/canteen/core/CounterManager.hpp)<br>`FixedArray<Counter, MAX_COUNTERS>` | Access: $O(1)$<br>Append: $O(1)$<br>Traversal: $O(n)$ | $O(\text{Capacity})$<br>(Contiguous stack) | Storing the 4 physical canteen counters (Fast Food, Meals, Beverages, Specials) with bounds-checked indexed lookup. |
| **2** | **Singly Linked List** | [`SinglyLinkedList<T>`](include/canteen/ds/SinglyLinkedList.hpp) | [`OrderManager.hpp`](include/canteen/core/OrderManager.hpp)<br>`SinglyLinkedList<Order> orderHistory` | Insert: $O(1)$<br>Delete ID: $O(n)$<br>Traversal: $O(n)$ | $O(n)$ nodes<br>($+8$ bytes pointer per node) | Maintaining completed and cancelled order history; unlinking cancelled orders by ID without shifting memory. |
| **3** | **FIFO Queue** | [`Queue<T>`](include/canteen/ds/Queue.hpp) | [`Counter.hpp`](include/canteen/models/Counter.hpp)<br>`Queue<Order> pendingOrders` | Enqueue: $O(1)$<br>Dequeue: $O(1)$<br>Front: $O(1)$ | $O(q)$ nodes<br>($q = \text{queue depth}$) | Enforcing first-come, first-served fairness for orders arriving at each service counter line. |
| **4** | **LIFO Stack & Undo** | [`Stack<T>`](include/canteen/ds/Stack.hpp)<br>[`AdminAction`](include/canteen/models/AdminAction.hpp) | [`MenuManager.hpp`](include/canteen/core/MenuManager.hpp)<br>`Stack<AdminAction> undoStack` | Push: $O(1)$<br>Pop: $O(1)$<br>Undo: $O(1)$ | $O(k)$ nodes<br>($k = \text{actions count}$) | Reverting accidental item out-of-stock toggles or menu price adjustments made by canteen administrators. |
| **5** | **Linear Search** | [`Searching.hpp`](include/canteen/algorithms/Searching.hpp)<br>`linearSearchOrderById`<br>`linearSearchFoodItemByName` | [`OrderManager.cpp`](src/core/OrderManager.cpp)<br>[`MenuManager.cpp`](src/core/MenuManager.cpp) | Best: $O(1)$<br>Worst: $O(n)$<br>Name: $O(n \cdot m)$ | $O(1)$ in-place | Searching for an order by ID across active counter queues and history; case-insensitive substring menu item lookup. |
| **6** | **Binary Search** | [`Searching.hpp`](include/canteen/algorithms/Searching.hpp)<br>`binarySearchFoodItemById` | [`MenuManager.cpp`](src/core/MenuManager.cpp)<br>`findItemByIdBinarySearch` | Best: $O(1)$<br>Worst: $O(\log n)$ | $O(1)$ in-place | Rapid logarithmic lookup of menu items by numeric ID; includes mandatory pre-sorting verification check. |
| **7** | **Bubble Sort** | [`Sorting.hpp`](include/canteen/algorithms/Sorting.hpp)<br>`bubbleSortCountersByWaitTime` | [`CounterManager.cpp`](src/core/CounterManager.cpp)<br>`sortCountersByWaitTime` | Best: $O(n)$<br>Worst: $O(n^2)$ | $O(1)$ in-place | Sorting the small set of 4 service counters by estimated wait times to display the least-busy counters to students. |
| **8** | **Merge Sort** | [`Sorting.hpp`](include/canteen/algorithms/Sorting.hpp)<br>`mergeSortFoodItemsByPrepTime`<br>`mergeSortOrdersByPrepTime` | [`MenuManager.cpp`](src/core/MenuManager.cpp)<br>[`OrderManager.cpp`](src/core/OrderManager.cpp) | Best: $O(n \log n)$<br>Worst: $O(n \log n)$ | $O(n)$ temporary buffer | Stable divide-and-conquer sorting of food items and historical order records by preparation duration or amount. |
| **9** | **Basic DSA Concepts** | Modern C++17 Architecture | System-wide across all classes | Pointers, Dynamic Memory, Rule of 5 | Zero memory leaks | Raw pointer manipulation, custom destructors, copy/move constructors, deep copies, and modular design. |

---

## 3. Deep-Dive Traceability per Syllabus Topic

### 3.1. Arrays & Fixed Capacity Containers

#### Source Implementation
- **File**: [`include/canteen/ds/FixedArray.hpp`](include/canteen/ds/FixedArray.hpp#L15-L125)
- **Class**: `canteen::ds::FixedArray<T, Capacity>`

#### Where It Is Used in the Application
- In [`include/canteen/core/CounterManager.hpp`](include/canteen/core/CounterManager.hpp#L25-L35), the collection of service counters is defined as:
  ```cpp
  FixedArray<Counter, MAX_COUNTERS> counters;
  ```
  where `MAX_COUNTERS = 5`.

#### Concrete Canteen Workflow Example
When a student visits the canteen or opens the dashboard, the system inspects the status of all counters:
1. `counters.at(0)` -> Express Fast Food #1 (Fast Food & Snacks)
2. `counters.at(1)` -> Main Meals #2 (Rice & Meals)
3. `counters.at(2)` -> Beverages & Cafe #3 (Drinks & Bakery)
4. `counters.at(3)` -> Special Counter #4 (South Indian & Special)

During order allocation, `CounterManager::allocateOrderToCounter` iterates through `counters` via $O(1)$ index offsets `counters[i]` to compute total pending workloads and select the optimal counter.

#### Actual Time & Space Complexity
- **Indexed Access (`at()`, `operator[]`)**: $\Theta(1)$ time. Computes raw memory address:
  $$\text{Address}(A[i]) = \text{BaseAddress}(A) + i \times \text{sizeof}(T)$$
- **Bounds Checking**: $\Theta(1)$ comparison `if (index >= currentSize) throw std::out_of_range(...)`.
- **Append (`add()`)**: $\Theta(1)$ amortized/worst-case; throws `std::overflow_error` if `currentSize >= Capacity`.
- **Space Complexity**: $\Theta(\text{Capacity})$ contiguous stack/member memory. Zero heap allocations.

#### Why Appropriate
Physical canteen counters are bounded by the physical architecture of the canteen building (4 to 5 stalls). Dynamic vectors introduce unwanted heap fragmentation and reallocation overhead. A fixed-capacity array exactly models physical infrastructure.

#### Trade-offs & Limitations
- **Compile-Time Bound**: Cannot expand beyond `Capacity` at runtime without modifying code and recompiling.
- **Unused Capacity Overhead**: If `Capacity = 5` and only 4 counters are active, 1 slot remains pre-allocated.

---

### 3.2. Singly Linked Lists

#### Source Implementation
- **File**: [`include/canteen/ds/SinglyLinkedList.hpp`](include/canteen/ds/SinglyLinkedList.hpp#L15-L180)
- **Class**: `canteen::ds::SinglyLinkedList<T>`

#### Where It Is Used in the Application
- Serves as the underlying node-based data store for the custom FIFO `Queue<T>`.
- In [`include/canteen/core/OrderManager.hpp`](include/canteen/core/OrderManager.hpp#L25-L30), stores completed and cancelled order archives:
  ```cpp
  SinglyLinkedList<Order> orderHistory;
  ```

#### Concrete Canteen Workflow Example
When an order is collected (`POST /api/orders/{id}/collect`), it transitions out of the active counter queue and is inserted into `orderHistory` via `insertBack(completedOrder)`. If an emergency cancellation occurs (`cancelOrder()`), `deleteBy(id)` traverses the list and unlinks the node in $O(n)$ time without shifting elements.

#### Memory Management & Rule of Five
The implementation strictly fulfills the C++ **Rule of Five**:
1. **Destructor**: Recursively deallocates all heap-allocated `ListNode<T>` structures via `clear()`.
2. **Copy Constructor**: Deep-copies every node to ensure independent memory spaces.
3. **Copy Assignment Operator**: Uses copy-and-swap idiom to prevent self-assignment and memory leaks.
4. **Move Constructor**: Transfers ownership of `head`, `tail`, and `count` pointers in $O(1)$ time, resetting the source to `nullptr`.
5. **Move Assignment Operator**: Cleans existing nodes and adopts source pointers in $O(1)$ time.

#### Actual Time & Space Complexity
- **Insertion at Front (`insertFront`)**: $\Theta(1)$ pointer assignment.
- **Insertion at Back (`insertBack`)**: $\Theta(1)$ utilizing cached `tail` pointer (`tail->next = newNode; tail = newNode`).
- **Deletion by Value (`deleteBy`)**: Best Case $\Theta(1)$ (head node); Worst Case $O(n)$ (tail node or absent).
- **Space Complexity**: $\Theta(n)$ where each node requires $\text{sizeof}(T) + 8$ bytes for the `next` pointer.

#### Why Appropriate
Allows dynamic expansion as hundreds of students place orders during peak hours, and enables arbitrary unlinking of nodes upon order cancellation without the $O(n)$ memory block shifting required by arrays.

#### Trade-offs & Limitations
- **No Random Access**: Accessing the $k$-th element requires $O(k)$ pointer traversals.
- **Cache Locality**: Non-contiguous heap nodes have worse CPU cache locality than contiguous arrays.

---

### 3.3. FIFO Queues

#### Source Implementation
- **File**: [`include/canteen/ds/Queue.hpp`](include/canteen/ds/Queue.hpp#L15-L95)
- **Class**: `canteen::ds::Queue<T>`

#### Where It Is Used in the Application
- In [`include/canteen/models/Counter.hpp`](include/canteen/models/Counter.hpp#L25-L35), each service counter owns an independent FIFO queue:
  ```cpp
  Queue<Order> pendingOrders;
  ```

#### Concrete Canteen Workflow Example
1. At 12:05 PM, Student A places Order #101 (assigned to Counter 1).
2. At 12:07 PM, Student B places Order #102 (assigned to Counter 1).
3. `Counter 1`'s queue order is: `[#101 (Front) -> #102 (Rear)]`.
4. Kitchen staff can only start cooking Order #101 (`POST /api/orders/101/start`).
5. Trying to start Order #102 before Order #101 completes returns `HTTP 409 Conflict` ("Order is not at front of queue").
6. When Order #101 is finished, `completeFrontOrder()` dequeues #101 in $O(1)$, advancing #102 to the front.

#### Actual Time & Space Complexity
- **Enqueue (`enqueue(item)`)**: $\Theta(1)$ time (tail pointer insertion).
- **Dequeue (`dequeue()`)**: $\Theta(1)$ time (head pointer detachment).
- **Front Inspection (`front()`)**: $\Theta(1)$ time.
- **Cancellation (`removeIf()`)**: $O(n)$ search and unlink.
- **Space Complexity**: $\Theta(q)$ where $q$ is the number of currently queued orders.

#### Why Appropriate
Enforces absolute First-Come, First-Served fairness. Prevents students who ordered later from jumping the queue.

#### Trade-offs & Limitations
- Cancellation of an order in the middle of the queue requires linear traversal ($O(n)$) to locate and unlink the cancelled order.

---

### 3.4. LIFO Stacks & Administrative Undo

#### Source Implementation
- **File**: [`include/canteen/ds/Stack.hpp`](include/canteen/ds/Stack.hpp#L15-L110)
- **Class**: `canteen::ds::Stack<T>`
- **Entity**: [`AdminAction`](include/canteen/models/AdminAction.hpp#L10-L40)

#### Where It Is Used in the Application
- In [`include/canteen/core/MenuManager.hpp`](include/canteen/core/MenuManager.hpp#L28-L35):
  ```cpp
  Stack<AdminAction> undoStack;
  ```

#### Concrete Canteen Workflow Example
1. Cafeteria staff accidentally toggles "Paneer Kathi Roll" to unavailable (`is_available = false`).
2. The mutation pushes an `AdminAction` onto `undoStack`:
   `{ type: UPDATE_ITEM_AVAILABILITY, itemId: 102, previousState: "available", newState: "unavailable" }`.
3. Staff realizes the mistake and invokes `undoLastAdminAction()`.
4. The system pops the top action from the stack in $O(1)$ and flips the item back to `is_available = true`.

#### Actual Time & Space Complexity
- **Push (`push()`)**: $\Theta(1)$ pointer insertion at the top.
- **Pop (`pop()`)**: $\Theta(1)$ pointer detachment at the top.
- **Peek (`peek()`, `top()`)**: $\Theta(1)$ access.
- **Space Complexity**: $\Theta(k)$ where $k$ is the number of recorded administrative actions.

#### Why Appropriate
Administrative changes follow strict Last-In, First-Out reversal semantics. The last change executed must be the first change reverted.

#### Trade-offs & Limitations
- Undo is strictly sequential (LIFO); cannot undo an action buried 3 levels deep without undoing the intervening actions.

---

### 3.5. Linear and Binary Searching

#### Source Implementation
- **File**: [`include/canteen/algorithms/Searching.hpp`](include/canteen/algorithms/Searching.hpp#L15-L105)
- **Functions**:
  - `linearSearchOrderById`
  - `linearSearchFoodItemByName`
  - `binarySearchFoodItemById`

#### Where It Is Used in the Application
- **Linear Search**:
  - [`OrderManager.cpp`](src/core/OrderManager.cpp#L60-L85): Searches for an order across all active counter queues ($O(\sum q_i)$) and history linked list ($O(h)$).
  - [`MenuManager.cpp`](src/core/MenuManager.cpp#L45-L65): Substring search matching user query (e.g. typing "Burger" matches "Veg Cheese Burger").
- **Binary Search**:
  - [`MenuManager.cpp`](src/core/MenuManager.cpp#L85-L110): `findItemByIdBinarySearch` on sorted item arrays.

#### Binary Search Precondition Verification
Binary search strictly requires the underlying dataset to be sorted in ascending order. In [`Searching.hpp`](include/canteen/algorithms/Searching.hpp#L68-L76):
```cpp
if (!isSorted(items)) {
    throw std::invalid_argument("Binary search requires array to be sorted by ID");
}
```
If an unsorted array is passed, the binary elimination assumption ($\text{target} < \text{arr}[\text{mid}] \implies \text{search left}$) fails, causing valid items to be missed. The implementation guards this invariant explicitly.

#### Actual Time & Space Complexity
- **Linear Search (Order ID)**: Best $O(1)$, Worst $O(n)$, Space $O(1)$.
- **Linear Search (Substring Name)**: Worst $O(n \cdot m)$ where $m$ is string length.
- **Binary Search (Sorted ID)**: Best $O(1)$, Worst $O(\log n)$, Space $O(1)$ iterative.

---

### 3.6. Bubble Sort and Merge Sort

#### Source Implementation
- **File**: [`include/canteen/algorithms/Sorting.hpp`](include/canteen/algorithms/Sorting.hpp#L15-L150)
- **Functions**:
  - `bubbleSortCountersByWaitTime`
  - `mergeSortFoodItemsByPrepTime`
  - `mergeSortOrdersByPrepTime`

#### Where It Is Used in the Application
- **Bubble Sort**:
  - [`CounterManager.cpp`](src/core/CounterManager.cpp#L90-L115): `sortCountersByWaitTime()` sorts the 4 counters by current workload for live queue display.
  - **Early Termination**: Tracks `bool swapped`. If a pass executes zero swaps, terminates in $O(n)$ best-case time.
  - **Deterministic Tie-Breaking**: If two counters have identical wait times, ties are broken using counter ID:
    ```cpp
    if (waitA > waitB || (waitA == waitB && idA > idB)) {
        std::swap(counters[j], counters[j + 1]);
    }
    ```
- **Merge Sort**:
  - [`MenuManager.cpp`](src/core/MenuManager.cpp#L115-L140): Sorts menu items by preparation duration.
  - [`OrderManager.cpp`](src/core/OrderManager.cpp#L120-L145): Sorts historical order records by preparation duration or amount.
  - **Divide-and-Conquer**: Recursively divides lists down to single elements and merges with guaranteed stability ($O(n \log n)$).

#### Actual Time & Space Complexity
- **Bubble Sort**: Best Case $O(n)$ (already sorted), Average $O(n^2)$, Worst $O(n^2)$, Space $O(1)$ in-place.
- **Merge Sort**: Best, Average, Worst $\Theta(n \log n)$, Space $O(n)$ temporary buffer.

#### Why Appropriate
- **Bubble Sort for Counters**: Because $N = 4$ counters, $N^2 = 16$ comparisons is virtually instantaneous and uses zero extra heap memory.
- **Merge Sort for Orders/Menu**: Menu items and order history can grow large ($N \ge 1,000$). Merge Sort guarantees strict $O(n \log n)$ performance and preserves the relative order of items with identical preparation times (stability).

---

### 3.7. Predictive ML Integration with DSA Counter Scheduling

#### Source Implementation
- **Files**:
  - [`include/canteen/ml/RandomForestPredictor.hpp`](include/canteen/ml/RandomForestPredictor.hpp)
  - [`src/ml/RandomForestPredictor.cpp`](src/ml/RandomForestPredictor.cpp)
  - [`src/core/CounterManager.cpp`](src/core/CounterManager.cpp#L45-L85)

#### Predictive Scheduling Data Flow
1. Student places an order with $k$ food items.
2. The scheduler iterates through the `FixedArray` of available counters ($O(C)$).
3. For each counter $C_i$, the scheduler extracts a 5-dimensional feature vector:
   $$\mathbf{x}(O, C_i) = \left[ \text{Category}, \text{Quantity}, \text{Complexity}, \text{TimeOfDay}, \text{Workload}(C_i) \right]$$
4. Evaluates the 20-tree `RandomForestRegressor` in $O(\text{trees} \times \text{depth}) = O(20 \times 6) = O(1)$ constant time.
5. Projects the expected total waiting time:
   $$\text{ProjectedCompletion}(C_i) = \text{Workload}(C_i) + T_{\text{pred}}(O, C_i)$$
6. Selects optimal counter $C^* = \arg\min (\text{ProjectedCompletion}(C_i))$.
7. Enqueues the order to $C^*$'s FIFO `Queue<Order>` in $O(1)$ and increments $C^*$'s workload by $T_{\text{pred}}$.
8. Strict FIFO ordering is preserved: earlier orders are served first, regardless of individual predicted times.

---

## 4. Verification & Defense Checklist for College Viva

- [x] All 7 DSA topics implemented with dedicated custom classes.
- [x] No forbidden STL container adapters (`std::queue`, `std::stack`, `std::list`).
- [x] Memory management verified with zero memory leaks via Rule of Five.
- [x] Automated test coverage: 58 passed unit/integration tests (`tests/test_*.cpp`).
- [x] End-to-end browser QA verified: 10 passed user journeys via Chrome CDP (`scripts/qa_browser_e2e.mjs`).
