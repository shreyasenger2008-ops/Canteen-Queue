#include "canteen/core/CanteenSystem.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    canteen::CanteenSystem system;

    // Check if user requested automated demo via CLI flag
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--demo" || arg == "-d" || arg == "--test") {
            system.runDemonstrationSimulation();
            return 0;
        }
    }

    std::cout << "Starting AI-Based College Canteen Queue Optimization System...\n";
    std::cout << "Tip: Pass '--demo' argument to run automated demonstration simulation.\n";

    system.runInteractiveCLI();
    return 0;
}
