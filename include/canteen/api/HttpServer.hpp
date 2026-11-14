#ifndef CANTEEN_HTTP_SERVER_HPP
#define CANTEEN_HTTP_SERVER_HPP

#include "canteen/core/CanteenSystem.hpp"
#include "canteen/db/DatabaseManager.hpp"
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

namespace httplib {
    class Server;
    struct Request;
}

namespace canteen {
namespace api {

/**
 * @brief Lightweight C++ REST API exposing the canteen queue optimization engine
 * to web dashboards and client applications.
 */
class HttpServer {
private:
    CanteenSystem& system;
    db::DatabaseManager& db;
    std::unique_ptr<httplib::Server> server;
    std::string host;
    int port{8080};
    std::atomic<bool> running{false};
    std::thread serverThread;
    mutable std::mutex stateMutex; // Synchronizes HTTP requests with in-memory C++ state

    void setupRoutes();
    void setupCors();
    bool isStaffAuthorized(const httplib::Request& req) const;

public:
    HttpServer(CanteenSystem& sys, db::DatabaseManager& database, int listenPort = 8080);
    ~HttpServer();

    // Lifecycle
    bool start(bool runInBackground = false, const std::string& bindHost = "0.0.0.0");
    void stop();
    bool isRunning() const { return running.load(); }
    int getPort() const { return port; }
    std::string getHost() const { return host; }
};

} // namespace api
} // namespace canteen

#endif // CANTEEN_HTTP_SERVER_HPP
