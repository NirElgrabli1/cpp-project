/**
 * @file WeatherData.cpp
 * @brief Implementation of the WeatherData class
 */
#include "WeatherData.h"
#include <algorithm>

void WeatherData::updateCurrentWeather(const WeatherInfo& info) {
    std::lock_guard<std::mutex> lock(weatherMutex);
    currentWeather[info.cityName] = info;
}

void WeatherData::updateForecast(const std::string& cityName, const std::vector<ForecastInfo>& forecastData) {
    std::lock_guard<std::mutex> lock(forecastMutex);
    forecasts[cityName] = forecastData;
}

bool WeatherData::getCurrentWeather(const std::string& cityName, WeatherInfo& info) const {
    std::lock_guard<std::mutex> lock(weatherMutex);
    auto it = currentWeather.find(cityName);
    if (it != currentWeather.end()) {
        info = it->second;
        return true;
    }
    return false;
}

bool WeatherData::getForecast(const std::string& cityName, std::vector<ForecastInfo>& forecastData) const {
    std::lock_guard<std::mutex> lock(forecastMutex);
    auto it = forecasts.find(cityName);
    if (it != forecasts.end()) {
        forecastData = it->second;
        return true;
    }
    return false;
}

std::vector<std::string> WeatherData::getAllCities() const {
    std::lock_guard<std::mutex> lock(weatherMutex);
    std::vector<std::string> cities;
    cities.reserve(currentWeather.size());
    for (const auto& pair : currentWeather) {
        cities.push_back(pair.first);
    }
    return cities;
}

void WeatherData::clearData() {
    {
        std::lock_guard<std::mutex> lock(weatherMutex);
        currentWeather.clear();
    }
    {
        std::lock_guard<std::mutex> lock(forecastMutex);
        forecasts.clear();
    }
}