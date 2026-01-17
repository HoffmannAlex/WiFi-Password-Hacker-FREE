/**
 * @file main.cpp
 * @brief Main entry point for the Hack WiFi AI application
 * 
 * This file contains the main function that initializes and runs the HackWifiAI application.
 * It handles command-line arguments and manages the application lifecycle.
 */

#include "../include/HackWifiAI.h"
#include <iostream>
#include <string>

/**
 * @brief Main entry point of the application
 * 
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return int Exit code (0 for success, non-zero for error)
 */
int main(int argc, char* argv[]) {
    // Display a friendly message
    std::cout << "🚀 Starting Hack WiFi AI - Advanced WiFi Security Testing Tool\n";
    std::cout << "   Version 2025.1 - Educational Use Only\n\n";
    
    try {
        // Create and run the main application
        HackWifiAI app;
        return app.run();
    } 
    catch (const std::exception& e) {
        // Handle standard exceptions
        std::cerr << "\n❌ Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        // Handle any other exceptions
        std::cerr << "\n❌ Unknown error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
