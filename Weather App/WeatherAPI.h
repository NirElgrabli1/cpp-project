/**
 * @file WeatherAPI.h
 * @brief Interface for retrieving weather data from API
 */
#pragma once
#include <string>
#include <vector>
#include <future>
#include <atomic>
#include "WeatherData.h"
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

/**
 * @class WeatherAPI
 * @brief Class for interacting with the OpenWeatherMap API
 */
class WeatherAPI {
private:
    std::string apiKey;
    std::string baseUrl;
    std::atomic<bool> isRunning;

    WeatherInfo parseCurrentWeatherJson(const json& json);
    std::vector<ForecastInfo> parseForecastJson(const json& json);

public:
    WeatherAPI(const std::string& apiKey);
    ~WeatherAPI();

    std::future<WeatherInfo> getCurrentWeather(const std::string& cityName);
    std::future<std::vector<ForecastInfo>> getForecast(const std::string& cityName, int days = 5);
    std::future<std::vector<std::string>> searchCity(const std::string& query);
    void cancel();
    void updateApiKey(const std::string& newApiKey);

};