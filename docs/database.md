# SQLite Persistence & State Recovery Architecture

## 1. Overview

The **AI-Based College Canteen Queue Optimization System** uses a hybrid architecture:
1. **In-Memory C++ DSA Engine** is authoritative for real-time scheduling: custom arrays, singly linked lists, FIFO queues, and stacks maintain live queue ordering without disk latency.
2. **SQLite Relational Layer** provides ACID durability: persists menu, counters, orders, order items, payment status, idempotency keys, and audit logs.

---

## 2. Relational Schema

```mermaid
erDiagram
    MENU_ITEMS ||--o{ ORDER_ITEMS : "referenced in"
    COUNTERS ||--o{ ORDERS : "services"
    ORDERS ||--|{ ORDER_ITEMS : "contains"

    MENU_ITEMS {
        INTEGER id PK
        TEXT name
        TEXT category
        REAL price
        INTEGER prep_time_minutes
        INTEGER is_available
        INTEGER complexity
    }
    COUNTERS {
        INTEGER id PK
        TEXT name
        TEXT counter_type
        INTEGER is_available
    }
    ORDERS {
        INTEGER id PK
        TEXT student_id
        TEXT student_name
        TEXT status
        TEXT payment_status
        INTEGER assigned_counter_id FK
        INTEGER total_prep_time
        REAL total_amount
        INTEGER order_timestamp
        TEXT idempotency_key UNIQUE
    }
    ORDER_ITEMS {
        INTEGER id PK
        INTEGER order_id FK
        INTEGER item_id FK
        TEXT item_name
        REAL item_price
        INTEGER quantity
    }
    ADMIN_AUDIT_LOG {
        INTEGER id PK
        TEXT action_type
        INTEGER target_id
        TEXT target_name
        INTEGER prev_bool
        INTEGER new_bool
        REAL prev_double
        REAL new_double
        TEXT description
        INTEGER timestamp
    }
```

---

## 3. Field Reference — orders table

| Column | Type | Default | Description |
|---|---|---|---|
| `id` | INTEGER PK | — | Auto-assigned by system on order placement |
| `student_id` | TEXT | — | Student roll number / ID |
| `student_name` | TEXT | — | Student display name |
| `status` | TEXT | `QUEUED` | Order lifecycle state |
| `payment_status` | TEXT | `PENDING` | Payment state (`PENDING` or `PAID`) |
| `assigned_counter_id` | INTEGER FK | — | Counter serving this order |
| `total_prep_time` | INTEGER | — | Predicted prep time in minutes (ML) |
| `total_amount` | REAL | — | Sum of all line item prices × quantities |
| `order_timestamp` | INTEGER | — | Unix epoch seconds when order was placed |
| `idempotency_key` | TEXT UNIQUE | NULL | Prevents duplicate order submission |

### payment_status values

| Value | Meaning |
|---|---|
| `PENDING` | Default — payment not yet received |
| `PAID` | Confirmed by staff at counter (cash/UPI) |

Payment confirmation is persisted immediately via `UPDATE orders SET payment_status = 'PAID'` using a prepared statement. It survives application restart. Payment cannot be confirmed for CANCELLED orders.

### status values

| Value | Terminal | Next valid states |
|---|---|---|
| `QUEUED` | No | `PREPARING`, `CANCELLED` |
| `PREPARING` | No | `READY`, `CANCELLED` |
| `READY` | No | `COLLECTED` |
| `COLLECTED` | ✅ Yes | — |
| `CANCELLED` | ✅ Yes | — |

---

## 4. Schema Migration

`payment_status` was added as a non-breaking migration. On startup, `createSchema()` runs:

```sql
-- Safe migration: ignored if column already exists
ALTER TABLE orders ADD COLUMN payment_status TEXT NOT NULL DEFAULT 'PENDING';
```

This allows existing databases created before this change to continue working without data loss.

---

## 5. State Recovery on Restart

On application start, `DatabaseManager::recoverSystemState()`:
1. Loads all orders with `status IN ('QUEUED', 'PREPARING')` from SQLite.
2. Re-enqueues them into the appropriate counter's FIFO `Queue<Order>`.
3. Reconstructs counter workload totals from line items.
4. Loads `payment_status` from DB — payment persisted before restart is correctly restored.

---

## 6. Audit Log

Every staff operation that produces an acknowledgement (`/accept`) is logged in `admin_audit_log` with:
- `action_type`: e.g., `ORDER_ACKNOWLEDGED`
- `target_id`: order ID
- `target_name`: student name
- `description`: human-readable summary
- `timestamp`: Unix epoch

---

## 7. Idempotency

Each `POST /api/orders` request may include an `Idempotency-Key` header. If a matching key already exists in SQLite, the existing order is returned (HTTP 200) instead of creating a duplicate. This prevents double-ordering from network retries.

---

## 8. Thread Safety

All database operations are protected by a `std::recursive_mutex` (`dbMutex`). The HTTP server uses a separate `std::mutex` (`stateMutex`) to serialize concurrent request handlers against the in-memory DSA engine.
