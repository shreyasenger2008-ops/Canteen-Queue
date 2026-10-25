# Data Structures and Algorithms Specification

This document provides technical specifications and implementation details for all data structures and algorithms engineered for the **AI-Based College Canteen Queue Optimization System**.

---

## 1. Custom Fixed Array (`FixedArray<T, Capacity>`)

### Purpose
Represents the fixed physical infrastructure of the canteen: service counters. Unlike dynamically resizing vectors, a fixed array models physical counters where capacity is bounded by canteen layout ($N \le 5$).

### Implementation Highlights
- **Contiguous Allocation**: Elements are laid out sequentially in stack memory: `T elements[Capacity]`.
- **$O(1)$ Indexed Access**: `at(size_t index)` and `operator[](size_t index)` compute direct address offsets `base_address + index * sizeof(T)`.
- **Bounds Checking**: `at()` validates that `index < currentSize`, throwing `std::out_of_range` on violation.
- **Overflow Prevention**: `add()` checks `currentSize < Capacity`, throwing `std::overflow_error` if full.
- **Traversal**: Linear traversal $O(n)$ via the `traverse(Visitor&&)` method and standard pointer iterators (`begin()`, `end()`).

---

## 2. Custom Singly Linked List (`SinglyLinkedList<T>`)

### Purpose
Provides dynamic node-based sequential storage for orders where dynamic insertions and arbitrary deletions (e.g. order cancellation) frequently occur without shifting contiguous memory.

### Implementation Highlights
- **Node Structure**:
  ```cpp
  template <typename T>
  struct ListNode {
      T data;
      ListNode<T>* next{nullptr};
  };
  ```
- **Dual End Pointers**: Maintains both `ListNode<T>* head` and `ListNode<T>* tail`.
  - Insertion at Front: `head = newNode; newNode->next = oldHead` in $O(1)$.
  - Insertion at Back: `tail->next = newNode; tail = newNode` in $O(1)$.
- **Deletion by Predicate / ID**:
  Handles four distinct boundary cases:
  1. *Empty List*: Returns `false` immediately.
  2. *Head Match*: Moves `head` to `head->next`; if list becomes empty, resets `tail = nullptr`.
  3. *Middle Match*: `prev->next = curr->next; delete curr;`
  4. *Tail Match*: `prev->next = nullptr; tail = prev; delete curr;`
- **Memory Safety (Rule of Five)**:
  - Custom destructor deletes all nodes sequentially.
  - Deep copy constructor and copy assignment operator replicate each node into distinct heap memory blocks.
  - Move constructor and move assignment operator transfer pointers and reset source to `nullptr` in $O(1)$ without allocation.

---

## 3. Custom FIFO Queue (`Queue<T>`)

### Purpose
Models the waiting line at each individual canteen counter. It enforces strict First-In, First-Out order: the first student to place an order at a counter is the first to have their food prepared and served.

### Implementation Highlights
- **Composition over Inheritance**: Wraps `SinglyLinkedList<T>`.
- **$O(1)$ Enqueue**: Inserts new orders at the `tail` of the linked list.
- **$O(1)$ Dequeue**: Extracts the order at the `head` of the linked list.
- **Strict Invariants**:
  - `dequeue()` and `front()` throw `std::underflow_error` when invoked on an empty queue.
- **Arbitrary Cancellation**: `removeIf(Predicate&&)` enables cancellation of queued orders without disturbing the relative arrival sequence of remaining students.

### Order State Machine Transitions
Orders progress through well-defined lifecycle states:
```
  [ QUEUED ] ───► [ PREPARING ] ───► [ READY ] ───► [ COLLECTED ] (Terminal)
       │                 │
       ▼                 ▼
  [ CANCELLED ]    [ CANCELLED ] (Terminal)
```
Invalid transitions (e.g., `COLLECTED -> QUEUED` or `CANCELLED -> READY`) are rejected with `std::invalid_argument`.

---

## 4. Custom Stack (`Stack<T>`) and Administrative Undo

### Purpose
Records administrative mutations (e.g., toggling food item availability or adjusting menu pricing) to allow cafeteria managers to revert accidental changes.

### Implementation Highlights
- **LIFO Semantics**: Last action performed is the first action undone.
- **Linked Node Structure**: Each `push()` dynamically allocates a `StackNode<T>` at the top ($O(1)$); `pop()` deallocates the top node ($O(1)$).
- **Audit Entity (`AdminAction`)**:
  Stores:
  - Action identifier & type (`UPDATE_ITEM_AVAILABILITY`, `UPDATE_ITEM_PRICE`)
  - Target item ID and item name
  - Previous state and new state
  - Timestamp & textual description
- **Integrity Rule**: Undo is strictly restricted to reversible administrative metadata. Queue positions and already-collected meals cannot be corrupted by undo actions.

---

## 5. Searching Algorithms

### A. Linear Search (`searching::linearSearch*`)
- **Use Cases**:
  - Locating an order by ID across active counter queues and historical archives.
  - Searching menu items by case-insensitive name or substring (e.g. searching "coffee" matches "Cold Coffee").
- **Mechanism**: Iterates from index 0 to $n-1$, testing each element against the query.
- **Complexities**:
  - Best Case: $O(1)$ (target at index 0)
  - Worst Case: $O(n)$ (target at index $n-1$ or absent)
  - Auxiliary Space: $O(1)$

### B. Binary Search (`searching::binarySearch*`)
- **Use Cases**: Fast $O(\log n)$ lookup of food items by ID in a sorted menu.
- **Precondition Verification**: Explicitly verifies that the array is sorted in ascending order before executing binary search. Throws `std::invalid_argument` if unsorted.
- **Monotonicity Principle**:
  $$\text{If } \text{target} < \text{arr}[\text{mid}], \quad \forall k \ge \text{mid}: \text{arr}[k] > \text{target}$$
  Because the array is monotonically ordered, half of the remaining elements are eliminated in each step. In an unsorted array, smaller elements could be located anywhere in the discarded partition, leading to incorrect negative results.
- **Overflow Prevention**: Middle index calculated via `mid = low + (high - low) / 2` instead of `(low + high) / 2`.
- **Complexities**:
  - Best Case: $O(1)$ (target matches initial midpoint)
  - Worst Case: $O(\log n)$
  - Auxiliary Space: $O(1)$ iterative

---

## 6. Sorting Algorithms

### A. Bubble Sort (`sorting::bubbleSortCountersByWaitTime`)
- **Use Cases**: Sorting counters by estimated waiting time to display the least-busy counters to students.
- **Mechanism**:
  - Repeatedly steps through the array, comparing adjacent counter wait times.
  - Swaps adjacent counters if `wait[j] > wait[j+1]`.
  - **Early Termination Flag**: Tracks `bool swapped`. If an entire pass completes without swaps, the array is already sorted and the algorithm terminates immediately ($O(n)$ best case).
  - **Deterministic Tie-Breaking**: When two counters have identical wait times, ties are broken using counter ID ascending.
- **Complexities**:
  - Best Case: $O(n)$
  - Worst Case: $O(n^2)$ (reverse sorted)
  - Auxiliary Space: $O(1)$ in-place

### B. Merge Sort (`sorting::mergeSort*`)
- **Use Cases**: Sorting large lists of menu items and order records by preparation time, price, or ID.
- **Mechanism (Divide-and-Conquer)**:
  1. **Divide**: Calculates middle point `mid = left + (right - left) / 2`.
  2. **Conquer**: Recursively sorts left subarray `[left ... mid]` and right subarray `[mid+1 ... right]`.
  3. **Combine**: Merges sorted subarrays into a temporary buffer using two pointers `i` and `j`.
  - **Stability**: Elements with duplicate keys maintain their original relative order by checking `!comp(right[j], left[i])` during merge.
- **Complexities**:
  - Best / Average / Worst Case: $\Theta(n \log n)$
  - Auxiliary Space: $O(n)$ (temporary merge buffers)

---

## 7. Counter Allocation Strategy & Predictive Scheduling

The system implements two distinct scheduling strategies, cleanly decoupled via the `IPrepTimePredictor` interface:

### A. Phase 1 Static Baseline Heuristic
1. Traverse all registered counters in `FixedArray`.
2. Filter for counters where `isAvailable == true`.
3. Compute total pending workload:
   $$\text{Workload}(C) = \sum_{O \in \text{Queue}(C)} \text{PrepTime}(O)$$
4. Select counter $C^*$ satisfying:
   $$C^* = \arg\min_{C \text{ available}} \left( \text{Workload}(C), \, \text{ID}(C) \right)$$
5. If workloads are tied, the counter with the lower numeric ID is selected deterministically.

### B. Phase 2 Machine-Learning-Driven Predictive Scheduler
Instead of relying on fixed menu preparation sums, the scheduler queries the trained `RandomForestPredictor` for each available counter $C$:
1. **Contextual Feature Extraction**:
   Constructs the feature vector $\mathbf{x}(O, C) = [ \text{Category}(O), \text{Quantity}(O), \text{Complexity}(O), \text{Hour}, \text{Workload}(C) ]$.
2. **Model Inference**:
   $$T_{\text{pred}}(O, C) = \text{RandomForestPredictor}.\text{predict}(\mathbf{x}(O, C))$$
   The prediction is validated for sanity ($T_{\text{pred}} \ge 1.0$, non-NaN). If inference fails or the model is missing, the static baseline sum is used as a safe fallback.
3. **Projected Completion Time**:
   $$\text{ProjectedCompletion}(C) = \text{Workload}(C) + T_{\text{pred}}(O, C)$$
4. **Optimal Counter Selection**:
   $$C^* = \arg\min_{C \text{ available}} \left( \text{ProjectedCompletion}(C), \, \text{ID}(C) \right)$$
5. **FIFO Queue & Workload Updates**:
   - Order preparation time is updated to $\text{round}(T_{\text{pred}}(O, C^*))$.
   - Order is enqueued to $C^*$'s custom FIFO queue.
   - $C^*$'s workload is incremented by this predicted duration.
   - When the order is completed or cancelled, its preparation duration is cleanly deducted from $C^*$'s workload. FIFO queue ordering of all remaining orders is strictly preserved.
