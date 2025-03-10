/**
 * @file Main.cpp
 * @brief Main entry point for the Weather App
 */
#include "WeatherApp.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        WeatherApp app;
        app.initialize();
        app.run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}