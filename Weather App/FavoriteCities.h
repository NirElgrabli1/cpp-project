/**
 * @file FavoriteCities.h
 * @brief Management of favorite cities with persistence
 */
#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <fstream>
#include <filesystem>

 /**
  * @class FavoriteCities
  * @brief Class for managing favorite cities with thread-safe access and file persistence
  */
class FavoriteCities {
private:
    std::unordered_set<std::string> favorites;
    mutable std::mutex favoritesMutex;
    std::string saveFilePath;

public:
    FavoriteCities(const std::string& saveFilePath);
    ~FavoriteCities();

    void addFavorite(const std::string& cityName);
    void removeFavorite(const std::string& cityName);
    bool isFavorite(const std::string& cityName) const;
    std::vector<std::string> getAllFavorites() const;

    void loadFromFile();
    void saveToFile() const;
};