# College Project Demonstration Script (5–7 Minutes)

**Project**: AI-Based College Canteen Queue Optimization System  
**Presenter**: Student / Engineering Candidate  
**Target Audience**: Course Professor, External Viva Examiner, Project Evaluator

---

## ⏱️ Demonstration Timeline Summary

| Time | Phase | Target Screen / Component | Key Talking Point |
|---|---|---|---|
| **0:00 - 0:45** | **1. System Startup** | 2 Terminals (Backend + Frontend) | Start C++ REST API daemon on port 8080 and React dev server on port 5173. |
| **0:45 - 1:45** | **2. Student Ordering** | Browser `localhost:5173` (Menu Tab) | Browse 8 catalog items, filter by Beverages, search "Burger", adjust cart quantities. |
| **1:45 - 2:30** | **3. AI Scheduling** | Order Confirmation Modal | Explain returned counter assignment and Random Forest preparation prediction. |
| **2:30 - 3:15** | **4. Live Queue Telemetry**| "Live Queues" Tab | Show 4 counter cards (`FixedArray`), active workloads, and load balancing telemetry. |
| **3:15 - 4:15** | **5. Order Lifecycle** | "Track Order" + "Admin Ops" Tab | Progress order through state machine: `QUEUED` → `PREPARING` → `READY` → `COLLECTED`. |
| **4:15 - 5:00** | **6. Admin Order Listing**| Admin Persisted Orders Table | Demonstrate `GET /api/orders` SQLite query, status filter pills, and pagination. |
| **5:00 - 5:45** | **7. DSA Code Walkthrough**| Visual Studio / VS Code Editor | Point to exact source files implementing all 7 syllabus DSA concepts. |
| **5:45 - 6:30** | **8. Tests & ML Disclaimer**| Terminal (Test Runner) | Execute 58 backend tests + 10 browser QA flows; explain synthetic ML bounds. |

---

## 🚀 Step-by-Step Execution Guide

### Phase 1: Launch Environment (0:00 - 0:45)

**Terminal 1 (Backend C++ REST API)**:
```cmd
build\Debug\canteen_app.exe --server 8080
```
*Expected Console Output*:
```text
[REST API SERVER] Listening on http://0.0.0.0:8080
[REST API SERVER] SQLite Database connected: canteen.db
[REST API SERVER] ML Predictor active: RandomForestRegressor JSON (20 trees)
```

**Terminal 2 (Frontend React SPA)**:
```cmd
cd frontend
npm run dev
```
*Action*: Open Google Chrome at `http://localhost:5173`. Point to the top-right green status indicator: **"C++ Backend Online"**.

---

### Phase 2: Food Menu & Cart Interaction (0:45 - 1:45)

1. **Browse Food Catalog**: Show that the food menu renders dynamically from `GET /api/menu` (Veg Cheese Burger ₹75, Paneer Kathi Roll ₹90, Masala Dosa ₹60, Cold Coffee ₹45, etc.).
2. **Category Filter**: Click the **"Beverages"** pill. Point out that the grid filters down to Cold Coffee and Masala Chai. Click **"All"** to restore.
3. **Search Filter**: In the search bar, type `Burger`. Point out that the grid instantly isolates "Veg Cheese Burger". Clear the search bar.
4. **Cart Controls**:
   - On "Veg Cheese Burger", click **Add** (quantity becomes 1).
   - Click the **`+`** icon (quantity becomes 2).
   - Point to the floating bottom bar: *"2 items in order — ₹150.00"*.
   - Click **Review Order & Pay** to open the `CartDrawer`.

---

### Phase 3: Order Submission & Predictive Scheduling (1:45 - 2:30)

1. **Input Validation**:
   - Leave Student ID blank and click **Confirm & Place Order**.
   - Show the browser validation tooltip: *"Student ID is required"*.
2. **Order Placement**:
   - Enter Student ID: `STU-2026-DEMO`
   - Enter Student Name: `Priya Sharma`
   - Click **Confirm & Place Order**.
3. **Explain the Confirmation Modal**:
   - Point to the assigned counter: *"Counter #1: Express Fast Food"*.
   - Point to the predicted preparation duration: *"~10 minutes"*.
   - **Viva Explanation**:
     > *"When the order was placed, our C++ `CounterManager` traversed our custom `FixedArray` of 4 counters. For each counter, it evaluated a 20-tree Random Forest Regressor using order quantity, food category, preparation complexity, rush hour, and the counter's existing workload. It selected Counter 1 because it minimized total projected wait time."*

---

### Phase 4: Live Queue Telemetry (2:30 - 3:15)

1. Click **Done** on the modal and navigate to the **"Live Queues"** tab.
2. Show the 4 counter cards:
   - Counter #1: Express Fast Food
   - Counter #2: Main Meals
   - Counter #3: Beverages & Cafe
   - Counter #4: Special Counter
3. Point out that Counter 1 now shows **1 pending order** and an active workload of **10 minutes**.
4. Click the **Refresh** button to demonstrate dynamic polling via `GET /api/counters` and `GET /api/analytics`.

---

### Phase 5: Order Lifecycle Progression (3:15 - 4:15)

1. Navigate to the **"Track Order"** tab.
   - Enter the Order ID from Phase 3 (e.g. `107`). Click **Track**.
   - Show the 4-stage visual progress stepper:
     - `[Active: Order Queued]` → `[Upcoming: Cooking]` → `[Upcoming: Ready]` → `[Upcoming: Collected]`
2. Open the **"Admin Ops"** tab:
   - In the Persisted Orders table, locate Order #107. Click **Manage**.
   - Show the Action Console matrix with 4 state transition buttons:
     1. Click **1. Start Cooking**: Order status updates to `PREPARING`.
     2. Click **2. Mark Ready**: Order pops from Counter 1's FIFO queue; status becomes `READY`.
     3. Click **3. Collect Order**: Meal handed to student; status becomes `COLLECTED`.
3. Demonstrate State Invariant Rejection:
   - Point out that **Start Cooking** is now disabled because a collected order cannot be restarted.

---

### Phase 6: Admin Order Listing & SQLite Queries (4:15 - 5:00)

1. In the **"Admin Ops"** tab, scroll to the **Persisted Orders** table.
2. **Status Filter Pills**:
   - Click **"Queued"**: Table filters to only orders currently waiting in line.
   - Click **"Cooking"**: Table filters to orders currently in preparation.
   - Click **"All Orders"**: Restores the complete historical audit log.
3. **Pagination Controls**:
   - Point to the bottom pagination bar: *"Showing 1 to 10 of N orders — Page 1 of X"*.
   - Click **Next Page** and **Previous Page** to prove SQLite parameterized query execution (`LIMIT ? OFFSET ?`).

---

### Phase 7: DSA Code Architecture Walkthrough (5:00 - 5:45)

Switch to your IDE / editor and highlight the exact files implementing the 7 syllabus concepts:
1. [`include/canteen/ds/FixedArray.hpp`](include/canteen/ds/FixedArray.hpp): Statically sized, $O(1)$ bounds-checked counter collection.
2. [`include/canteen/ds/SinglyLinkedList.hpp`](include/canteen/ds/SinglyLinkedList.hpp): Pointer-based linked nodes with full Rule of Five compliance.
3. [`include/canteen/ds/Queue.hpp`](include/canteen/ds/Queue.hpp): FIFO queue enforcing first-come, first-served counter ordering.
4. [`include/canteen/ds/Stack.hpp`](include/canteen/ds/Stack.hpp): LIFO stack enabling administrative undo for menu mutations.
5. [`include/canteen/algorithms/Searching.hpp`](include/canteen/algorithms/Searching.hpp): Linear search across queues; binary search on sorted item IDs with strict precondition validation.
6. [`include/canteen/algorithms/Sorting.hpp`](include/canteen/algorithms/Sorting.hpp): In-place Bubble Sort with early-termination flag; stable divide-and-conquer Merge Sort.

---

### Phase 8: Verification & Academic ML Disclaimer (5:45 - 6:30)

1. **Run Backend Test Suite**:
   In a terminal, run:
   ```cmd
   build.bat
   ```
   Show the summary: **58 PASSED, 0 FAILED (100% Pass Rate)**.
2. **Run Headless Browser QA Automation**:
   ```cmd
   node scripts/qa_browser_e2e.mjs
   ```
   Show the summary: **10 / 10 Flows Passed** in headless Chrome.
3. **Final Viva Statement on ML**:
   > *"Our Random Forest model reduces preparation estimation error by 81.9% compared to the baseline mean. However, as documented in our project audit, this model was trained on a controlled synthetic dataset. In an actual physical deployment, real empirical timestamps from a kitchen display system would be required to validate real-world cooking variance."*

---

## 🧹 Safe Clean Recovery & Reset Procedure

If you ever need to reset the system during demonstrations without losing project files:

1. **Stop Background Servers**:
   In the terminal windows where `canteen_app.exe` or `npm run dev` are running, press `Ctrl + C`.
2. **Reset Database to Clean Default (Optional)**:
   If you want to clear test orders while preserving menu items and counters:
   ```cmd
   del canteen.db
   ```
   *Note*: When `canteen_app.exe` restarts, it will automatically recreate `canteen.db` and populate the default 8 menu items and 4 service counters.
3. **Rebuild Clean Binary**:
   ```cmd
   build.bat
   ```
