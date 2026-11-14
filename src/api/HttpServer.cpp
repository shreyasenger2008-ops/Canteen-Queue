#include "canteen/api/HttpServer.hpp"
#include "httplib.h"
#include "nlohmann/json.hpp"

#include <iostream>
#include <chrono>
#include <cstdlib>
#include <sstream>

using json = nlohmann::json;

namespace canteen {
namespace api {

static json menuItemToJson(const FoodItem& item) {
    return json{
        {"id", item.getId()},
        {"name", item.getName()},
        {"category", item.getCategory()},
        {"price", item.getPrice()},
        {"prep_time_minutes", item.getPreparationTimeMinutes()},
        {"is_available", item.getIsAvailable()},
        {"complexity", item.getComplexity()}
    };
}

static json orderToJson(const Order& order, const CanteenSystem* sys = nullptr) {
    json itemsArray = json::array();
    for (const auto& line : order.getItems()) {
        itemsArray.push_back({
            {"item_id", line.item.getId()},
            {"item_name", line.item.getName()},
            {"price", line.item.getPrice()},
            {"quantity", line.quantity}
        });
    }

    int queuePosition = 0;
    if (sys != nullptr && (order.getStatus() == OrderStatus::QUEUED || order.getStatus() == OrderStatus::PREPARING)) {
        const Counter* c = sys->getCounterManager().getCounterById(order.getAssignedCounterId());
        if (c != nullptr) {
            size_t pos = 1;
            c->getPendingQueue().traverse([&](const Order& o) {
                if (o.getId() == order.getId()) {
                    queuePosition = static_cast<int>(pos);
                }
                pos++;
            });
        }
    }

    return json{
        {"id", order.getId()},
        {"student_id", order.getStudentId()},
        {"student_name", order.getStudentName()},
        {"status", orderStatusToString(order.getStatus())},
        {"payment_status", order.getPaymentStatus()},
        {"queue_position", queuePosition},
        {"assigned_counter_id", order.getAssignedCounterId()},
        {"total_prep_time_minutes", order.getTotalPreparationTimeMinutes()},
        {"total_amount", order.getTotalAmount()},
        {"order_timestamp", order.getOrderTimestamp()},
        {"items", itemsArray}
    };
}

static json counterToJson(const Counter& counter) {
    return json{
        {"id", counter.getId()},
        {"name", counter.getName()},
        {"counter_type", counter.getCounterType()},
        {"is_available", counter.getIsAvailable()},
        {"pending_orders_count", counter.getPendingOrderCount()},
        {"current_workload_minutes", counter.getCurrentWorkloadMinutes()},
        {"total_orders_served", counter.getTotalOrdersServed()}
    };
}

HttpServer::HttpServer(CanteenSystem& sys, db::DatabaseManager& database, int listenPort)
    : system(sys), db(database), port(listenPort) {
    server = std::make_unique<httplib::Server>();
    setupCors();
    setupRoutes();
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::setupCors() {
    server->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, Idempotency-Key");
        if (req.method == "OPTIONS") {
            res.status = 204;
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });
}

bool HttpServer::isStaffAuthorized(const httplib::Request& req) const {
    // Staff token: read from CANTEEN_STAFF_TOKEN env var; fallback to hardcoded default for dev.
    // IMPORTANT: Set CANTEEN_STAFF_TOKEN in production environment.
    const char* envToken = std::getenv("CANTEEN_STAFF_TOKEN");
    std::string validToken = envToken ? std::string(envToken) : "staff-dev-token-1234";

    if (!req.has_header("Authorization")) {
        return false;
    }
    std::string authHeader = req.get_header_value("Authorization");
    // Expected: "Bearer <token>"
    if (authHeader.rfind("Bearer ", 0) != 0) {
        return false;
    }
    std::string provided = authHeader.substr(7);
    return provided == validToken;
}

void HttpServer::setupRoutes() {
    // Staff Authentication: POST /api/staff/auth
    server->Post("/api/staff/auth", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string pin = body.value("pin", "");

            // Read valid PINs from env
            const char* envPins = std::getenv("CANTEEN_STAFF_PINS");
            std::string pinsStr = envPins ? std::string(envPins) : "1234,canteen2026,admin";
            
            // Parse comma-separated PINs
            bool valid = false;
            std::istringstream ss(pinsStr);
            std::string tok;
            while (std::getline(ss, tok, ',')) {
                if (tok == pin) { valid = true; break; }
            }

            if (!valid) {
                res.status = 401;
                res.set_content(json{{"error", "Invalid staff PIN"}}.dump(), "application/json");
                return;
            }

            // Return the server-side token
            const char* envToken = std::getenv("CANTEEN_STAFF_TOKEN");
            std::string token = envToken ? std::string(envToken) : "staff-dev-token-1234";
            
            res.status = 200;
            res.set_content(json{{"token", token}, {"message", "Staff authenticated successfully"}}.dump(2), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid request body"}}.dump(), "application/json");
        }
    });

    // 1. GET /api/health
    server->Get("/api/health", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        json response = {
            {"status", "healthy"},
            {"service", "AI-Based College Canteen Queue Optimization System"},
            {"database_connected", db.isOpen()},
            {"ml_model_loaded", system.isMLPredictorActive()},
            {"ml_model_source", system.getPredictor().getModelName()},
            {"active_counters_count", system.getCounterManager().getCounterCount()}
        };
        res.set_content(response.dump(2), "application/json");
    });

    // 2. GET /api/menu
    server->Get("/api/menu", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        json array = json::array();
        for (const auto& item : system.getMenuManager().getItems()) {
            array.push_back(menuItemToJson(item));
        }
        res.set_content(array.dump(2), "application/json");
    });

    // 3. GET /api/counters
    server->Get("/api/counters", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        json array = json::array();
        const auto& countersArr = system.getCounterManager().getCountersArray();
        for (size_t i = 0; i < countersArr.size(); ++i) {
            array.push_back(counterToJson(countersArr[i]));
        }
        res.set_content(array.dump(2), "application/json");
    });

    // 4. POST /api/orders
    server->Post("/api/orders", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        try {
            auto body = json::parse(req.body);

            // Validation
            if (!body.contains("student_id") || !body.contains("student_name") || !body.contains("items")) {
                res.status = 400;
                res.set_content(json{{"error", "Missing required fields: student_id, student_name, items"}}.dump(), "application/json");
                return;
            }

            std::string studentId = body["student_id"];
            std::string studentName = body["student_name"];
            std::string idempotencyKey = body.value("idempotency_key", "");
            int preferredCounterId = body.value("preferred_counter_id", -1);

            if (studentId.empty() || studentName.empty()) {
                res.status = 400;
                res.set_content(json{{"error", "student_id and student_name cannot be empty"}}.dump(), "application/json");
                return;
            }

            if (!body["items"].is_array() || body["items"].empty()) {
                res.status = 400;
                res.set_content(json{{"error", "Order items list cannot be empty"}}.dump(), "application/json");
                return;
            }

            // Check Idempotency Key
            if (!idempotencyKey.empty()) {
                auto existing = db.findOrderByIdempotencyKey(idempotencyKey);
                if (existing.has_value()) {
                    res.status = 200; // Return existing idempotent order
                    res.set_content(orderToJson(*existing, &system).dump(2), "application/json");
                    return;
                }
            }

            // Validate requested items
            std::vector<std::pair<int, int>> itemRequests;
            for (const auto& it : body["items"]) {
                if (!it.contains("item_id") || !it.contains("quantity")) {
                    res.status = 400;
                    res.set_content(json{{"error", "Each item requires item_id and quantity"}}.dump(), "application/json");
                    return;
                }
                int itId = it["item_id"];
                int qty = it["quantity"];

                if (qty <= 0) {
                    res.status = 400;
                    res.set_content(json{{"error", "Item quantity must be greater than 0"}}.dump(), "application/json");
                    return;
                }

                const FoodItem* menuItem = system.getMenuManager().getItemById(itId);
                if (menuItem == nullptr) {
                    res.status = 400;
                    res.set_content(json{{"error", "Item with ID " + std::to_string(itId) + " does not exist"}}.dump(), "application/json");
                    return;
                }
                if (!menuItem->getIsAvailable()) {
                    res.status = 400;
                    res.set_content(json{{"error", "Item '" + menuItem->getName() + "' is currently OUT OF STOCK"}}.dump(), "application/json");
                    return;
                }
                itemRequests.emplace_back(itId, qty);
            }

            // Place order using C++ scheduling engine with ML prediction
            int orderId = system.placeOrder(studentId, studentName, itemRequests, preferredCounterId, idempotencyKey);
            if (orderId <= 0) {
                res.status = 500;
                res.set_content(json{{"error", "Failed to schedule order to available counter"}}.dump(), "application/json");
                return;
            }

            Order placedOrder;
            if (!system.searchOrderById(orderId, placedOrder)) {
                res.status = 500;
                res.set_content(json{{"error", "Order scheduled but failed lookup"}}.dump(), "application/json");
                return;
            }

            res.status = 201;
            res.set_content(orderToJson(placedOrder, &system).dump(2), "application/json");
        } catch (const std::exception& ex) {
            res.status = 400;
            res.set_content(json{{"error", std::string("Malformed JSON body: ") + ex.what()}}.dump(), "application/json");
        }
    });

    // 4b. GET /api/orders (paginated list with optional status filter)
    server->Get("/api/orders", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);

        int limit = 20;
        int offset = 0;
        std::string statusFilter;

        // Parse & validate limit
        if (req.has_param("limit")) {
            std::string limitStr = req.get_param_value("limit");
            try {
                size_t idx = 0;
                int parsed = std::stoi(limitStr, &idx);
                if (idx != limitStr.size() || parsed <= 0) {
                    res.status = 400;
                    res.set_content(json{{"error", "Query parameter 'limit' must be a positive integer"}}.dump(), "application/json");
                    return;
                }
                limit = std::min(parsed, 100);
            } catch (...) {
                res.status = 400;
                res.set_content(json{{"error", "Query parameter 'limit' must be a valid integer"}}.dump(), "application/json");
                return;
            }
        }

        // Parse & validate offset
        if (req.has_param("offset")) {
            std::string offsetStr = req.get_param_value("offset");
            try {
                size_t idx = 0;
                int parsed = std::stoi(offsetStr, &idx);
                if (idx != offsetStr.size() || parsed < 0) {
                    res.status = 400;
                    res.set_content(json{{"error", "Query parameter 'offset' must be a non-negative integer"}}.dump(), "application/json");
                    return;
                }
                offset = parsed;
            } catch (...) {
                res.status = 400;
                res.set_content(json{{"error", "Query parameter 'offset' must be a valid integer"}}.dump(), "application/json");
                return;
            }
        }

        // Parse & validate status
        if (req.has_param("status")) {
            statusFilter = req.get_param_value("status");
            if (!statusFilter.empty() && statusFilter != "ALL") {
                if (statusFilter != "QUEUED" && statusFilter != "PREPARING" &&
                    statusFilter != "READY" && statusFilter != "COLLECTED" &&
                    statusFilter != "CANCELLED") {
                    res.status = 400;
                    res.set_content(json{{"error", "Invalid status filter. Allowed values: ALL, QUEUED, PREPARING, READY, COLLECTED, CANCELLED"}}.dump(), "application/json");
                    return;
                }
            } else {
                statusFilter = "";
            }
        }

        auto paginated = db.loadOrdersPaginated(limit, offset, statusFilter);

        json ordersArr = json::array();
        for (const auto& ord : paginated.orders) {
            ordersArr.push_back(orderToJson(ord, &system));
        }

        json response = {
            {"orders", ordersArr},
            {"total", paginated.totalCount},
            {"limit", paginated.limit},
            {"offset", paginated.offset}
        };

        res.status = 200;
        res.set_content(response.dump(2), "application/json");
    });

    // 5. GET /api/orders/:id
    server->Get(R"(/api/orders/(\d+)/?)", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        int orderId = std::stoi(req.matches[1]);

        Order order;
        if (system.searchOrderById(orderId, order)) {
            res.status = 200;
            res.set_content(orderToJson(order, &system).dump(2), "application/json");
            return;
        }

        // Check SQLite if order was from past session
        auto optOrder = db.loadOrderById(orderId);
        if (optOrder.has_value()) {
            res.status = 200;
            res.set_content(orderToJson(*optOrder, &system).dump(2), "application/json");
            return;
        }

        res.status = 404;
        res.set_content(json{{"error", "Order #" + std::to_string(orderId) + " not found"}}.dump(), "application/json");
    });

    // 6. POST /api/orders/:id/start
    server->Post(R"(/api/orders/(\d+)/start)", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        if (!isStaffAuthorized(req)) {
            res.status = 401;
            res.set_content(json{{"error", "Staff authorization required. Include 'Authorization: Bearer <token>' header."}}.dump(), "application/json");
            return;
        }
        int orderId = std::stoi(req.matches[1]);

        Order order;
        if (!system.searchOrderById(orderId, order)) {
            res.status = 404;
            res.set_content(json{{"error", "Order not found"}}.dump(), "application/json");
            return;
        }

        if (order.getStatus() != OrderStatus::QUEUED) {
            res.status = 409;
            res.set_content(json{{"error", "Cannot start order in status " + orderStatusToString(order.getStatus())}}.dump(), "application/json");
            return;
        }

        // Start preparing order at assigned counter
        bool ok = system.startPreparingOrder(order.getAssignedCounterId());
        if (!ok) {
            res.status = 409;
            res.set_content(json{{"error", "Order is not at front of queue or cannot be transitioned"}}.dump(), "application/json");
            return;
        }

        db.updateOrderStatus(orderId, OrderStatus::PREPARING);
        system.searchOrderById(orderId, order);
        res.status = 200;
        res.set_content(orderToJson(order, &system).dump(2), "application/json");
    });

    // 7. POST /api/orders/:id/ready
    server->Post(R"(/api/orders/(\d+)/ready)", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        if (!isStaffAuthorized(req)) {
            res.status = 401;
            res.set_content(json{{"error", "Staff authorization required. Include 'Authorization: Bearer <token>' header."}}.dump(), "application/json");
            return;
        }
        int orderId = std::stoi(req.matches[1]);

        Order order;
        if (!system.searchOrderById(orderId, order)) {
            res.status = 404;
            res.set_content(json{{"error", "Order not found"}}.dump(), "application/json");
            return;
        }

        if (order.getStatus() != OrderStatus::PREPARING) {
            res.status = 409;
            res.set_content(json{{"error", "Order must be PREPARING before marking READY (current status: " + orderStatusToString(order.getStatus()) + ")"}}.dump(), "application/json");
            return;
        }

        Counter* c = system.getCounterManager().getCounterById(order.getAssignedCounterId());
        if (c == nullptr || c->getPendingQueue().isEmpty() || c->getPendingQueue().front().getId() != orderId) {
            res.status = 409;
            res.set_content(json{{"error", "Order is not at front of counter queue"}}.dump(), "application/json");
            return;
        }

        Order readyOrder = c->completeFrontOrder(); // Sets status to READY and frees up counter queue
        system.getOrderManager().recordOrderCompleted(readyOrder);
        if (db.isOpen()) {
            db.updateOrderStatus(orderId, OrderStatus::READY);
        }

        res.status = 200;
        res.set_content(orderToJson(readyOrder, &system).dump(2), "application/json");
    });

    // 8. POST /api/orders/:id/collect
    server->Post(R"(/api/orders/(\d+)/collect)", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        if (!isStaffAuthorized(req)) {
            res.status = 401;
            res.set_content(json{{"error", "Staff authorization required. Include 'Authorization: Bearer <token>' header."}}.dump(), "application/json");
            return;
        }
        int orderId = std::stoi(req.matches[1]);

        Order order;
        if (!system.searchOrderById(orderId, order)) {
            res.status = 404;
            res.set_content(json{{"error", "Order not found"}}.dump(), "application/json");
            return;
        }

        if (order.getStatus() == OrderStatus::COLLECTED) {
            res.status = 200;
            res.set_content(orderToJson(order, &system).dump(2), "application/json");
            return;
        }

        if (order.getStatus() != OrderStatus::READY) {
            res.status = 409;
            res.set_content(json{{"error", "Order must be READY before collecting (current status: " + orderStatusToString(order.getStatus()) + ")"}}.dump(), "application/json");
            return;
        }

        order.transitionTo(OrderStatus::COLLECTED);
        system.getOrderManager().recordOrderCompleted(order);
        if (db.isOpen()) {
            db.updateOrderStatus(orderId, OrderStatus::COLLECTED);
        }

        res.status = 200;
        res.set_content(orderToJson(order, &system).dump(2), "application/json");
    });

    // 9. POST /api/orders/:id/cancel
    server->Post(R"(/api/orders/(\d+)/cancel)", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        if (!isStaffAuthorized(req)) {
            res.status = 401;
            res.set_content(json{{"error", "Staff authorization required. Include 'Authorization: Bearer <token>' header."}}.dump(), "application/json");
            return;
        }
        int orderId = std::stoi(req.matches[1]);

        Order order;
        if (!system.searchOrderById(orderId, order)) {
            res.status = 404;
            res.set_content(json{{"error", "Order not found in pending queues"}}.dump(), "application/json");
            return;
        }

        if (order.getStatus() == OrderStatus::COLLECTED) {
            res.status = 409;
            res.set_content(json{{"error", "Cannot cancel an already collected order"}}.dump(), "application/json");
            return;
        }

        if (order.getStatus() == OrderStatus::CANCELLED) {
            res.status = 200;
            res.set_content(orderToJson(order, &system).dump(2), "application/json");
            return;
        }

        bool removed = system.cancelOrder(orderId);
        if (removed) {
            db.updateOrderStatus(orderId, OrderStatus::CANCELLED);
            system.searchOrderById(orderId, order);
            res.status = 200;
            res.set_content(orderToJson(order, &system).dump(2), "application/json");
        } else {
            res.status = 500;
            res.set_content(json{{"error", "Failed to cancel order from queue"}}.dump(), "application/json");
        }
    });

    // 9b. POST /api/orders/:id/pay (Confirm payment by staff or cashier)
    server->Post(R"(/api/orders/(\d+)/pay)", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        if (!isStaffAuthorized(req)) {
            res.status = 401;
            res.set_content(json{{"error", "Staff authorization required. Include 'Authorization: Bearer <token>' header."}}.dump(), "application/json");
            return;
        }
        int orderId = std::stoi(req.matches[1]);

        Order order;
        bool found = system.searchOrderById(orderId, order);
        if (!found) {
            auto opt = db.loadOrderById(orderId);
            if (opt.has_value()) {
                order = *opt;
                found = true;
            }
        }

        if (!found) {
            res.status = 404;
            res.set_content(json{{"error", "Order not found"}}.dump(), "application/json");
            return;
        }

        if (order.getStatus() == OrderStatus::CANCELLED) {
            res.status = 409;
            res.set_content(json{{"error", "Cannot process payment for a CANCELLED order"}}.dump(), "application/json");
            return;
        }

        order.setPaymentStatus("PAID");
        db.updateOrderPaymentStatus(orderId, "PAID");

        // If in active counter queue, update payment status in memory
        Counter* c = system.getCounterManager().getCounterById(order.getAssignedCounterId());
        if (c != nullptr) {
            Order* inQ = c->getPendingQueue().find([orderId](const Order& o) { return o.getId() == orderId; });
            if (inQ != nullptr) {
                inQ->setPaymentStatus("PAID");
            }
        }

        res.status = 200;
        res.set_content(orderToJson(order, &system).dump(2), "application/json");
    });

    // 9c. POST /api/orders/:id/accept (Acknowledge and accept incoming order)
    server->Post(R"(/api/orders/(\d+)/accept)", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        if (!isStaffAuthorized(req)) {
            res.status = 401;
            res.set_content(json{{"error", "Staff authorization required. Include 'Authorization: Bearer <token>' header."}}.dump(), "application/json");
            return;
        }
        int orderId = std::stoi(req.matches[1]);

        Order order;
        if (!system.searchOrderById(orderId, order)) {
            res.status = 404;
            res.set_content(json{{"error", "Order not found in active queues"}}.dump(), "application/json");
            return;
        }

        if (order.getStatus() != OrderStatus::QUEUED) {
            res.status = 409;
            res.set_content(json{{"error", "Only QUEUED orders can be acknowledged (current: " + orderStatusToString(order.getStatus()) + ")"}}.dump(), "application/json");
            return;
        }

        // Log acknowledgement to audit log
        if (db.isOpen()) {
            AdminAction ack;
            ack.type = ActionType::ORDER_ACKNOWLEDGED;
            ack.actionType = "ORDER_ACKNOWLEDGED";
            ack.targetId = orderId;
            ack.targetName = order.getStudentName();
            ack.description = "Staff acknowledged order #" + std::to_string(orderId) + " for student " + order.getStudentName();
            ack.timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            db.logAdminAction(ack);
        }

        json resp = orderToJson(order, &system);
        resp["acknowledged"] = true;
        resp["message"] = "Order acknowledged by kitchen staff. Proceed to Start Cooking when ready.";
        res.status = 200;
        res.set_content(resp.dump(2), "application/json");
    });

    // 9d. DELETE /api/orders/:id (Permanently delete order row — staff only)
    server->Delete(R"(/api/orders/(\d+)/?)", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);
        if (!isStaffAuthorized(req)) {
            res.status = 401;
            res.set_content(json{{"error", "Staff authorization required. Include 'Authorization: Bearer <token>' header."}}.dump(), "application/json");
            return;
        }
        int orderId = std::stoi(req.matches[1]);

        if (!system.deleteOrder(orderId)) {
            res.status = 404;
            res.set_content(json{{"error", "Order not found"}}.dump(), "application/json");
            return;
        }

        res.status = 200;
        res.set_content(json{
            {"deleted", true},
            {"id", orderId},
            {"message", "Order permanently deleted"}
        }.dump(2), "application/json");
    });

    // 10. GET /api/analytics
    server->Get("/api/analytics", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMutex);

        size_t totalHistory = system.getOrderManager().getHistoryCount();
        int pendingOrders = 0;
        int activeWorkload = 0;

        json countersData = json::array();
        const auto& countersArr = system.getCounterManager().getCountersArray();
        for (size_t i = 0; i < countersArr.size(); ++i) {
            const auto& c = countersArr[i];
            pendingOrders += static_cast<int>(c.getPendingOrderCount());
            activeWorkload += c.getCurrentWorkloadMinutes();
            countersData.push_back(counterToJson(c));
        }

        json response = {
            {"timestamp", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()},
            {"total_orders_history_count", totalHistory},
            {"currently_pending_orders", pendingOrders},
            {"total_active_workload_minutes", activeWorkload},
            {"ml_model_active", system.isMLPredictorActive()},
            {"ml_model_name", system.getPredictor().getModelName()},
            {"counters_analytics", countersData}
        };

        res.set_content(response.dump(2), "application/json");
    });
}

bool HttpServer::start(bool runInBackground, const std::string& bindHost) {
    host = bindHost;
    if (running.load()) {
        return false;
    }

    if (runInBackground) {
        running.store(true);
        serverThread = std::thread([this]() {
            std::cout << "[REST API SERVER] Listening on http://" << host << ":" << port << "\n";
            server->listen(host.c_str(), port);
            running.store(false);
        });
        // Brief sleep to allow port binding
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return true;
    } else {
        running.store(true);
        std::cout << "[REST API SERVER] Listening on http://" << host << ":" << port << "\n";
        bool ok = server->listen(host.c_str(), port);
        running.store(false);
        return ok;
    }
}

void HttpServer::stop() {
    if (running.load() && server != nullptr) {
        server->stop();
        if (serverThread.joinable()) {
            serverThread.join();
        }
        running.store(false);
        std::cout << "[REST API SERVER] Stopped.\n";
    }
}

} // namespace api
} // namespace canteen
