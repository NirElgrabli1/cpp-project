// Update the WeatherApp class to make updateWeatherData return a bool
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <ctime>
#include <fstream>
#include <memory>

// Forward declaration
class WeatherAPI;

// Weather data structure
struct WeatherData {
    std::string city;
    double temperature;
    int humidity;
    double windSpeed;
    std::string description;
    std::time_t timestamp;

    // Forecast structure
    struct Forecast {
        std::string date;
        double temperature;
        double minTemp;
        double maxTemp;
        int humidity;
        double windSpeed;
        std::string description;
    };

    // Vector of forecasts
    std::vector<Forecast> forecast;
};

// Main application class
class WeatherApp {
private:
    std::vector<std::string> favoriteCities;
    std::unordered_map<std::string, WeatherData> weatherCache;
    mutable std::mutex cacheMutex;
    std::atomic<bool> isRunning{ true };
    std::unique_ptr<WeatherAPI> api;

    // Generate synthetic weather data (fallback if API fails)
    WeatherData generateWeatherData(const std::string& city);

    // Background update thread
    void backgroundUpdateThread();

    // Save and load data
    void saveData();
    void loadData();
    void loadApiKey();  // New method to load API key

public:
    // Constructor
    WeatherApp();

    // Destructor
    ~WeatherApp();

    // Update weather data for all favorite cities - return true if at least one city updated successfully
    bool updateWeatherData();

    // Get weather data for all favorite cities
    std::vector<WeatherData> getWeatherData() const;

    // Add a city to favorites
    bool addFavoriteCity(const std::string& city);

    // Remove a city from favorites
    bool removeFavoriteCity(const std::string& city);

    // Check if a city is in favorites
    bool isCityInFavorites(const std::string& city) const;

    // Set API key
    void setApiKey(const std::string& key);
};