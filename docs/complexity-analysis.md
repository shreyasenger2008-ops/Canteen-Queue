# Comprehensive Complexity Analysis

This document provides rigorous asymptotic time and space complexity evaluations for every data structure and algorithm implemented in the **AI-Based College Canteen Queue Optimization System**.

---

## 1. Summary Complexity Matrix

| Component | Operation | Best Case | Average Case | Worst Case | Auxiliary Space | In-Place? |
|---|---|---|---|---|---|---|
| **FixedArray** | Indexed Access `at(i)` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Yes |
| | Append `add(x)` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Yes |
| | Linear Traversal | $O(n)$ | $O(n)$ | $O(n)$ | $O(1)$ | Yes |
| | Remove & Shift `removeAt(i)` | $O(1)$ (tail) | $O(n)$ | $O(n)$ (head) | $O(1)$ | Yes |
| **SinglyLinkedList** | Insert Front | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | No ($+1$ node) |
| | Insert Back (with tail) | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | No ($+1$ node) |
| | Delete Front | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Yes |
| | Delete by ID / Predicate | $O(1)$ (head) | $O(n)$ | $O(n)$ (tail/absent) | $O(1)$ | Yes |
| | Search / Traversal | $O(1)$ (head) | $O(n)$ | $O(n)$ | $O(1)$ | Yes |
| **Queue** | `enqueue(item)` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | No ($+1$ node) |
| | `dequeue()` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Yes |
| | `front()` / `rear()` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Yes |
| | `isEmpty()` / `size()` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Yes |
| | `removeIf` (Cancellation) | $O(1)$ | $O(n)$ | $O(n)$ | $O(1)$ | Yes |
| **Stack** | `push(action)` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | No ($+1$ node) |
| | `pop()` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Yes |
| | `peek()` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Yes |
| | `isEmpty()` / `size()` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Yes |
| | Administrative Undo | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Yes |
| **Searching** | Linear Search (ID) | $O(1)$ | $O(n)$ | $O(n)$ | $O(1)$ | Yes |
| | Linear Search (Name) | $O(m)$ | $O(n \cdot m)$ | $O(n \cdot m)$ | $O(1)$ | Yes |
| | Binary Search (ID) | $O(1)$ | $O(\log n)$ | $O(\log n)$ | $O(1)$ | Yes |
| **Sorting** | Bubble Sort (Counters) | $O(n)$ | $O(n^2)$ | $O(n^2)$ | $O(1)$ | Yes (In-Place) |
| | Merge Sort (Items/Orders) | $O(n \log n)$ | $O(n \log n)$ | $O(n \log n)$ | $O(n)$ | No (Buffer) |

*$n$ is the number of items/orders/counters; $m$ is the average length of name strings.*

---

## 2. In-Depth Operational Derivations

### A. Arrays (`FixedArray<T, Capacity>`)
1. **Indexed Access ($O(1)$)**:
   Because elements are stored contiguously in memory, the address of index $i$ is calculated via:
   $$\text{Address}(A[i]) = \text{BaseAddress}(A) + i \times \text{sizeof}(T)$$
   This involves one multiplication and one addition, taking constant time $\Theta(1)$.
2. **Bounds Checking ($O(1)$)**:
   A single comparison `if (index >= currentSize)` runs in constant time $\Theta(1)$.
3. **Traversal ($O(n)$)**:
   Visiting each of the $n$ active elements to render workloads executes the loop exactly $n$ times.

---

### B. Singly Linked List (`SinglyLinkedList<T>`)
1. **Insertion at Ends ($O(1)$)**:
   - Front: Allocates node, sets `node->next = head`, updates `head = node`. Count of pointer operations is constant $\Theta(1)$.
   - Back: Utilizing the cached `tail` pointer, `tail->next = node` and `tail = node`. Constant time $\Theta(1)$ without needing to traverse from head to tail.
2. **Deletion by Value ($O(n)$)**:
   - Best Case: Target matches head node; 1 comparison and 0 pointer hops $\implies \Theta(1)$.
   - Worst Case: Target is the last node or does not exist; requires traversing all $n$ nodes $\implies \Theta(n)$.
   - Average Case: Target located near $n/2$ nodes $\implies \Theta(n/2) = \Theta(n)$.

---

### C. FIFO Queue (`Queue<T>`)
1. **Enqueue ($O(1)$)**:
   Inserts at the tail of the underlying singly linked list via the tail pointer. Time is strictly $\Theta(1)$.
2. **Dequeue ($O(1)$)**:
   Removes the node at the head of the list, adjusts `head = head->next`, and decrements size. Strict $\Theta(1)$ with no shifting of subsequent elements (unlike array-based queues).
3. **Space Complexity**:
   Total space is $O(n)$ where each order consumes $1 \times \text{ListNode}$ with pointer overhead ($8$ bytes on 64-bit systems).

---

### D. LIFO Stack (`Stack<T>`) & Undo Mechanism
1. **Push & Pop ($O(1)$)**:
   Operations alter only the `topNode` pointer:
   - Push: `newNode->next = topNode; topNode = newNode;` $\implies \Theta(1)$.
   - Pop: `temp = topNode; topNode = topNode->next; delete temp;` $\implies \Theta(1)$.
2. **Undo Operation ($O(1)$)**:
   An administrative undo consists of a single `stack.pop()` followed by direct assignment to the target item properties (`item->setIsAvailable(action.prevBoolValue)`). Hence, total undo execution time is $O(1)$.

---

### E. Searching Algorithms

#### Linear Search
$$\text{Comparisons}(n) = \sum_{i=1}^n i \cdot P(\text{target at } i)$$
Assuming uniform probability $P = 1/n$:
$$\mathbb{E}[\text{Comparisons}] = \frac{1}{n} \sum_{i=1}^n i = \frac{n(n+1)}{2n} = \frac{n+1}{2} = O(n)$$

#### Binary Search
At each iteration, the interval $[low, high]$ of size $k$ is halved:
$$k_{i+1} = \left\lfloor \frac{k_i}{2} \right\rfloor$$
The algorithm terminates when $k_m \le 1$.
$$\frac{n}{2^m} \le 1 \implies 2^m \ge n \implies m = \lceil \log_2 n \rceil$$
Hence, at most $\lceil \log_2 n \rceil$ iterations are executed, establishing $O(\log n)$ time complexity.

---

### F. Sorting Algorithms

#### Bubble Sort
- Outer loop runs $n-1$ times.
- In pass $i$, inner loop performs $(n - 1 - i)$ comparisons.
- Total comparisons in worst/average case:
  $$\sum_{i=0}^{n-2} (n - 1 - i) = \frac{(n-1)n}{2} = \frac{n^2 - n}{2} = O(n^2)$$
- **Early Termination Best Case**:
  With the `swapped` boolean flag, if the array is already sorted, inner loop performs $(n-1)$ comparisons in pass 0, detects `swapped == false`, and terminates immediately:
  $$\text{Best-Case Time} = \Theta(n)$$
- **Auxiliary Space**:
  All exchanges occur using scalar temporaries; auxiliary memory is strictly $O(1)$.

#### Merge Sort
- **Recurrence Relation**:
  $$T(n) = 2T(n/2) + \Theta(n)$$
- By the Master Theorem ($a = 2, b = 2, f(n) = \Theta(n)$, where $\log_b a = \log_2 2 = 1$):
  $$T(n) = \Theta(n^{\log_2 2} \log n) = \Theta(n \log n)$$
- **Auxiliary Space**:
  Merging requires copying elements into temporary subarrays `leftArr` of size $n/2$ and `rightArr` of size $n/2$:
  $$\text{Auxiliary Space} = \frac{n}{2} + \frac{n}{2} = \Theta(n)$$

---

## 3. Assumptions and Real-World Trade-Offs

1. **Fixed Counters**: The number of service counters is relatively static ($C \le 5$). Therefore, while sorting counters takes $O(C^2)$ asymptotically with Bubble Sort, $C^2 \le 25$ operations, which executes in sub-microsecond time.
2. **Queue Fairness**: Node-based FIFO queues guarantee true $O(1)$ operations regardless of queue depth, avoiding the occasional $O(n)$ reallocation spikes found in dynamic array implementations.
3. **Memory Locality vs Dynamic Allocation**:
   - `FixedArray` offers optimal CPU cache locality for frequent counter lookups.
   - `SinglyLinkedList` trades some cache line locality for immediate $O(1)$ insertions and zero reallocation copies.
