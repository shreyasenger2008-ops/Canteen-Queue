#include "test_framework.hpp"
#include "canteen/core/CanteenSystem.hpp"
#include "canteen/api/HttpServer.hpp"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <cstdio>
#include <thread>
#include <chrono>

using json = nlohmann::json;
using namespace canteen;

static const int TEST_API_PORT = 18089;
static const std::string TEST_API_HOST = "127.0.0.1";
static const std::string TEST_API_DB = "test_api.db";

// Helper test fixture runner
struct ApiTestHarness {
    std::string dbFile;
    std::unique_ptr<CanteenSystem> system;
    std::unique_ptr<api::HttpServer> server;
    std::unique_ptr<httplib::Client> client;

    ApiTestHarness(const std::string& dbPath = TEST_API_DB) : dbFile(dbPath) {
        std::remove(dbFile.c_str());
        system = std::make_unique<CanteenSystem>(dbFile);
        system->setUseMLPredictor(false); // deterministic prep times for tests
        server = std::make_unique<api::HttpServer>(*system, system->getDatabaseManager(), TEST_API_PORT);
        server->start(true, TEST_API_HOST);
        client = std::make_unique<httplib::Client>(TEST_API_HOST, TEST_API_PORT);
        client->set_connection_timeout(2, 0);
        client->set_read_timeout(2, 0);
    }

    ~ApiTestHarness() {
        server->stop();
        server.reset();
        system.reset();
        std::remove(dbFile.c_str());
    }
};

TEST(API_HealthEndpoint) {
    ApiTestHarness harness("test_api_health.db");
    auto res = harness.client->Get("/api/health");
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->status, 200);

    auto j = json::parse(res->body);
    ASSERT_EQ(j["status"], "healthy");
    ASSERT_TRUE(j["database_connected"].get<bool>());
    ASSERT_EQ(j["active_counters_count"], 4);
}

TEST(API_MenuEndpoint) {
    ApiTestHarness harness("test_api_menu.db");
    auto res = harness.client->Get("/api/menu");
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->status, 200);

    auto j = json::parse(res->body);
    ASSERT_TRUE(j.is_array());
    ASSERT_TRUE(j.size() >= 8); // Seeded default menu items
    ASSERT_EQ(j[0]["id"], 101);
    ASSERT_EQ(j[0]["name"], "Poha");
}

TEST(API_CountersEndpoint) {
    ApiTestHarness harness("test_api_counters.db");
    auto res = harness.client->Get("/api/counters");
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->status, 200);

    auto j = json::parse(res->body);
    ASSERT_TRUE(j.is_array());
    ASSERT_EQ(j.size(), size_t(4));
    ASSERT_EQ(j[0]["id"], 1);
    ASSERT_TRUE(j[0]["is_available"].get<bool>());
}

TEST(API_PlaceOrderValidAndLookup) {
    ApiTestHarness harness("test_api_order.db");

    json orderReq = {
        {"student_id", "STU101"},
        {"student_name", "Rahul Sharma"},
        {"items", json::array({
            {{"item_id", 101}, {"quantity", 2}},
            {{"item_id", 105}, {"quantity", 1}}
        })}
    };

    auto res = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->status, 201);

    auto j = json::parse(res->body);
    int orderId = j["id"];
    ASSERT_TRUE(orderId > 0);
    ASSERT_EQ(j["student_id"], "STU101");
    ASSERT_EQ(j["student_name"], "Rahul Sharma");
    ASSERT_EQ(j["status"], "QUEUED");
    ASSERT_TRUE(j["assigned_counter_id"] > 0);
    ASSERT_EQ(j["items"].size(), size_t(2));

    // GET /api/orders/{id}
    auto getRes = harness.client->Get(("/api/orders/" + std::to_string(orderId)).c_str());
    ASSERT_TRUE(getRes != nullptr);
    ASSERT_EQ(getRes->status, 200);
    auto getJ = json::parse(getRes->body);
    ASSERT_EQ(getJ["id"], orderId);
    ASSERT_EQ(getJ["student_name"], "Rahul Sharma");

    // Non-existent order
    auto notFound = harness.client->Get("/api/orders/99999");
    ASSERT_TRUE(notFound != nullptr);
    ASSERT_EQ(notFound->status, 404);
}

TEST(API_PlaceOrderIdempotencyKey) {
    ApiTestHarness harness("test_api_idemp.db");

    json orderReq = {
        {"student_id", "STU202"},
        {"student_name", "Priya Patel"},
        {"idempotency_key", "client-txn-uuid-8888"},
        {"items", json::array({
            {{"item_id", 103}, {"quantity", 1}}
        })}
    };

    // First placement -> 201 Created
    auto res1 = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
    ASSERT_TRUE(res1 != nullptr);
    ASSERT_EQ(res1->status, 201);
    auto j1 = json::parse(res1->body);
    int firstOrderId = j1["id"];

    // Retry placement with same key -> 200 OK with same order
    auto res2 = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
    ASSERT_TRUE(res2 != nullptr);
    ASSERT_EQ(res2->status, 200);
    auto j2 = json::parse(res2->body);
    int secondOrderId = j2["id"];

    ASSERT_EQ(firstOrderId, secondOrderId);
}

TEST(API_PlaceOrderValidationFailures) {
    ApiTestHarness harness("test_api_val.db");

    // Missing student_name
    json bad1 = {
        {"student_id", "STU1"},
        {"items", json::array({{{"item_id", 101}, {"quantity", 1}}})}
    };
    auto res1 = harness.client->Post("/api/orders", bad1.dump(), "application/json");
    ASSERT_TRUE(res1 != nullptr);
    ASSERT_EQ(res1->status, 400);

    // Empty items
    json bad2 = {
        {"student_id", "STU1"},
        {"student_name", "Bob"},
        {"items", json::array()}
    };
    auto res2 = harness.client->Post("/api/orders", bad2.dump(), "application/json");
    ASSERT_TRUE(res2 != nullptr);
    ASSERT_EQ(res2->status, 400);

    // Invalid quantity
    json bad3 = {
        {"student_id", "STU1"},
        {"student_name", "Bob"},
        {"items", json::array({{{"item_id", 101}, {"quantity", 0}}})}
    };
    auto res3 = harness.client->Post("/api/orders", bad3.dump(), "application/json");
    ASSERT_TRUE(res3 != nullptr);
    ASSERT_EQ(res3->status, 400);

    // Non-existent item
    json bad4 = {
        {"student_id", "STU1"},
        {"student_name", "Bob"},
        {"items", json::array({{{"item_id", 9999}, {"quantity", 1}}})}
    };
    auto res4 = harness.client->Post("/api/orders", bad4.dump(), "application/json");
    ASSERT_TRUE(res4 != nullptr);
    ASSERT_EQ(res4->status, 400);
}

TEST(API_OrderLifecycleProgression) {
    ApiTestHarness harness("test_api_lifecycle.db");
    httplib::Headers headers = {{"Authorization", "Bearer staff-dev-token-1234"}};

    // 1. Create order
    json orderReq = {
        {"student_id", "STU303"},
        {"student_name", "Vikram Singh"},
        {"preferred_counter_id", 2},
        {"items", json::array({{{"item_id", 104}, {"quantity", 1}}})}
    };
    auto res = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->status, 201);
    int orderId = json::parse(res->body)["id"];

    // 2. Start preparing: POST /api/orders/{id}/start -> 200
    auto startRes = harness.client->Post(("/api/orders/" + std::to_string(orderId) + "/start").c_str(), headers, "", "application/json");
    ASSERT_TRUE(startRes != nullptr);
    ASSERT_EQ(startRes->status, 200);
    ASSERT_EQ(json::parse(startRes->body)["status"], "PREPARING");

    // Illegal duplicate start -> 409
    auto startDup = harness.client->Post(("/api/orders/" + std::to_string(orderId) + "/start").c_str(), headers, "", "application/json");
    ASSERT_TRUE(startDup != nullptr);
    ASSERT_EQ(startDup->status, 409);

    // 3. Mark ready: POST /api/orders/{id}/ready -> 200
    auto readyRes = harness.client->Post(("/api/orders/" + std::to_string(orderId) + "/ready").c_str(), headers, "", "application/json");
    ASSERT_TRUE(readyRes != nullptr);
    ASSERT_EQ(readyRes->status, 200);
    ASSERT_EQ(json::parse(readyRes->body)["status"], "READY");

    // 4. Collect: POST /api/orders/{id}/collect -> 200
    auto collectRes = harness.client->Post(("/api/orders/" + std::to_string(orderId) + "/collect").c_str(), headers, "", "application/json");
    ASSERT_TRUE(collectRes != nullptr);
    ASSERT_EQ(collectRes->status, 200);
    ASSERT_EQ(json::parse(collectRes->body)["status"], "COLLECTED");

    // Cannot cancel after collected -> 409
    auto cancelRes = harness.client->Post(("/api/orders/" + std::to_string(orderId) + "/cancel").c_str(), headers, "", "application/json");
    ASSERT_TRUE(cancelRes != nullptr);
    ASSERT_EQ(cancelRes->status, 409);
}

TEST(API_OrderCancellationInQueue) {
    ApiTestHarness harness("test_api_cancel.db");
    httplib::Headers headers = {{"Authorization", "Bearer staff-dev-token-1234"}};

    json orderReq = {
        {"student_id", "STU404"},
        {"student_name", "Ananya"},
        {"preferred_counter_id", 1},
        {"items", json::array({{{"item_id", 101}, {"quantity", 1}}})}
    };
    auto res = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->status, 201);
    int orderId = json::parse(res->body)["id"];

    // Cancel order
    auto cancelRes = harness.client->Post(("/api/orders/" + std::to_string(orderId) + "/cancel").c_str(), headers, "", "application/json");
    ASSERT_TRUE(cancelRes != nullptr);
    ASSERT_EQ(cancelRes->status, 200);
    ASSERT_EQ(json::parse(cancelRes->body)["status"], "CANCELLED");

    // Counter workload should have dropped to 0
    auto countersRes = harness.client->Get("/api/counters");
    auto countersJ = json::parse(countersRes->body);
    ASSERT_EQ(countersJ[0]["pending_orders_count"], 0);
}

TEST(API_AnalyticsAndCorsHeaders) {
    ApiTestHarness harness("test_api_analytics.db");

    // Analytics
    auto res = harness.client->Get("/api/analytics");
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->status, 200);
    auto j = json::parse(res->body);
    ASSERT_TRUE(j.contains("total_orders_history_count"));
    ASSERT_TRUE(j.contains("currently_pending_orders"));
    ASSERT_TRUE(j.contains("total_active_workload_minutes"));
    ASSERT_TRUE(j.contains("counters_analytics"));

    // CORS Headers
    ASSERT_TRUE(res->has_header("Access-Control-Allow-Origin"));
    ASSERT_EQ(res->get_header_value("Access-Control-Allow-Origin"), "*");

    // Preflight OPTIONS
    auto optRes = harness.client->Options("/api/menu");
    ASSERT_TRUE(optRes != nullptr);
    ASSERT_EQ(optRes->status, 204);
    ASSERT_TRUE(optRes->has_header("Access-Control-Allow-Origin"));
}

TEST(API_GetOrdersPaginatedAndFilter) {
    ApiTestHarness harness("test_api_paginated.db");

    // Place 3 orders
    for (int i = 1; i <= 3; ++i) {
        json orderReq = {
            {"student_id", "STU_" + std::to_string(i)},
            {"student_name", "Student " + std::to_string(i)},
            {"items", json::array({{{"item_id", 101}, {"quantity", 1}}})}
        };
        auto res = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
        ASSERT_TRUE(res != nullptr);
        ASSERT_EQ(res->status, 201);
    }

    // 1. Basic paginated query: limit 2, offset 0
    auto res1 = harness.client->Get("/api/orders?limit=2&offset=0");
    ASSERT_TRUE(res1 != nullptr);
    ASSERT_EQ(res1->status, 200);
    auto j1 = json::parse(res1->body);
    ASSERT_EQ(j1["total"], 3);
    ASSERT_EQ(j1["limit"], 2);
    ASSERT_EQ(j1["offset"], 0);
    ASSERT_EQ(j1["orders"].size(), size_t(2));

    // 2. Query page 2: limit 2, offset 2
    auto res2 = harness.client->Get("/api/orders?limit=2&offset=2");
    ASSERT_TRUE(res2 != nullptr);
    ASSERT_EQ(res2->status, 200);
    auto j2 = json::parse(res2->body);
    ASSERT_EQ(j2["total"], 3);
    ASSERT_EQ(j2["orders"].size(), size_t(1));

    // 3. Status filter: all currently QUEUED
    auto resQueued = harness.client->Get("/api/orders?status=QUEUED");
    ASSERT_TRUE(resQueued != nullptr);
    ASSERT_EQ(resQueued->status, 200);
    auto jQueued = json::parse(resQueued->body);
    ASSERT_EQ(jQueued["total"], 3);

    // 4. Status filter: CANCELLED (none exist yet) -> empty array
    auto resEmpty = harness.client->Get("/api/orders?status=CANCELLED");
    ASSERT_TRUE(resEmpty != nullptr);
    ASSERT_EQ(resEmpty->status, 200);
    auto jEmpty = json::parse(resEmpty->body);
    ASSERT_EQ(jEmpty["total"], 0);
    ASSERT_TRUE(jEmpty["orders"].empty());

    // 5. Invalid query parameters rejection (HTTP 400)
    auto resBadLimit = harness.client->Get("/api/orders?limit=-5");
    ASSERT_TRUE(resBadLimit != nullptr);
    ASSERT_EQ(resBadLimit->status, 400);

    auto resBadOffset = harness.client->Get("/api/orders?offset=-1");
    ASSERT_TRUE(resBadOffset != nullptr);
    ASSERT_EQ(resBadOffset->status, 400);

    auto resBadStatus = harness.client->Get("/api/orders?status=UNKNOWN_XYZ");
    ASSERT_TRUE(resBadStatus != nullptr);
    ASSERT_EQ(resBadStatus->status, 400);
}

// ============================================================
// AUTH TESTS
// ============================================================

TEST(API_StaffAuth_ValidPin) {
    ApiTestHarness harness("test_auth_valid.db");
    json body = {{"pin", "1234"}};
    auto res = harness.client->Post("/api/staff/auth", body.dump(), "application/json");
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->status, 200);
    auto j = json::parse(res->body);
    ASSERT_TRUE(j.contains("token"));
    ASSERT_FALSE(j["token"].get<std::string>().empty());
}

TEST(API_StaffAuth_InvalidPin) {
    ApiTestHarness harness("test_auth_invalid.db");
    json body = {{"pin", "wrong"}};
    auto res = harness.client->Post("/api/staff/auth", body.dump(), "application/json");
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->status, 401);
}

TEST(API_StaffProtected_NoToken) {
    ApiTestHarness harness("test_auth_notoken.db");
    // Place an order first
    json orderReq = {{"student_id","STU99"},{"student_name","Test"},{"items",json::array({json{{"item_id",101},{"quantity",1}}})}};
    auto placeRes = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
    ASSERT_EQ(placeRes->status, 201);
    int orderId = json::parse(placeRes->body)["id"];

    // Try staff action without token -> 401
    auto res = harness.client->Post("/api/orders/" + std::to_string(orderId) + "/start", "", "application/json");
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->status, 401);
}

TEST(API_StaffProtected_WithToken) {
    ApiTestHarness harness("test_auth_withtoken.db");
    // Get token
    json authBody = {{"pin","1234"}};
    auto authRes = harness.client->Post("/api/staff/auth", authBody.dump(), "application/json");
    ASSERT_EQ(authRes->status, 200);
    std::string token = json::parse(authRes->body)["token"];

    httplib::Headers headers = {{"Authorization", "Bearer " + token}};

    // Place order
    json orderReq = {{"student_id","STU88"},{"student_name","Authorized"},{"items",json::array({json{{"item_id",101},{"quantity",1}}})}};
    auto placeRes = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
    ASSERT_EQ(placeRes->status, 201);
    int orderId = json::parse(placeRes->body)["id"];

    // Start with valid token
    auto res = harness.client->Post("/api/orders/" + std::to_string(orderId) + "/start", headers, "", "application/json");
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->status, 200);
    ASSERT_EQ(json::parse(res->body)["status"], "PREPARING");
}

// ============================================================
// PAYMENT PERSISTENCE TESTS
// ============================================================

TEST(API_PaymentPersistence) {
    ApiTestHarness harness("test_payment.db");

    // Get staff token
    json authBody = {{"pin","1234"}};
    auto authRes = harness.client->Post("/api/staff/auth", authBody.dump(), "application/json");
    std::string token = json::parse(authRes->body)["token"];
    httplib::Headers headers = {{"Authorization", "Bearer " + token}};

    // Place order
    json orderReq = {{"student_id","STU77"},{"student_name","Pay Test"},{"items",json::array({json{{"item_id",101},{"quantity",1}}})}};
    auto placeRes = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
    ASSERT_EQ(placeRes->status, 201);
    int orderId = json::parse(placeRes->body)["id"];
    ASSERT_EQ(json::parse(placeRes->body)["payment_status"], "PENDING");

    // Confirm payment
    auto payRes = harness.client->Post("/api/orders/" + std::to_string(orderId) + "/pay", headers, "", "application/json");
    ASSERT_EQ(payRes->status, 200);
    ASSERT_EQ(json::parse(payRes->body)["payment_status"], "PAID");

    // Look up order — must still show PAID (from memory)
    auto getRes = harness.client->Get("/api/orders/" + std::to_string(orderId));
    ASSERT_EQ(json::parse(getRes->body)["payment_status"], "PAID");
}

TEST(API_PaymentCancelledOrder) {
    ApiTestHarness harness("test_pay_cancelled.db");

    // Get staff token
    json authBody = {{"pin","1234"}};
    auto authRes = harness.client->Post("/api/staff/auth", authBody.dump(), "application/json");
    std::string token = json::parse(authRes->body)["token"];
    httplib::Headers headers = {{"Authorization", "Bearer " + token}};

    // Place and cancel order
    json orderReq = {{"student_id","STU66"},{"student_name","Cancel Test"},{"items",json::array({json{{"item_id",101},{"quantity",1}}})}};
    auto placeRes = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
    ASSERT_EQ(placeRes->status, 201);
    int orderId = json::parse(placeRes->body)["id"];

    auto cancelRes = harness.client->Post("/api/orders/" + std::to_string(orderId) + "/cancel", headers, "", "application/json");
    ASSERT_EQ(cancelRes->status, 200);

    // Try to pay — must return 409
    auto payRes = harness.client->Post("/api/orders/" + std::to_string(orderId) + "/pay", headers, "", "application/json");
    ASSERT_EQ(payRes->status, 409);
}

// ============================================================
// ACCEPT ORDER TESTS
// ============================================================

TEST(API_AcceptOrder_Valid) {
    ApiTestHarness harness("test_accept.db");
    json authBody = {{"pin","1234"}};
    auto authRes = harness.client->Post("/api/staff/auth", authBody.dump(), "application/json");
    std::string token = json::parse(authRes->body)["token"];
    httplib::Headers headers = {{"Authorization", "Bearer " + token}};

    json orderReq = {{"student_id","STU55"},{"student_name","Accept Test"},{"items",json::array({json{{"item_id",101},{"quantity",1}}})}};
    auto placeRes = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
    ASSERT_EQ(placeRes->status, 201);
    int orderId = json::parse(placeRes->body)["id"];

    auto res = harness.client->Post("/api/orders/" + std::to_string(orderId) + "/accept", headers, "", "application/json");
    ASSERT_EQ(res->status, 200);
    auto j = json::parse(res->body);
    ASSERT_EQ(j["status"], "QUEUED");  // Status unchanged
    ASSERT_TRUE(j["acknowledged"].get<bool>());
}

TEST(API_AcceptOrder_NoToken) {
    ApiTestHarness harness("test_accept_noauth.db");
    json orderReq = {{"student_id","STU44"},{"student_name","No Auth"},{"items",json::array({json{{"item_id",101},{"quantity",1}}})}};
    auto placeRes = harness.client->Post("/api/orders", orderReq.dump(), "application/json");
    ASSERT_EQ(placeRes->status, 201);
    int orderId = json::parse(placeRes->body)["id"];

    auto res = harness.client->Post("/api/orders/" + std::to_string(orderId) + "/accept", "", "application/json");
    ASSERT_EQ(res->status, 401);
}
