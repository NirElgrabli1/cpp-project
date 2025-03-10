/**
 * @file FavoriteCities.cpp
 * @brief Implementation of the FavoriteCities class
 */
#include "FavoriteCities.h"
#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;

FavoriteCities::FavoriteCities(const std::string& saveFilePath)
    : saveFilePath(saveFilePath) {
    loadFromFile();
}

FavoriteCities::~FavoriteCities() {
    saveToFile();
}

void FavoriteCities::addFavorite(const std::string& cityName) {
    std::lock_guard<std::mutex> lock(favoritesMutex);
    favorites.insert(cityName);
    saveToFile();
}

void FavoriteCities::removeFavorite(const std::string& cityName) {
    std::lock_guard<std::mutex> lock(favoritesMutex);
    favorites.erase(cityName);
    saveToFile();
}

bool FavoriteCities::isFavorite(const std::string& cityName) const {
    std::lock_guard<std::mutex> lock(favoritesMutex);
    return favorites.find(cityName) != favorites.end();
}

std::vector<std::string> FavoriteCities::getAllFavorites() const {
    std::lock_guard<std::mutex> lock(favoritesMutex);
    std::vector<std::string> result;
    result.reserve(favorites.size());
    for (const auto& city : favorites) {
        result.push_back(city);
    }
    return result;
}

void FavoriteCities::loadFromFile() {
    std::lock_guard<std::mutex> lock(favoritesMutex);
    favorites.clear();

    if (!fs::exists(saveFilePath)) {
        return;
    }

    std::ifstream file(saveFilePath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                favorites.insert(line);
            }
        }
        file.close();
    }
}

void FavoriteCities::saveToFile() const {
    std::lock_guard<std::mutex> lock(favoritesMutex);

    fs::path path(saveFilePath);
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }

    std::ofstream file(saveFilePath);
    if (file.is_open()) {
        for (const auto& city : favorites) {
            file << city << std::endl;
        }
        file.close();
    }
}