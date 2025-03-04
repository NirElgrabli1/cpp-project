#include "WeatherApp.h"
#include "WeatherAPI.h"
#include <random>
#include <algorithm>
#include <thread>
#include <iostream>

void WeatherApp::loadApiKey() {
    // Load API key from file if it exists
    std::string loadedKey = "";
    std::ifstream apiKeyFile("api_key.txt");
    if (apiKeyFile.is_open()) {
        std::getline(apiKeyFile, loadedKey);
        apiKeyFile.close();
    }

    // Initialize API with loaded key or default
    if (!loadedKey.empty()) {
        setApiKey(loadedKey);
    }
    else {
        // Use default key (should be replaced with real key)
        setApiKey("YOUR_API_KEY_HERE");
    }
}

WeatherApp::WeatherApp() {
    // Load saved data
    loadData();

    // Add default cities if none exist
    if (favoriteCities.empty()) {
        favoriteCities = { "Tel Aviv", "Jerusalem", "Haifa" };
    }

    // Load API key
    loadApiKey();

    // Initial weather update
    updateWeatherData();

    // Start background update thread
    std::thread([this]() {
        this->backgroundUpdateThread();
        }).detach();
}

WeatherApp::~WeatherApp() {
    isRunning = false;
    saveData();
}

void WeatherApp::backgroundUpdateThread() {
    while (isRunning) {
        // Wait for 5 minutes
        for (int i = 0; i < 300 && isRunning; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (isRunning) {
            updateWeatherData();
        }
    }
}

void WeatherApp::setApiKey(const std::string& key) {
    api = std::make_unique<WeatherAPI>(key);

    // Save API key to file
    std::ofstream apiKeyFile("api_key.txt");
    if (apiKeyFile.is_open()) {
        apiKeyFile << key;
    }
    else {
        std::cerr << "Failed to save API key to file" << std::endl;
    }
}

bool WeatherApp::updateWeatherData() {
    std::vector<std::string> citiesToUpdate;
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        citiesToUpdate = favoriteCities;
    }

    bool anySuccess = false;

    for (const auto& city : citiesToUpdate) {
        if (!isRunning) break;

        WeatherData weather;
        bool success = false;

        // Try to get data from API
        if (api) {
            auto result = api->getCurrentWeather(city);
            if (result) {
                weather = *result;
                api->getForecast(weather);
                success = true;
                anySuccess = true; // At least one city succeeded
            }
        }

        // Fall back to generated data if API failed
        if (!success) {
            std::cerr << "API request failed for " << city << ". Using generated data instead." << std::endl;
            weather = generateWeatherData(city);
        }

        std::lock_guard<std::mutex> lock(cacheMutex);
        weatherCache[city] = weather;
    }

    saveData();
    return anySuccess;
}

WeatherData WeatherApp::generateWeatherData(const std::string& city) {
    // Create random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> tempDist(5.0, 35.0);
    std::uniform_real_distribution<> windDist(0.0, 20.0);
    std::uniform_int_distribution<> humidityDist(30, 90);

    // List of possible weather descriptions
    std::vector<std::string> descriptions = {
        "Clear sky", "Few clouds", "Scattered clouds", "Broken clouds",
        "Shower rain", "Rain", "Thunderstorm", "Snow", "Mist"
    };
    std::uniform_int_distribution<> descIdx(0, static_cast<int>(descriptions.size()) - 1);

    // Generate current weather
    WeatherData weather;
    weather.city = city;
    weather.temperature = tempDist(gen);
    weather.humidity = humidityDist(gen);
    weather.windSpeed = windDist(gen);
    weather.description = descriptions[descIdx(gen)];
    weather.timestamp = std::time(nullptr);

    // Generate 5-day forecast
    for (int i = 1; i <= 5; i++) {
        WeatherData::Forecast forecast;

        // Format date (current date + i days)
        std::time_t futureTime = std::time(nullptr) + i * 24 * 60 * 60;
        std::tm* future = std::localtime(&futureTime);
        char dateStr[11];
        std::strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", future);
        forecast.date = dateStr;

        // Generate forecast data
        forecast.temperature = tempDist(gen);
        forecast.minTemp = forecast.temperature - 5.0 + (gen() % 3);
        forecast.maxTemp = forecast.temperature + (gen() % 5);
        forecast.humidity = humidityDist(gen);
        forecast.windSpeed = windDist(gen);
        forecast.description = descriptions[descIdx(gen)];

        weather.forecast.push_back(forecast);
    }

    return weather;
}

std::vector<WeatherData> WeatherApp::getWeatherData() const {
    std::lock_guard<std::mutex> lock(cacheMutex);

    std::vector<WeatherData> result;
    for (const auto& city : favoriteCities) {
        auto it = weatherCache.find(city);
        if (it != weatherCache.end()) {
            result.push_back(it->second);
        }
    }

    return result;
}

bool WeatherApp::addFavoriteCity(const std::string& city) {
    if (city.empty()) return false;

    {
        std::lock_guard<std::mutex> lock(cacheMutex);

        // Check if city already exists
        if (std::find(favoriteCities.begin(), favoriteCities.end(), city) != favoriteCities.end()) {
            return false;
        }

        // Add to favorites
        favoriteCities.push_back(city);
    }

    // Generate weather in background
    std::thread([this, city]() {
        WeatherData weather = generateWeatherData(city);

        std::lock_guard<std::mutex> lock(cacheMutex);
        weatherCache[city] = weather;
        saveData();
        }).detach();

    return true;
}

bool WeatherApp::removeFavoriteCity(const std::string& city) {
    std::lock_guard<std::mutex> lock(cacheMutex);

    auto it = std::find(favoriteCities.begin(), favoriteCities.end(), city);
    if (it != favoriteCities.end()) {
        favoriteCities.erase(it);
        weatherCache.erase(city);
        saveData();
        return true;
    }

    return false;
}

bool WeatherApp::isCityInFavorites(const std::string& city) const {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return std::find(favoriteCities.begin(), favoriteCities.end(), city) != favoriteCities.end();
}

void WeatherApp::saveData() {
    try {
        // Save favorites
        std::ofstream favoritesFile("favorites.txt");
        for (const auto& city : favoriteCities) {
            favoritesFile << city << std::endl;
        }

        // Save weather cache (basic version)
        std::ofstream cacheFile("weather_cache.txt");
        for (const auto& pair : weatherCache) {
            const std::string& city = pair.first;
            const WeatherData& weather = pair.second;

            cacheFile << "CITY:" << city << std::endl;
            cacheFile << "TEMP:" << weather.temperature << std::endl;
            cacheFile << "HUM:" << weather.humidity << std::endl;
            cacheFile << "WIND:" << weather.windSpeed << std::endl;
            cacheFile << "DESC:" << weather.description << std::endl;
            cacheFile << "TIME:" << weather.timestamp << std::endl;

            // Save forecasts
            cacheFile << "FCCOUNT:" << weather.forecast.size() << std::endl;

            for (const auto& fc : weather.forecast) {
                cacheFile << "FC_DATE:" << fc.date << std::endl;
                cacheFile << "FC_TEMP:" << fc.temperature << std::endl;
                cacheFile << "FC_MIN:" << fc.minTemp << std::endl;
                cacheFile << "FC_MAX:" << fc.maxTemp << std::endl;
                cacheFile << "FC_HUM:" << fc.humidity << std::endl;
                cacheFile << "FC_WIND:" << fc.windSpeed << std::endl;
                cacheFile << "FC_DESC:" << fc.description << std::endl;
            }

            cacheFile << "END_CITY" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving data: " << e.what() << std::endl;
    }
}

void WeatherApp::loadData() {
    try {
        // Load favorites
        std::ifstream favoritesFile("favorites.txt");
        if (favoritesFile.is_open()) {
            std::string line;
            while (std::getline(favoritesFile, line)) {
                if (!line.empty()) {
                    favoriteCities.push_back(line);
                }
            }
        }

        // Load weather cache
        std::ifstream cacheFile("weather_cache.txt");
        if (cacheFile.is_open()) {
            std::string line, key, value;
            WeatherData currentWeather;
            bool inCity = false;

            while (std::getline(cacheFile, line)) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    key = line.substr(0, pos);
                    value = line.substr(pos + 1);

                    if (key == "CITY") {
                        currentWeather = WeatherData();
                        currentWeather.city = value;
                        inCity = true;
                    }
                    else if (key == "TEMP" && inCity) {
                        currentWeather.temperature = std::stod(value);
                    }
                    else if (key == "HUM" && inCity) {
                        currentWeather.humidity = std::stoi(value);
                    }
                    else if (key == "WIND" && inCity) {
                        currentWeather.windSpeed = std::stod(value);
                    }
                    else if (key == "DESC" && inCity) {
                        currentWeather.description = value;
                    }
                    else if (key == "TIME" && inCity) {
                        currentWeather.timestamp = std::stoll(value);
                    }
                    else if (key == "FCCOUNT" && inCity) {
                        // Just a count marker, no action needed
                    }
                    else if (key == "FC_DATE" && inCity) {
                        WeatherData::Forecast fc;
                        fc.date = value;

                        // Assume the next 6 lines are the rest of the forecast data
                        if (std::getline(cacheFile, line)) {
                            pos = line.find(':');
                            fc.temperature = std::stod(line.substr(pos + 1));
                        }
                        if (std::getline(cacheFile, line)) {
                            pos = line.find(':');
                            fc.minTemp = std::stod(line.substr(pos + 1));
                        }
                        if (std::getline(cacheFile, line)) {
                            pos = line.find(':');
                            fc.maxTemp = std::stod(line.substr(pos + 1));
                        }
                        if (std::getline(cacheFile, line)) {
                            pos = line.find(':');
                            fc.humidity = std::stoi(line.substr(pos + 1));
                        }
                        if (std::getline(cacheFile, line)) {
                            pos = line.find(':');
                            fc.windSpeed = std::stod(line.substr(pos + 1));
                        }
                        if (std::getline(cacheFile, line)) {
                            pos = line.find(':');
                            fc.description = line.substr(pos + 1);
                        }

                        currentWeather.forecast.push_back(fc);
                    }
                }
                else if (line == "END_CITY" && inCity) {
                    weatherCache[currentWeather.city] = currentWeather;
                    inCity = false;
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading data: " << e.what() << std::endl;
    }
}