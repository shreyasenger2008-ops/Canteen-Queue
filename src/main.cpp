#include "canteen/core/CanteenSystem.hpp"
#include "canteen/api/HttpServer.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    canteen::CanteenSystem system("canteen.db");

    // Command line argument handling
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--demo" || arg == "-d" || arg == "--test") {
            system.runDemonstrationSimulation();
            return 0;
        }

        if (arg == "--server" || arg == "-s") {
            int port = 8080;
            if (argc > 2) {
                try {
                    port = std::stoi(argv[2]);
                } catch (...) {
                    port = 8080;
                }
            }
            std::cout << "Starting C++ REST API Server on port " << port << "...\n";
            canteen::api::HttpServer server(system, system.getDatabaseManager(), port);
            server.start(false, "0.0.0.0"); // Blocks until stopped
            return 0;
        }
    }

    std::cout << "Starting AI-Based College Canteen Queue Optimization System...\n";
    std::cout << "Usage:\n";
    std::cout << "  --server [port]   Start REST API HTTP Server (default: 8080)\n";
    std::cout << "  --demo            Run automated DSA demonstration\n\n";

    system.runInteractiveCLI();
    return 0;
}
