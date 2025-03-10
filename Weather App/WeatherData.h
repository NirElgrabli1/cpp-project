/**
 * @file WeatherData.h
 * @brief Data structures and management for weather information
 */
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <mutex>

 /**
  * @struct WeatherInfo
  * @brief Structure to store current weather information for a city
  */
struct WeatherInfo {
    std::string cityName;
    std::string countryCode;
    double temperature;
    double feelsLike;
    double tempMin;
    double tempMax;
    double pressure;
    double humidity;
    double windSpeed;
    double windDeg;
    std::string weatherMain;
    std::string weatherDescription;
    std::string weatherIcon;
    long long sunrise;
    long long sunset;
    std::string lastUpdated;
};

/**
 * @struct ForecastInfo
 * @brief Structure to store forecast information for a specific time
 */
struct ForecastInfo {
    long long dateTime;
    double temperature;
    double feelsLike;
    double tempMin;
    double tempMax;
    double pressure;
    double humidity;
    double windSpeed;
    double windDeg;
    std::string weatherMain;
    std::string weatherDescription;
    std::string weatherIcon;
};

/**
 * @class WeatherData
 * @brief Class for managing weather data with thread-safe access
 */
class WeatherData {
private:
    std::unordered_map<std::string, WeatherInfo> currentWeather;
    std::unordered_map<std::string, std::vector<ForecastInfo>> forecasts;
    mutable std::mutex weatherMutex;
    mutable std::mutex forecastMutex;

public:
    WeatherData() = default;
    ~WeatherData() = default;

    void updateCurrentWeather(const WeatherInfo& info);
    void updateForecast(const std::string& cityName, const std::vector<ForecastInfo>& forecastData);
    bool getCurrentWeather(const std::string& cityName, WeatherInfo& info) const;
    bool getForecast(const std::string& cityName, std::vector<ForecastInfo>& forecastData) const;
    std::vector<std::string> getAllCities() const;
    void clearData();
};