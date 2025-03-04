// WeatherAPI.cpp
#include "WeatherAPI.h"
#include "httplib.h"
#include "json.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

WeatherAPI::WeatherAPI(const std::string& key) : apiKey(key) {}

std::optional<WeatherData> WeatherAPI::getCurrentWeather(const std::string& city) {
    try {
        // Create httplib client (standard HTTP version)
        httplib::Client cli("api.openweathermap.org");

        // Set timeouts to prevent hanging
        cli.set_connection_timeout(5, 0); // 5 seconds
        cli.set_read_timeout(5, 0); // 5 seconds

        // Construct the API endpoint
        std::string endpoint = "/data/2.5/weather?q=" + httplib::detail::encode_url(city) +
            "&units=metric&appid=" + apiKey;

        // Make the request
        auto res = cli.Get(endpoint.c_str());

        // Check if request was successful
        if (res && res->status == 200) {
            // Parse the response
            return parseCurrentWeather(res->body, city);
        }
        else {
            std::cerr << "API request failed: ";
            if (res) {
                std::cerr << "Status: " << res->status << " - " << res->body;
            }
            else {
                std::cerr << "connection error";
            }
            std::cerr << std::endl;
            return std::nullopt;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error fetching weather data: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool WeatherAPI::getForecast(WeatherData& weatherData) {
    try {
        // Create httplib client
        httplib::Client cli("api.openweathermap.org");

        // Set timeouts to prevent hanging
        cli.set_connection_timeout(5, 0); // 5 seconds
        cli.set_read_timeout(5, 0); // 5 seconds

        // Construct the API endpoint
        std::string endpoint = "/data/2.5/forecast?q=" + httplib::detail::encode_url(weatherData.city) +
            "&units=metric&appid=" + apiKey;

        // Make the request
        auto res = cli.Get(endpoint.c_str());

        // Check if request was successful
        if (res && res->status == 200) {
            // Parse the response
            weatherData.forecast = parseForecast(res->body);
            return true;
        }
        else {
            std::cerr << "Forecast API request failed: ";
            if (res) {
                std::cerr << "Status: " << res->status << " - " << res->body;
            }
            else {
                std::cerr << "connection error";
            }
            std::cerr << std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error fetching forecast data: " << e.what() << std::endl;
        return false;
    }
}

// The rest of the file stays the same
WeatherData WeatherAPI::parseCurrentWeather(const std::string& jsonResponse, const std::string& city) {
    WeatherData weather;
    weather.city = city;

    try {
        json data = json::parse(jsonResponse);

        // Parse main weather data
        weather.temperature = data["main"]["temp"].get<double>();
        weather.humidity = data["main"]["humidity"].get<int>();
        weather.windSpeed = data["wind"]["speed"].get<double>();

        // Parse weather description
        if (!data["weather"].empty()) {
            weather.description = data["weather"][0]["description"].get<std::string>();
        }
        else {
            weather.description = "No description available";
        }

        // Set timestamp
        weather.timestamp = std::time(nullptr);

    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing weather JSON: " << e.what() << std::endl;
    }

    return weather;
}

std::vector<WeatherData::Forecast> WeatherAPI::parseForecast(const std::string& jsonResponse) {
    // This stays the same as in your original code
    // ...
    std::vector<WeatherData::Forecast> forecasts;

    try {
        json data = json::parse(jsonResponse);

        if (!data["list"].empty()) {
            std::string currentDate = "";
            WeatherData::Forecast dailyForecast;

            // Track min/max temperatures for each day
            double minTemp = 100.0;
            double maxTemp = -100.0;

            for (const auto& item : data["list"]) {
                // Extract date from timestamp
                std::time_t timestamp = item["dt"].get<int>();
                std::tm* timeinfo = std::localtime(&timestamp);

                char dateStr[11];
                std::strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", timeinfo);
                std::string itemDate(dateStr);

                // If new date or first item
                if (currentDate != itemDate) {
                    // Save previous date's forecast (except for first iteration)
                    if (!currentDate.empty()) {
                        dailyForecast.minTemp = minTemp;
                        dailyForecast.maxTemp = maxTemp;
                        forecasts.push_back(dailyForecast);
                    }

                    // Reset for new date
                    currentDate = itemDate;
                    dailyForecast = WeatherData::Forecast();
                    dailyForecast.date = itemDate;
                    minTemp = 100.0;
                    maxTemp = -100.0;

                    // Use noon data for the day's representative forecast
                    if (timeinfo->tm_hour >= 11 && timeinfo->tm_hour <= 13) {
                        dailyForecast.temperature = item["main"]["temp"].get<double>();
                        dailyForecast.humidity = item["main"]["humidity"].get<int>();
                        dailyForecast.windSpeed = item["wind"]["speed"].get<double>();

                        if (!item["weather"].empty()) {
                            dailyForecast.description = item["weather"][0]["description"].get<std::string>();
                        }
                    }
                }

                // Update min/max temperatures
                double temp = item["main"]["temp"].get<double>();
                if (temp < minTemp) minTemp = temp;
                if (temp > maxTemp) maxTemp = temp;
            }

            // Add the last day's forecast
            if (!currentDate.empty()) {
                dailyForecast.minTemp = minTemp;
                dailyForecast.maxTemp = maxTemp;
                forecasts.push_back(dailyForecast);
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing forecast JSON: " << e.what() << std::endl;
    }

    // Limit to 5 days
    if (forecasts.size() > 5) {
        forecasts.resize(5);
    }

    return forecasts;
}