# C++ DSA & AI Queue System: 26 Viva Questions & Answers

**Course**: Data Structures & Algorithms (DSA) Course Project  
**Project**: AI-Based College Canteen Queue Optimization System  
**Implementation**: C++17, Custom DSA, Random Forest Inference, SQLite3, REST API, React SPA  

---

## Category 1: Custom Data Structures & Memory Management

### Q1: Why did you implement a custom `FixedArray` instead of using `std::vector`?
**Answer**:
In our canteen system, the physical infrastructure consists of a static, compile-time bounded number of service counters ($N = 4 \le \text{Capacity} = 5$). 
1. **Physical Modeling**: Counters are physical stalls that do not dynamically grow at runtime. A fixed-capacity array models this physical reality directly.
2. **Deterministic Performance**: Stack allocation avoids heap fragmentation, eliminates pointer indirection, and guarantees zero dynamic reallocation spikes ($O(1)$ guaranteed insertion vs amortized $O(1)$ in `std::vector`).
3. **Memory Safety**: Our `FixedArray` provides explicit bounds checking with `std::out_of_range` exceptions on out-of-bounds access.

---

### Q2: How does your `SinglyLinkedList` implement the C++ "Rule of Five", and why is it necessary?
**Answer**:
Because `SinglyLinkedList<T>` manages raw heap memory (`ListNode<T>*` allocated via `new`), adhering to the **Rule of Five** prevents memory leaks, dangling pointers, and double-free crashes:
1. **Destructor**: Iteratively calls `clear()` to `delete` every heap node when the list leaves scope.
2. **Copy Constructor**: Performs a deep copy, allocating brand new nodes with identical values in a separate heap region.
3. **Copy Assignment Operator**: Uses the copy-and-swap idiom to ensure strong exception safety and prevent self-assignment memory corruption.
4. **Move Constructor**: Transfers the raw `head`, `tail`, and `count` pointers from the temporary source object to the new object in $O(1)$ time, setting the source's pointers to `nullptr`.
5. **Move Assignment Operator**: Deallocates existing nodes and adopts the source's pointers in $O(1)$ time without memory allocations.

---

### Q3: How does your FIFO `Queue` enforce First-Come, First-Served fairness?
**Answer**:
Our custom `Queue<T>` wraps `SinglyLinkedList<T>` using composition:
- **Enqueue (`enqueue`)**: Appends incoming orders strictly to the `tail` of the list using our cached tail pointer in $O(1)$ time.
- **Dequeue (`dequeue`)**: Pops orders strictly from the `head` of the list in $O(1)$ time.
- **Front Inspection (`front`)**: Inspects the order at `head`.
Because items are only added at the back and only served from the front, earlier arrivals are always processed before later arrivals. Bypassing the front order is prohibited by the C++ state machine, which returns `HTTP 409 Conflict` if kitchen staff attempts to start an order that is not at `pendingOrders.front()`.

---

### Q4: How does order cancellation work in your queue, and how does it avoid memory shifting?
**Answer**:
Cancellation is implemented in `Queue<T>::removeIf(Predicate&&)` via pointer unlinking on the underlying linked list:
- The algorithm traverses the singly linked list using two pointers (`curr` and `prev`).
- When an order matching the target ID is found, the previous node's pointer is updated to skip the target node: `prev->next = curr->next`.
- If the cancelled order was at the tail, `tail` is updated to `prev`.
- The target node is deallocated via `delete curr`.
- **Contrast with Arrays**: In an array-based queue, removing an element from the middle requires shifting all subsequent elements left by one index ($O(n)$ data movement). In our node-based queue, only two pointers are updated without moving any remaining order objects in memory.

---

### Q5: How does your `Stack` implement administrative undo, and why can't it undo an arbitrary historical order?
**Answer**:
`Stack<T>` operates on strict **Last-In, First-Out (LIFO)** semantics. In `MenuManager`:
- Every administrative mutation (e.g. toggling food availability or updating price) pushes an `AdminAction` struct onto `undoStack` in $O(1)$ time.
- When `undoLastAdminAction()` is called, the most recent action is popped from the top of the stack in $O(1)$ time, and its `previousState` is applied to revert the change.
- **Why it cannot undo arbitrary orders**: A stack only provides access to its top element (`top()`). Undoing an order placed 10 minutes ago when other orders have since been queued would violate LIFO invariants, corrupt sequential dependencies, and require popping intervening actions. Arbitrary order cancellation is therefore handled by `Queue::removeIf`, not by the stack.

---

## Category 2: Algorithm Complexity, Searching & Sorting

### Q6: What is the exact time complexity of indexed access in `FixedArray`, and why is it $O(1)$?
**Answer**:
Indexed access `at(i)` is strictly $\Theta(1)$ constant time because array elements are stored in contiguous memory addresses. The CPU computes the physical address in a single arithmetic operation:
$$\text{Address}(A[i]) = \text{BaseAddress}(A) + i \times \text{sizeof}(T)$$
This requires exactly one multiplication and one addition regardless of whether $i=0$ or $i=1,000,000$.

---

### Q7: Compare the real-world trade-offs between your Linear Search and Binary Search implementations.
**Answer**:
In [`Searching.hpp`](include/canteen/algorithms/Searching.hpp):
- **Linear Search (`linearSearchOrderById`)**:
  - Time: Best $O(1)$, Worst $O(n)$. Space: $O(1)$.
  - Traverses sequentially from head to tail. It works on unsorted linked lists and supports substring pattern matching on food names (e.g., searching "Burger" inside "Veg Cheese Burger").
- **Binary Search (`binarySearchFoodItemById`)**:
  - Time: Best $O(1)$, Worst $O(\log n)$. Space: $O(1)$.
  - Operates on contiguous arrays. In each iteration, it eliminates half of the remaining search interval: `mid = low + (high - low) / 2`.
  - **Trade-off**: Binary search cannot run on linked lists without losing logarithmic efficiency (traversing to the middle node of a linked list takes $O(n)$ time, making list binary search $O(n)$). It strictly requires contiguous indexing and pre-sorted data.

---

### Q8: Why does your Binary Search verify that data is sorted, and what happens mathematically if data is unsorted?
**Answer**:
Our implementation explicitly validates the `isSorted(items)` precondition:
```cpp
if (!isSorted(items)) {
    throw std::invalid_argument("Binary search requires array to be sorted by ID");
}
```
**Mathematical reason**: Binary search relies on the **Monotonicity Principle**:
$$\text{If } \text{target} < \text{arr}[\text{mid}], \quad \text{then } \forall k \ge \text{mid}, \, \text{arr}[k] > \text{target}$$
This assumption allows the algorithm to safely discard the right half `[mid ... high]`. If the array is unsorted, the target element could be located in the discarded partition, causing the algorithm to report that the item does not exist even when it is present.

---

### Q9: Why did you choose Bubble Sort for service counters, and what optimizations were included?
**Answer**:
Bubble Sort was chosen specifically for the service counters collection in `CounterManager`:
1. **Dataset Size**: There are only $N = 4$ counters in the canteen. For $N = 4$, $N^2 = 16$ comparisons is negligible and executes in nanoseconds.
2. **In-Place Execution**: Auxiliary space is strictly $O(1)$ with zero memory allocations.
3. **Optimizations**:
   - **Early Termination Flag (`swapped`)**: If a pass executes with zero swaps, the array is already sorted and terminates immediately in $O(n)$ best-case time.
   - **Deterministic Tie-Breaking**: If two counters have identical wait times, ties are broken using counter ID ascending (`idA > idB`), ensuring stable, predictable ordering.

---

### Q10: Why did you choose Merge Sort for menu items and order records instead of Bubble Sort?
**Answer**:
Unlike the 4 physical counters, food menu items and historical order archives can grow into hundreds or thousands of records ($N \ge 1,000$):
1. **Guaranteed Time Complexity**: Bubble Sort degrades to $O(n^2)$ ($1,000^2 = 1,000,000$ operations). Merge Sort guarantees $\Theta(n \log n)$ time in all cases (best, average, worst) using divide-and-conquer.
2. **Stability**: Merge Sort is inherently stable. When sorting historical orders by preparation duration, orders with identical preparation times maintain their original chronological arrival order (`!comp(right[j], left[i])`).
3. **Trade-off**: Merge Sort requires $O(n)$ temporary buffer space to merge subarrays.

---

## Category 3: Machine Learning Training, Inference & Auditing

### Q11: What machine learning model was chosen, and what are the 5 input features?
**Answer**:
We trained a **`RandomForestRegressor`** (20 estimators, maximum depth 6, random state 42) using `scikit-learn`. The 5 input features are:
1. `food_category` ($0$: Fast Food, $1$: South Indian, $2$: Meals/Chinese, $3$: Beverages, $4$: Bakery/Snacks)
2. `order_quantity` (integer from $1$ to $5$ items)
3. `preparation_complexity` (ordinal integer from $1$ to $5$)
4. `time_of_day_hour` (continuous decimal hour, e.g. $12.50 = 12:30\text{ PM}$)
5. `current_kitchen_workload` (continuous minutes currently pending at the evaluated counter, $0.0$ to $30.0$)
The target variable is `observed_prep_time_minutes`.

---

### Q12: What were the evaluation metrics, and how did the model compare to the baseline?
**Answer**:
On a held-out test set of 500 samples (20% split):
- **Baseline (Training-Set Mean Predictor, $\bar{y} = 9.73\text{m}$)**:
  - MAE: **4.5003 minutes**
  - RMSE: **5.5477 minutes**
- **Trained RandomForestRegressor**:
  - MAE: **0.8139 minutes** (**81.91% error reduction**)
  - RMSE: **1.0411 minutes** (**81.23% error reduction**)
  - $R^2$ Score: **0.9648** (explains 96.48% of preparation duration variance)

---

### Q13: How is machine learning inference executed in pure C++ without Python or ONNX Runtime?
**Answer**:
In [`RandomForestPredictor.cpp`](src/ml/RandomForestPredictor.cpp):
1. During startup, C++ parses `ml/model.json` into an in-memory `std::vector<DecisionTree>`.
2. Each decision tree is represented as a contiguous array of `Node` structs storing `feature`, `threshold`, `leftChild`, `rightChild`, and `value`.
3. To predict, each tree is traversed iteratively in $O(\text{depth}) = O(6)$ time:
   ```cpp
   curr = (features[node.feature] <= node.threshold) ? node.leftChild : node.rightChild;
   ```
4. The ensemble prediction is the arithmetic mean across all 20 trees: $\hat{y} = \frac{1}{20} \sum \text{Tree}_i(\mathbf{x})$.
5. **Precision**: Verified by unit test `ML_CompareWithPythonHeldOutPredictions` to match Python's floating-point outputs to within floating-point tolerance ($< 10^{-6}$).

---

### Q14: What is the academic disclaimer regarding your synthetic dataset?
**Answer**:
The dataset `ml/canteen_prep_data.csv` was generated programmatically (`ml/generate_dataset.py`, $N = 2,500$ samples, seed 42) using mathematical operational rules (category baselines, sub-linear quantity scaling, rush hour penalties, workload delays). 
**It does NOT represent empirical sensor observations from an operating commercial canteen.**
While synthetic evaluation validates mathematical convergence, pipeline integration, and scheduling load-balancing, real-world deployment would require empirical timestamps collected from a Kitchen Display System (KDS) to capture unmodeled human and thermal factors.

---

### Q15: What is the fallback behavior if `model.json` is missing or corrupted at runtime?
**Answer**:
Our C++ engine implements multi-layered fault tolerance:
1. **Missing File**: `RandomForestPredictor::loadFromJsonFile()` catches the file error, logs a warning, and automatically switches to pre-compiled tree constants in [`EmbeddedModelData.hpp`](include/canteen/models/EmbeddedModelData.hpp) or returns the training set mean ($9.73$m).
2. **Invalid Inputs**: `predictRaw()` verifies vector length $\ge 5$ and tests for `NaN` / `Infinity` via `std::isnan` and `std::isinf`. If invalid, `CounterManager` falls back to the static menu preparation time sum.
3. **Physical Clamping**: Every prediction is clamped to $\ge 1.0$ minute to prevent zero or negative cooking estimates.

---

## Category 4: Scheduling Engine & Queue Load-Balancing

### Q16: How does the AI predictive scheduler decide which counter receives an order?
**Answer**:
In `CounterManager::allocateOrderToCounter`:
1. The scheduler iterates through all available counters in the `FixedArray` ($O(C)$ where $C=4$).
2. For each candidate counter $C_i$, it constructs feature vector $\mathbf{x} = [\text{category}, \text{qty}, \text{complexity}, \text{hour}, \text{workload}(C_i)]$.
3. Evaluates $T_{\text{pred}}(O, C_i)$ via `RandomForestPredictor`.
4. Calculates **Projected Wait Time**:
   $$\text{ProjectedCompletion}(C_i) = \text{Workload}(C_i) + T_{\text{pred}}(O, C_i)$$
5. Selects the counter $C^*$ that minimizes Projected Wait Time. If workloads are tied, the counter with the lower numeric ID is selected deterministically.
6. Enqueues the order to $C^*$'s FIFO queue and adds $T_{\text{pred}}$ to $C^*$'s workload.

---

### Q17: If an order with a 2-minute prep time arrives after an order with a 10-minute prep time at the same counter, which order is served first?
**Answer**:
**The 10-minute order is served first.**
Our system enforces strict **FIFO queue ordering** within each counter's line. The machine learning model is used exclusively for **counter allocation** (directing orders to the counter with the shortest projected queue) and **accurate wait time countdowns**. It **never reorders** orders inside a counter's queue. Bypassing the earlier order would constitute queue jumping and violate customer fairness.

---

### Q18: What happens to a counter's workload when an order is completed or cancelled?
**Answer**:
Workload is strictly synchronized:
- When an order transitions to `READY` via `Counter::completeFrontOrder()`, its preparation duration is subtracted from `current_workload_minutes`:
  $$\text{Workload}(C) \leftarrow \max(0, \, \text{Workload}(C) - \text{order.prep\_time})$$
- When an order is cancelled via `CanteenSystem::cancelOrder()`, `Queue::removeIf` unlinks the order node, and its duration is immediately deducted from the counter's workload. Completed and cancelled orders never contribute to active kitchen wait times.

---

## Category 5: SQLite Persistence & REST API Design

### Q19: Which is authoritative: in-memory C++ DSA structures or SQLite database records?
**Answer**:
**The in-memory C++ DSA structures are authoritative for active scheduling operations.**
- High-frequency operations (queueing, FIFO pop, linear search across front orders, workload math) execute directly in memory with $O(1)$ and $O(n)$ efficiency, completely bypassing database disk I/O.
- SQLite acts as the **durable persistence and audit tier**. Every state transition (`placeOrder`, `startOrder`, `readyOrder`, `collectOrder`, `cancelOrder`) is asynchronously mirrored to SQLite via prepared statements and atomic transactions.

---

### Q20: How does the system recover active counter queues after an unexpected server crash or restart?
**Answer**:
During startup, `CanteenSystem` executes the **Crash Recovery Protocol** in [`DatabaseManager::loadActiveOrders`](src/db/DatabaseManager.cpp):
1. Queries SQLite for orders with status `QUEUED` or `PREPARING`, ordered chronologically:
   `SELECT * FROM orders WHERE status IN ('QUEUED', 'PREPARING') ORDER BY order_timestamp ASC`.
2. For each recovered order, queries its line items from `order_items`.
3. Enqueues each order back into its designated counter's in-memory FIFO `Queue<Order>`.
4. Re-accumulates the counter's `current_workload_minutes`.
5. Historical orders (`READY`, `COLLECTED`, `CANCELLED`) are loaded into `OrderManager::orderHistory`.
6. Sets `nextOrderId` to $\max(\text{existing IDs}) + 1$ to prevent ID collisions.

---

### Q21: What is an Idempotency Key, and how does your API prevent duplicate orders?
**Answer**:
An idempotency key is a client-generated unique identifier (UUID v4) sent in the HTTP request payload:
- In SQLite, the `orders` table has an explicit constraint: `idempotency_key TEXT UNIQUE`.
- If a student submits an order, but a network glitch drops the HTTP response, the client retries with the same `idempotency_key`.
- In `DatabaseManager::saveOrderWithItems`, the server first queries `findOrderByIdempotencyKey(key)`.
- If found, it skips queue insertion and returns the already-created order with `HTTP 200 OK` instead of creating a duplicate order and charging the student twice.

---

### Q22: Why does `POST /api/orders/{id}/start` return `HTTP 409 Conflict` instead of `HTTP 400 Bad Request` on invalid state transitions?
**Answer**:
Under RFC 9110 HTTP standards:
- `400 Bad Request` indicates a syntactically invalid client request (e.g. malformed JSON, negative quantity, missing student name).
- `409 Conflict` indicates that the request is syntactically valid, but cannot be processed due to a **conflict with the current state of the resource**.
If Order #102 is currently `QUEUED` behind Order #101, Order #102 cannot start cooking until Order #101 is finished. Returning `HTTP 409 Conflict` explicitly conveys that the request violates business state machine rules.

---

### Q23: How are pagination and status filtering implemented in `GET /api/orders`?
**Answer**:
In [`DatabaseManager::loadOrdersPaginated`](src/db/DatabaseManager.cpp):
- Query parameters `limit` (default: 20, max: 100), `offset` (default: 0), and optional `status` are validated in `HttpServer.cpp`.
- Executes parameterized SQL queries using `sqlite3_bind_int` and `sqlite3_bind_text`:
  ```sql
  SELECT id, student_id, student_name, status, assigned_counter_id, total_prep_time_minutes, total_amount, order_timestamp 
  FROM orders WHERE (? = '' OR status = ?) 
  ORDER BY order_timestamp DESC, id DESC LIMIT ? OFFSET ?
  ```
- Also queries `SELECT COUNT(*) FROM orders WHERE (? = '' OR status = ?)` to return the total matching count.
- Reconstructs line items for each order and returns structured JSON: `{"orders": [...], "total": N, "limit": L, "offset": O}`.

---

## Category 6: Full-Stack Integration & Architecture

### Q24: How does the React frontend maintain real-time queue awareness with the backend?
**Answer**:
The React frontend uses standard REST polling:
- On the **Live Queues** dashboard, a `setInterval` hook polls `GET /api/counters` and `GET /api/analytics` every 5 seconds.
- Counter cards dynamically update their queue depth badges (`(3q)`), active preparation workload progress bars, and operational status indicators.
- When an admin executes a state transition (e.g. `Mark Ready`), the frontend triggers an immediate telemetry re-fetch to reflect decremented workloads without waiting for the 5-second interval.

---

### Q25: How does your system prevent SQL injection?
**Answer**:
In `DatabaseManager.cpp`, string concatenation is strictly prohibited for SQL queries. All database operations use **SQLite prepared statements** (`sqlite3_prepare_v2`) and parameter binding:
- Integers are bound using `sqlite3_bind_int(stmt, index, value)`.
- Strings are bound using `sqlite3_bind_text(stmt, index, str.c_str(), -1, SQLITE_TRANSIENT)`.
- Floats are bound using `sqlite3_bind_double(stmt, index, val)`.
User inputs such as `student_id` or `student_name` containing SQL injection payloads (e.g. `' OR 1=1; --`) are treated as literal string parameters by the SQLite engine, preventing injection attacks.

---

### Q26: What are the primary technical limitations of the current system, and how would you enhance it?
**Answer**:
1. **Multi-Item Counter Specialization**:
   - *Current*: A multi-item order is assigned in its entirety to a single counter based on its primary category.
   - *Improvement*: Implement sub-order splitting where a Burger is prepared at Counter 1 and Cold Coffee is simultaneously prepared at Counter 3, with an order aggregation barrier.
2. **Real-World Empirical Retraining**:
   - *Current*: ML model trained on synthetic data.
   - *Improvement*: Integrate with actual Kitchen Display System (KDS) bump bars to collect physical cooking durations and retrain weekly.
3. **Real-Time Push Notifications**:
   - *Current*: Polling every 5 seconds via HTTP GET.
   - *Improvement*: Implement WebSockets (`ws://`) or Server-Sent Events (SSE) in `cpp-httplib` for instant zero-latency kitchen order dispatch.


---

### Q27: How is staff authentication implemented, and why is client-side route protection alone insufficient?
**Answer**:
1. **The Vulnerability of Frontend-Only Protection**:
   Hiding an admin dashboard behind React state, `sessionStorage`, or hardcoded PIN checks (`if (pin === "1234")`) provides zero security against an attacker or curious student who can inspect frontend JavaScript bundles, use curl/Postman, or execute API calls directly in the browser console.
2. **Server-Side Authorization**:
   - In `HttpServer.cpp`, all mutating staff actions (`/start`, `/ready`, `/collect`, `/cancel`, `/pay`, `/accept`) are protected by `isStaffAuthorized()`.
   - The server verifies an `Authorization: Bearer <token>` header against a server-side secret configurable via `CANTEEN_STAFF_TOKEN`.
   - The token is retrieved via `POST /api/staff/auth` with a valid PIN configured in `CANTEEN_STAFF_PINS`.
   - Unauthorized requests immediately return `HTTP 401 Unauthorized` before any C++ state or database mutation occurs.

---

### Q28: How is order payment persistence implemented, and why is manual counter confirmation preferred over a mocked payment gateway?
**Answer**:
1. **SQLite Persistence**:
   - The `orders` table contains a dedicated `payment_status TEXT NOT NULL DEFAULT 'PENDING'` column.
   - Calling `POST /api/orders/{id}/pay` executes `db.updateOrderPaymentStatus(orderId, "PAID")` via a prepared statement (`UPDATE orders SET payment_status = ? WHERE id = ?`).
   - The status is persisted to disk, loaded in `loadOrderById` and `loadOrdersPaginated`, and survives complete server restarts.
   - Payment cannot be confirmed for `CANCELLED` orders (`HTTP 409 Conflict`).
2. **Design Honesty**:
   In a campus canteen environment, students typically pay with cash or direct UPI QR codes at the counter. Mocking a fake third-party gateway (e.g. Razorpay/Stripe) would introduce synthetic complexity without real-world utility; an honest staff-verified counter payment workflow directly matches the operational reality of college canteens.

---

### Q29: What was the architectural rationale for the "Accept Order" action, and why didn't you add an "ACCEPTED" status to the core lifecycle?
**Answer**:
1. **State Machine Integrity**:
   The core lifecycle is `QUEUED` $\rightarrow$ `PREPARING` $\rightarrow$ `READY` $\rightarrow$ `COLLECTED` (or `CANCELLED`). An order placed in the canteen is immediately queued into a physical counter's FIFO line.
2. **Avoiding Redundant States**:
   Introducing an artificial `ACCEPTED` state between `QUEUED` and `PREPARING` would create ambiguity regarding counter workload calculations and FIFO queue positions.
3. **Honest Acknowledgement**:
   `POST /api/orders/{id}/accept` verifies the order is in `QUEUED` status and writes an `ORDER_ACKNOWLEDGED` event to the `admin_audit_log` table. The order remains `QUEUED` in its counter's queue, preserving exact FIFO order and ML workload allocation, while providing kitchen staff with a formal acknowledgment action.

---

### Q30: How does the system ensure FIFO queue ordering both in the backend API and the frontend dashboard?
**Answer**:
1. **Backend Queue DSA**:
   Each counter maintains a custom `Queue<Order>` backed by a singly linked list with $O(1)$ tail enqueue and $O(1)$ head dequeue. Orders arrive and depart in strict FIFO sequence.
2. **Status-Conditional Query Sorting**:
   In `DatabaseManager::loadOrdersPaginated`:
   - For active kitchen lines (`QUEUED` and `PREPARING`), queries are sorted `ORDER BY order_timestamp ASC, id ASC` so kitchen staff always see the earliest-placed orders at the top of their workspace.
   - For historical views (`COLLECTED`, `CANCELLED`, `ALL`), queries sort `ORDER BY order_timestamp DESC, id DESC` to show recent history first.
3. **Frontend Visual Feedback**:
   The staff dashboard displays dynamic queue position badges (`#1`, `#2`, `#3`...) for queued orders and marks the first order with a prominent `▶ NEXT` indicator.
