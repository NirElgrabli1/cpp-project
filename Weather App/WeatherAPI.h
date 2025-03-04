// WeatherAPI.h
#pragma once

#include <string>
#include <optional>
#include "WeatherApp.h"

class WeatherAPI {
private:
    std::string apiKey;
    // Keep using HTTP for now since your httplib may not support SSL
    const std::string baseUrl = "http://api.openweathermap.org/data/2.5/";

public:
    WeatherAPI(const std::string& key);

    // Fetch current weather data for a city
    std::optional<WeatherData> getCurrentWeather(const std::string& city);

    // Fetch 5-day forecast for a city
    bool getForecast(WeatherData& weatherData);

    // Parse JSON response into WeatherData
    WeatherData parseCurrentWeather(const std::string& jsonResponse, const std::string& city);
    std::vector<WeatherData::Forecast> parseForecast(const std::string& jsonResponse);
};