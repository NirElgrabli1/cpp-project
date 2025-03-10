/**
 * @file WeatherApp.h
 * @brief Main application class with GUI implementation
 */
#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include "WeatherData.h"
#include "WeatherAPI.h"
#include "FavoriteCities.h"

 // Forward declarations
struct GLFWwindow;

/**
 * @class ThreadPool
 * @brief Thread pool for executing tasks in parallel
 */
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop.load()) {
                throw std::runtime_error("Enqueue on stopped ThreadPool");
            }
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }
};

/**
 * @class WeatherApp
 * @brief Main application class with GUI and weather data management
 */
class WeatherApp {
private:
    // Core components
    WeatherData weatherData;
    WeatherAPI weatherApi;
    FavoriteCities favoriteCities;
    ThreadPool threadPool;

    // GLFW and GUI
    GLFWwindow* window;

    // Application state
    std::atomic<bool> isRunning;
    std::string selectedCity;
    std::string searchQuery;
    bool showForecast;
    bool showFavorites;
    bool showAddCityPopup;
    bool showSettingsPopup;

    // Rendering methods
    void updateWeatherData();
    void renderMainWindow();
    void renderCityList();
    void renderWeatherDetails();
    void renderForecast();
    void renderAddCityPopup();
    void renderSettingsPopup();

public:
    WeatherApp();
    ~WeatherApp();

    void initialize();
    void run();
    void shutdown();

    void selectCity(const std::string& cityName);
    void addCity(const std::string& cityName);
    void refreshWeather();
    void toggleFavorite(const std::string& cityName);
    void setSearchQuery(const std::string& query);
};