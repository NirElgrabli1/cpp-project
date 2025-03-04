// main.cpp
/**
 * Weather Application - Main Entry Point
 *
 * This file contains the main function and UI functionality for the weather application.
 * The application provides a console interface for viewing weather data for multiple cities,
 * with data sourced from the OpenWeatherMap API.
 *
 * Features:
 * - Color-coded console interface
 * - Real-time weather data display
 * - 5-day weather forecasts
 * - Favorite cities management
 * - API key management
 *
 * @author Your Name
 * @date February 2025
 */

#include "WeatherApp.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>
#include <Windows.h>

 /**
  * Sets the color of text in the console.
  *
  * @param color The Windows console color code
  */
void setConsoleColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

/**
 * Clears the console screen.
 */
void clearScreen() {
    system("cls");
}

/**
 * Formats a time_t value to a human-readable string.
 *
 * @param time The time value to format
 * @return Formatted time string (HH:MM:SS DD/MM/YYYY)
 */
std::string formatTime(std::time_t time) {
    std::tm tm;
    localtime_s(&tm, &time);
    std::stringstream ss;
    ss << std::put_time(&tm, "%H:%M:%S %d/%m/%Y");
    return ss.str();
}

/**
 * Categorizes temperature into descriptions based on Celsius value.
 *
 * @param celsius Temperature in Celsius
 * @return String description of the temperature range
 */
std::string getTemperatureCategory(double celsius) {
    if (celsius < 10) return "Cold";
    if (celsius < 20) return "Cool";
    if (celsius < 25) return "Pleasant";
    if (celsius < 30) return "Warm";
    return "Hot";
}

/**
 * Displays weather data for all cities in a colorized format.
 *
 * @param weatherData Vector of WeatherData structures to display
 */
void displayWeather(const std::vector<WeatherData>& weatherData) {
    clearScreen();

    // Header
    setConsoleColor(14); // Yellow
    std::cout << "======================================================" << std::endl;
    std::cout << "                  WEATHER APP" << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << std::endl;

    if (weatherData.empty()) {
        setConsoleColor(12); // Red
        std::cout << "No cities in favorites. Add a city to view weather data." << std::endl;
        return;
    }

    // Display weather for each city
    for (const auto& weather : weatherData) {
        // City name
        setConsoleColor(15); // White
        std::cout << "City: ";
        setConsoleColor(11); // Light cyan
        std::cout << weather.city << std::endl;

        // Temperature
        setConsoleColor(15);
        std::cout << "Temperature: ";

        // Color based on temperature
        std::string tempCategory = getTemperatureCategory(weather.temperature);
        if (tempCategory == "Cold") setConsoleColor(9);      // Blue
        else if (tempCategory == "Cool") setConsoleColor(3); // Aqua
        else if (tempCategory == "Pleasant") setConsoleColor(10); // Green
        else if (tempCategory == "Warm") setConsoleColor(6); // Yellow
        else setConsoleColor(12);                            // Red

        std::cout << std::fixed << std::setprecision(1) << weather.temperature << "°C ("
            << tempCategory << ")" << std::endl;

        // Other weather details
        setConsoleColor(15);
        std::cout << "Humidity: ";
        setConsoleColor(14);
        std::cout << weather.humidity << "%" << std::endl;

        setConsoleColor(15);
        std::cout << "Wind Speed: ";
        setConsoleColor(14);
        std::cout << std::fixed << std::setprecision(1) << weather.windSpeed << " m/s" << std::endl;

        setConsoleColor(15);
        std::cout << "Description: ";
        setConsoleColor(14);
        std::cout << weather.description << std::endl;

        setConsoleColor(15);
        std::cout << "Last Updated: ";
        setConsoleColor(8); // Gray
        std::cout << formatTime(weather.timestamp) << std::endl;

        // Forecast (if available)
        if (!weather.forecast.empty()) {
            setConsoleColor(15);
            std::cout << "\nForecast:" << std::endl;

            for (const auto& forecast : weather.forecast) {
                setConsoleColor(7); // Light gray
                std::cout << forecast.date << ": ";

                // Color based on temperature
                std::string forecastTempCategory = getTemperatureCategory(forecast.temperature);
                if (forecastTempCategory == "Cold") setConsoleColor(9);
                else if (forecastTempCategory == "Cool") setConsoleColor(3);
                else if (forecastTempCategory == "Pleasant") setConsoleColor(10);
                else if (forecastTempCategory == "Warm") setConsoleColor(6);
                else setConsoleColor(12);

                std::cout << std::fixed << std::setprecision(1) << forecast.temperature << "°C ";
                setConsoleColor(8);
                std::cout << "(" << forecast.minTemp << "°C - " << forecast.maxTemp << "°C)";
                setConsoleColor(14);
                std::cout << ", " << forecast.description << std::endl;
            }
        }

        setConsoleColor(15);
        std::cout << "------------------------------------------------------" << std::endl;
    }
}

/**
 * Displays the application menu.
 */
void displayMenu() {
    setConsoleColor(10); // Green
    std::cout << std::endl;
    std::cout << "MENU:" << std::endl;
    std::cout << "1. Add city" << std::endl;
    std::cout << "2. Remove city" << std::endl;
    std::cout << "3. Refresh weather data" << std::endl;
    std::cout << "4. Set API key" << std::endl;
    std::cout << "5. Exit" << std::endl;
    std::cout << std::endl;

    setConsoleColor(15); // White
    std::cout << "Enter your choice (1-5): ";
}

/**
 * Main entry point for the application.
 */
int main() {
    // Enable UTF-8 output
    SetConsoleOutputCP(CP_UTF8);

    // Create the weather app
    WeatherApp app;

    // Variables for user input
    std::string input;
    int choice = 0;

    // Main loop
    while (true) {
        // Display current weather data
        auto weatherData = app.getWeatherData();
        displayWeather(weatherData);

        // Show menu and get user input
        displayMenu();
        std::getline(std::cin, input);

        try {
            choice = std::stoi(input);
        }
        catch (...) {
            choice = 0;
        }

        // Process user choice
        switch (choice) {
        case 1: // Add city
            setConsoleColor(15);
            std::cout << "Enter city name: ";
            std::getline(std::cin, input);
            if (!input.empty()) {
                app.addFavoriteCity(input);
                setConsoleColor(10);
                std::cout << "Adding " << input << " to favorites..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            break;

        case 2: // Remove city
            setConsoleColor(15);
            std::cout << "Enter city name to remove: ";
            std::getline(std::cin, input);
            if (!input.empty()) {
                if (app.removeFavoriteCity(input)) {
                    setConsoleColor(10);
                    std::cout << input << " was removed from favorites." << std::endl;
                }
                else {
                    setConsoleColor(12);
                    std::cout << input << " was not found in favorites." << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            break;

        case 3: // Refresh weather data
            setConsoleColor(14);
            std::cout << "Refreshing weather data..." << std::endl;
            app.updateWeatherData();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            break;

        case 4: // Set API key
            setConsoleColor(15);
            std::cout << "Enter your OpenWeatherMap API key: ";
            std::getline(std::cin, input);
            if (!input.empty()) {
                app.setApiKey(input);
                setConsoleColor(14);
                std::cout << "API key updated. Refreshing weather data..." << std::endl;
                bool success = app.updateWeatherData();
                if (success) {
                    setConsoleColor(10); // Green
                    std::cout << "Successfully updated weather data with new API key!" << std::endl;
                }
                else {
                    setConsoleColor(12); // Red
                    std::cout << "Failed to update weather with API. Check your key or internet connection." << std::endl;
                    std::cout << "Using generated data instead." << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            break;

        case 5: // Exit
            setConsoleColor(15);
            std::cout << "Exiting application. Goodbye!" << std::endl;
            return 0;

        default:
            setConsoleColor(12);
            std::cout << "Invalid choice. Please enter a number between 1 and 5." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            break;
        }
    }

    return 0;
}