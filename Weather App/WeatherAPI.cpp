/**
 * @file WeatherAPI.cpp
 * @brief Implementation of the WeatherAPI class with improved error handling
 */
#include "WeatherAPI.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

WeatherAPI::WeatherAPI(const std::string& apiKey)
    : apiKey(apiKey), baseUrl("api.openweathermap.org"), isRunning(true) {
}

WeatherAPI::~WeatherAPI() {
    cancel();
}

void WeatherAPI::cancel() {
    isRunning.store(false);
}

// Encode URL to handle spaces and special characters
std::string encodeURL(const std::string& input) {
    std::string result = input;
    std::string::size_type pos = 0;

    // Replace spaces with %20
    while ((pos = result.find(' ', pos)) != std::string::npos) {
        result.replace(pos, 1, "%20");
        pos += 3;
    }

    return result;
}

WeatherInfo WeatherAPI::parseCurrentWeatherJson(const json& data) {
    WeatherInfo info;

    info.cityName = data["name"].get<std::string>();
    info.countryCode = data["sys"]["country"].get<std::string>();

    // Temperature is already in Celsius because we use units=metric
    info.temperature = data["main"]["temp"].get<double>();
    info.feelsLike = data["main"]["feels_like"].get<double>();
    info.tempMin = data["main"]["temp_min"].get<double>();
    info.tempMax = data["main"]["temp_max"].get<double>();
    info.pressure = data["main"]["pressure"].get<double>();
    info.humidity = data["main"]["humidity"].get<double>();

    if (!data["wind"].is_null()) {
        info.windSpeed = data["wind"]["speed"].get<double>();
        info.windDeg = data["wind"]["deg"].get<double>();
    }

    if (!data["weather"].empty()) {
        info.weatherMain = data["weather"][0]["main"].get<std::string>();
        info.weatherDescription = data["weather"][0]["description"].get<std::string>();
        info.weatherIcon = data["weather"][0]["icon"].get<std::string>();
    }

    info.sunrise = data["sys"]["sunrise"].get<long long>();
    info.sunset = data["sys"]["sunset"].get<long long>();

    // Format current time as string
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    info.lastUpdated = ss.str();

    return info;
}

std::vector<ForecastInfo> WeatherAPI::parseForecastJson(const json& data) {
    std::vector<ForecastInfo> forecastList;

    for (const auto& item : data["list"]) {
        ForecastInfo forecast;

        forecast.dateTime = item["dt"].get<long long>();
        // Already in Celsius because of units=metric
        forecast.temperature = item["main"]["temp"].get<double>();
        forecast.feelsLike = item["main"]["feels_like"].get<double>();
        forecast.tempMin = item["main"]["temp_min"].get<double>();
        forecast.tempMax = item["main"]["temp_max"].get<double>();
        forecast.pressure = item["main"]["pressure"].get<double>();
        forecast.humidity = item["main"]["humidity"].get<double>();

        if (!item["wind"].is_null()) {
            forecast.windSpeed = item["wind"]["speed"].get<double>();
            forecast.windDeg = item["wind"]["deg"].get<double>();
        }

        if (!item["weather"].empty()) {
            forecast.weatherMain = item["weather"][0]["main"].get<std::string>();
            forecast.weatherDescription = item["weather"][0]["description"].get<std::string>();
            forecast.weatherIcon = item["weather"][0]["icon"].get<std::string>();
        }

        forecastList.push_back(forecast);
    }

    return forecastList;
}

std::future<WeatherInfo> WeatherAPI::getCurrentWeather(const std::string& cityName) {
    return std::async(std::launch::async, [this, cityName]() {
        if (!isRunning.load()) {
            throw std::runtime_error("API operation canceled");
        }

        // Properly encode the city name
        std::string encodedCity = encodeURL(cityName);

        httplib::Client cli(baseUrl);
        // Use units=metric to get Celsius temperatures
        std::string path = "/data/2.5/weather?q=" + encodedCity + "&appid=" + apiKey + "&units=metric";

        auto res = cli.Get(path.c_str());
        if (res && res->status == 200) {
            json data = json::parse(res->body);
            return parseCurrentWeatherJson(data);
        }
        else {
            std::string errorMsg = "Failed to get weather data";
            if (res) {
                errorMsg += ": " + std::to_string(res->status);
            }
            throw std::runtime_error(errorMsg);
        }
        });
}

std::future<std::vector<ForecastInfo>> WeatherAPI::getForecast(const std::string& cityName, int days) {
    return std::async(std::launch::async, [this, cityName, days]() {
        if (!isRunning.load()) {
            throw std::runtime_error("API operation canceled");
        }

        // Properly encode the city name
        std::string encodedCity = encodeURL(cityName);

        httplib::Client cli(baseUrl);
        // Use units=metric to get Celsius temperatures
        std::string path = "/data/2.5/forecast?q=" + encodedCity + "&cnt=" + std::to_string(days * 8) + "&appid=" + apiKey + "&units=metric";

        auto res = cli.Get(path.c_str());
        if (res && res->status == 200) {
            json data = json::parse(res->body);
            return parseForecastJson(data);
        }
        else {
            std::string errorMsg = "Failed to get forecast data";
            if (res) {
                errorMsg += ": " + std::to_string(res->status);
            }
            throw std::runtime_error(errorMsg);
        }
        });
}

std::future<std::vector<std::string>> WeatherAPI::searchCity(const std::string& query) {
    return std::async(std::launch::async, [this, query]() {
        if (!isRunning.load()) {
            throw std::runtime_error("API operation canceled");
        }

        // Properly encode the query
        std::string encodedQuery = encodeURL(query);

        httplib::Client cli(baseUrl);
        std::string path = "/geo/1.0/direct?q=" + encodedQuery + "&limit=5&appid=" + apiKey;

        auto res = cli.Get(path.c_str());
        if (res && res->status == 200) {
            json data = json::parse(res->body);
            std::vector<std::string> cities;

            for (const auto& city : data) {
                std::string cityName = city["name"].get<std::string>();
                std::string country = city["country"].get<std::string>();
                cities.push_back(cityName + ", " + country);
            }

            return cities;
        }
        else {
            std::string errorMsg = "Failed to search cities";
            if (res) {
                errorMsg += ": " + std::to_string(res->status);
            }
            throw std::runtime_error(errorMsg);
        }
        });
}