# AI-Based College Canteen Queue Optimization System

An advanced C++ Data Structures & Algorithms (DSA), Machine Learning, Relational Persistence, and REST API system engineered to optimize college canteen queues, predict dynamic food preparation durations using a trained `RandomForestRegressor`, balance kitchen workloads across counters, persist durable state with SQLite, and serve client applications via a multithreaded REST API.

---

## 📋 Course & Syllabus Alignment

All 7 core DSA topics required by the syllabus are implemented from scratch with zero reliance on standard-library container adapters (`std::queue`, `std::stack`, `std::list`):

| # | DSA Syllabus Topic | Custom Implementation Component | Purpose & Meaningful Application |
|---|---|---|---|
| 1 | **Arrays** | [`FixedArray<T, Capacity>`](include/canteen/ds/FixedArray.hpp) | Fixed collection of canteen service counters; $O(1)$ bounds-checked indexed access, array traversal, and workload updates. |
| 2 | **Linked Lists** | [`SinglyLinkedList<T>`](include/canteen/ds/SinglyLinkedList.hpp) | Raw pointer singly linked list with Rule of Five compliance; underlying dynamic storage for queues and history tracking. |
| 3 | **Queues** | [`Queue<T>`](include/canteen/ds/Queue.hpp) | Custom FIFO queue with $O(1)$ front and rear operations; maintains independent queue per counter and enforces first-in, first-out order processing. |
| 4 | **Stacks** | [`Stack<T>`](include/canteen/ds/Stack.hpp) | Custom LIFO stack using linked nodes; manages admin action history and enables $O(1)$ undo functionality for menu availability and pricing changes. |
| 5 | **Searching** | [`Searching.hpp`](include/canteen/algorithms/Searching.hpp) | **Linear Search** ($O(n)$) for orders by ID and items by name/ID; **Binary Search** ($O(\log n)$) on sorted item IDs with strict precondition sorting validation. |
| 6 | **Sorting** | [`Sorting.hpp`](include/canteen/algorithms/Sorting.hpp) | **Bubble Sort** ($O(n^2)$ in-place) to sort counters by estimated wait times; **Merge Sort** ($O(n \log n)$ stable) to sort menu items and orders by prep time or price. |
| 7 | **Basic DSA Concepts** | System Architecture | Classes, encapsulation, raw pointers, dynamic memory management (`new`/`delete`), Rule of Five (copy/move), state machines, and Big-O complexity. |

---

## 📚 Comprehensive Documentation Library

The project contains complete technical documentation and audit files in the [`docs/`](docs/) directory:

1. **[DSA Traceability & Syllabus Alignment](docs/dsa-traceability.md)**: Concrete mapping of all 7 syllabus topics to C++ source classes, real asymptotic time/space complexities, canteen workflow examples, and design trade-offs.
2. **[Architecture & Data Flow](docs/architecture-and-data-flow.md)**: 4-tier system architecture, end-to-end Mermaid sequence diagrams, state machine, and crash recovery protocol.
3. **[ML Model Audit & Real-World Applicability](docs/ml-audit.md)**: Complete audit of the `RandomForestRegressor`, input features, synthetic dataset assumptions, baseline comparisons, and real-world KDS sensor requirements.
4. **[5–7 Minute Viva Demonstration Script](docs/demo-script.md)**: Chronological presentation script, talking points, exact commands, and safe recovery procedures.
5. **[30 Viva Questions & Answers](docs/viva-questions.md)**: Code-grounded answers to likely examiner questions on DSA, ML, SQLite, API, auth security, and state invariants.
6. **[C++ REST API Reference](docs/api.md)**: Schemas, query parameters, CORS policy, Bearer token authentication, and HTTP status code conventions for all 14 endpoints.
7. **[Database Schema & Crash Recovery](docs/database.md)**: SQLite relational schema, payment persistence, foreign keys, prepared statements, and sequence alignment.
8. **[Verification & Testing Report](docs/testing-report.md)**: Full verification report covering 68 compiled backend tests and 10 automated browser QA user journeys.
9. **[Algorithmic Derivations & Complexity](docs/complexity-analysis.md)**: Formal mathematical proofs of $O(1)$, $O(\log n)$, $O(n)$, and $O(n \log n)$ bounds.

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

## 🗄️ SQLite Persistence & State Recovery (Phase 3)

The project includes an embedded SQLite 3 relational database layer (`DatabaseManager.cpp`) operating in synchrony with the in-memory DSA scheduling engine:
- **Relational Schema**: `menu_items`, `counters`, `orders`, `order_items`, `admin_audit_log`.
- **Integrity**: Primary keys, foreign key enforcement (`PRAGMA foreign_keys = ON;`), prepared parameter binding, and atomic transactions.
- **Deduplication**: `idempotency_key UNIQUE` constraint prevents duplicate orders on network retries.
- **Crash Recovery Protocol**: On application start, historical orders are recovered into `OrderManager::orderHistory`, sequence IDs are aligned to `max(orderId) + 1`, and active orders (`QUEUED`, `PREPARING`) are reconstructed into their assigned counter FIFO queues in chronological order without reassigning or duplicating workloads.
- **Documentation**: Detailed architecture guide in [`docs/database.md`](docs/database.md).

---

## 🌐 C++ REST API (Phase 3 & Security Pass)

A lightweight, multithreaded REST API server built using `cpp-httplib` and `nlohmann/json` exposes the C++ engine to web frontends (e.g., React dev server on `localhost:3000` / `localhost:5173`):
- **CORS Support**: Preflight `OPTIONS` and `Access-Control-Allow-Origin: *` configured across all endpoints.
- **Staff Authentication**: Server-side Bearer token authentication (`Authorization: Bearer <token>`). Staff logs in via `POST /api/staff/auth` with a server-validated PIN (`CANTEEN_STAFF_PINS`), returning a server token (`CANTEEN_STAFF_TOKEN`). Staff operational endpoints are protected from unauthenticated access.
- **Endpoints (14 Total)**:
  1. `GET /api/health` — Service health, database connection, and ML model status.
  2. `GET /api/menu` — Complete food item catalog with prices and availability.
  3. `GET /api/counters` — All counters, live pending order queues, and workloads.
  4. `POST /api/orders` — Submits order; invokes ML predictor, optimizes counter allocation, persists to SQLite.
  5. `GET /api/orders` — Paginated list of persisted orders with optional status filtering (`limit`, `offset`, `status`).
  6. `GET /api/orders/{id}` — Queries order details from in-memory queues, history, or DB.
  7. `POST /api/orders/{id}/start` — [Staff Protected] Kitchen staff action: advances front order to `PREPARING`.
  8. `POST /api/orders/{id}/ready` — [Staff Protected] Marks order `READY`, dequeues from cooking line, decrements counter workload.
  9. `POST /api/orders/{id}/collect` — [Staff Protected] Student collection: transitions order to `COLLECTED`.
  10. `POST /api/orders/{id}/cancel` — [Staff Protected] Emergency order cancellation with workload reclamation.
  11. `POST /api/orders/{id}/pay` — [Staff Protected] Cashier/Staff operation: confirms manual counter cash/UPI payment and persists `PAID` status to SQLite.
  12. `POST /api/orders/{id}/accept` — [Staff Protected] Staff action: formally acknowledges incoming queued order and records audit log without breaking FIFO.
  13. `GET /api/analytics` — Real-time telemetry, active queue pressure, total workload minutes.
  14. `POST /api/staff/auth` — Server-side staff PIN authentication, issuing bearer session token.
- **Documentation**: Complete API schemas and error codes in [`docs/api.md`](docs/api.md).

---

## 💻 React Frontend & Admin Management (Phase 4 & 5)

A responsive single-page application built using **React 19**, **TypeScript**, **Tailwind CSS v4**, and **Lucide Icons** connecting directly to the C++ REST API:
- **Student Ordering**: Real-time food item catalog with category filtering, instant search, quantity controls, cart drawer, and client-generated idempotency key deduplication.
- **Order Tracking**: Visual 4-stage lifecycle progress stepper (`QUEUED` → `PREPARING` → `READY` → `COLLECTED`), predicted preparation countdown, assigned counter indicator, and emergency cancellation.
- **Live Queues & Telemetry**: Dynamic counter cards showing queue depths, cooking workload minutes, active queue pressure, and background auto-polling.
- **Admin Order Management**:
  - Direct consumption of `GET /api/orders` with pagination (`limit`, `offset`) and status filtering (`All Orders`, `Queued`, `Cooking`, `Ready`, `Collected`, `Cancelled`).
  - Interactive Action Console for kitchen staff to transition orders through valid lifecycle steps.
  - Live system telemetry, active counter status indicators, and recent order history audit.
- **End-to-End Browser QA**: 10 automated test flows verified in headless Chrome via Chrome DevTools Protocol (CDP).

---

## 🏗️ Architecture & Project Structure

```
c:\SY\FODS CP FINAL\
├── CMakeLists.txt                         # Modern CMake build configuration (C & CXX)
├── build.bat                              # Automated Windows build & test runner
├── README.md                              # Comprehensive project documentation
├── ml/                                    # Machine learning training & export pipeline
│   ├── requirements.txt                   # scikit-learn, pandas, numpy
│   ├── generate_dataset.py                # Synthetic dataset generator (seed 42)
│   ├── train_model.py                     # RandomForestRegressor training script
│   ├── evaluate_model.py                  # Evaluation vs baseline generating metrics.json
│   ├── export_model.py                    # Exports trees to model.json & EmbeddedModelData.hpp
│   ├── canteen_prep_data.csv              # Synthetic dataset (2,500 rows)
│   ├── model.json                         # Serialized 20-tree Random Forest
│   ├── metrics.json                       # Evaluated MAE, RMSE, R^2, and feature importances
│   └── test_predictions.json             # Held-out samples for C++ vs Python cross-verification
├── third_party/                           # Self-contained header/amalgamation libraries
│   ├── sqlite/                            # SQLite3 amalgamation (sqlite3.h, sqlite3.c)
│   ├── httplib/                           # yhirose cpp-httplib (httplib.h)
│   └── nlohmann/                          # nlohmann Modern C++ JSON (json.hpp)
├── docs/
│   ├── dsa-traceability.md                # 100% traceability mapping syllabus concepts to source code & complexities
│   ├── architecture-and-data-flow.md      # 4-tier system architecture, Mermaid diagrams, state machine & recovery
│   ├── ml-audit.md                        # Machine learning audit, synthetic data disclosures & real-world KDS needs
│   ├── demo-script.md                     # Minute-by-minute 5–7 minute viva demonstration script & recovery
│   ├── viva-questions.md                  # 30 comprehensive viva questions with code-grounded answers
│   ├── api.md                             # REST API endpoints, schemas, CORS, Bearer auth & status codes
│   ├── database.md                        # SQLite schema, transactions, payment persistence & recovery
│   ├── algorithms.md                      # Data structures & predictive scheduling specifications
│   ├── complexity-analysis.md             # Asymptotic time and space complexity analysis
│   └── testing-report.md                  # Comprehensive verification report covering 68 tests and 10 E2E flows
├── include/
│   └── canteen/
│       ├── ds/                            # Syllabus DSA custom structures
│       │   ├── FixedArray.hpp             # Bounds-checked fixed array
│       │   ├── SinglyLinkedList.hpp       # Memory-safe singly linked list
│       │   ├── Queue.hpp                  # Custom FIFO Queue
│       │   └── Stack.hpp                  # Custom LIFO Stack
│       ├── algorithms/
│       │   ├── Searching.hpp              # Linear Search and Binary Search
│       │   └── Sorting.hpp                # Bubble Sort and Divide-and-Conquer Merge Sort
│       ├── ml/
│       │   ├── IPrepTimePredictor.hpp     # Abstract prediction interface and feature vector
│       │   └── RandomForestPredictor.hpp  # In-memory Random Forest inference engine
│       ├── db/
│       │   └── DatabaseManager.hpp        # SQLite relational persistence and state recovery
│       ├── api/
│       │   └── HttpServer.hpp             # C++ REST API server
│       ├── models/
│       │   ├── FoodItem.hpp               # FoodItem model (ID, price, prep time, complexity)
│       │   ├── Order.hpp                  # Order model with status lifecycle validation
│       │   ├── OrderStatus.hpp            # State machine: QUEUED -> PREPARING -> READY -> COLLECTED
│       │   ├── Counter.hpp                # Service counter holding its dedicated FIFO OrderQueue
│       │   ├── AdminAction.hpp            # Encapsulates administrative actions for stack undo
│       │   └── EmbeddedModelData.hpp      # Auto-generated zero-dependency compiled tree fallback
│       └── core/
│           ├── MenuManager.hpp            # Catalog management, search/sort, and undo stack
│           ├── CounterManager.hpp         # Counter workload optimization and allocation heuristic
│           ├── OrderManager.hpp           # Order placement, audit trail, and cross-queue search
│           └── CanteenSystem.hpp          # Unified facade, state recovery, and simulation coordinator
├── src/
│   ├── models/
│   ├── core/
│   ├── ml/
│   ├── db/
│   │   └── DatabaseManager.cpp            # SQLite transaction execution & crash recovery
│   ├── api/
│   │   └── HttpServer.cpp                 # cpp-httplib endpoints, CORS, JSON serialization
│   └── main.cpp                           # Entry point (--server [port], --demo, or interactive CLI)
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
    ├── test_ml.cpp                        # 7 Unit tests for ML inference & cross-language verification
    ├── test_database.cpp                  # 10 Unit tests for SQLite persistence & restart recovery
    └── test_api.cpp                       # 17 Integration tests for C++ REST API endpoints & auth
```

---

## ⚙️ Prerequisites & Toolchain

- **C++ Compiler**: C++17 or C++20 compliant compiler:
  - MSVC (Visual Studio 2019 / 2022 / 2026 Developer Command Prompt)
  - GCC / MinGW (g++ 7.0+)
  - Clang (clang++ 6.0+)
- **Build System**: CMake (version 3.15 or later)
- **Networking**: Winsock2 (`ws2_32`) and CryptoAPI (`crypt32`) on Windows (auto-linked via CMake).
- **Python**: Python 3.9+ with `scikit-learn`, `pandas`, `numpy` (for model training pipeline).

---

## 🚀 Build and Run Instructions

### 1. Build and Run All 68 Backend Tests via `build.bat` (Windows)

```cmd
build.bat
```
This builds `canteen_core.lib`, `canteen_app.exe`, and `canteen_tests.exe`, and executes all 68 unit and integration tests.

### 2. Run End-to-End Browser QA Automation (10 Flows)

```cmd
node scripts/qa_browser_e2e.mjs
```
Automates verification across all 10 frontend-backend user journeys in headless Chrome.

### 2. Start the C++ REST API Server (Terminal 1)

```cmd
build\Debug\canteen_app.exe --server 8080
```
Starts the C++ REST API server on `http://localhost:8080` with SQLite persistence enabled in `canteen.db`.

### 3. Start the React Frontend (Terminal 2)

```cmd
cd frontend
npm install
npm run dev
```
Starts the Vite React development server on `http://localhost:5173`. Open your browser to access the complete interactive student ordering, live queue telemetry, order tracking, and kitchen operations dashboard.

To build the frontend for production:
```cmd
cd frontend
npm run build
npm run preview
```

### 4. Run Automated DSA & ML Demonstration

```cmd
build\Debug\canteen_app.exe --demo
```

### 5. Interactive Console Application

```cmd
build\Debug\canteen_app.exe
```
Provides an interactive menu for students and canteen administrators.
