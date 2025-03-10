/**
 * @file WeatherApp.cpp
 * @brief Implementation of the WeatherApp class with enhanced visual styling and fixed search
 */
#include "WeatherApp.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>

 // Constructor
WeatherApp::WeatherApp()
    : weatherApi("16ba674059f20f1fbb75756ba6397cd9"), // Replace with your actual API key
    favoriteCities("favorites.txt"),
    threadPool(4),
    window(nullptr),
    isRunning(false),
    showForecast(false),
    showFavorites(false),
    showAddCityPopup(false),
    showSettingsPopup(false) {
}

// Destructor
WeatherApp::~WeatherApp() {
    shutdown();
}

// Initialize the application
void WeatherApp::initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // Create window with larger initial size
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1400, 800, "Weather App", NULL, NULL);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Clear fonts and add large font
    io.Fonts->Clear();
    float fontSize = 18.0f; // Larger font size

    // Try to load system fonts, if not available use default with scaling
    ImFont* font = nullptr;

    // Try to load Arial font or other system fonts
    font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", fontSize);
    if (!font) font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", fontSize);
    if (!font) font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\calibri.ttf", fontSize);

    // If all attempts failed, use default font with scaling
    if (!font) {
        font = io.Fonts->AddFontDefault();
        float fontScale = 1.5f;
        ImGui::GetStyle().ScaleAllSizes(fontScale);
    }

    // Initialize style - modern and beautiful
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // Rounded corners
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 8.0f;

    // Larger spacing for better readability
    style.ItemSpacing = ImVec2(10, 10);
    style.ItemInnerSpacing = ImVec2(8, 8);
    style.FramePadding = ImVec2(12, 8);
    style.WindowPadding = ImVec2(15, 15);

    // Enhanced color scheme - blue theme
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.08f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.30f, 0.50f, 0.55f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.41f, 0.68f, 0.50f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.20f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.30f, 0.45f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.93f, 0.94f, 0.95f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.16f, 0.25f, 1.00f);

    // Initialize rendering backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Load favorite cities
    auto favorites = favoriteCities.getAllFavorites();
    for (const auto& city : favorites) {
        addCity(city);
    }

    // Add some default cities if no favorites
    if (favorites.empty()) {
        addCity("Tel Aviv");
        addCity("Jerusalem");
        addCity("Haifa");
        addCity("New York");
        addCity("London");
    }

    isRunning.store(true);
}

// Run the application
void WeatherApp::run() {
    if (!window) {
        throw std::runtime_error("No active GLFW window");
    }

    // Main rendering loop
    while (!glfwWindowShouldClose(window) && isRunning.load()) {
        glfwPollEvents();

        // Start new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render main window
        renderMainWindow();

        // Finish frame and render
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.05f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

// Shutdown the application
void WeatherApp::shutdown() {
    isRunning.store(false);

    // Clean up ImGui resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Clean up GLFW
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

// Update weather data
void WeatherApp::updateWeatherData() {
    auto cities = weatherData.getAllCities();

    for (const auto& city : cities) {
        threadPool.enqueue([this, city]() {
            try {
                auto weatherFuture = weatherApi.getCurrentWeather(city);
                auto forecastFuture = weatherApi.getForecast(city);

                auto weather = weatherFuture.get();
                auto forecast = forecastFuture.get();

                weatherData.updateCurrentWeather(weather);
                weatherData.updateForecast(city, forecast);
            }
            catch (const std::exception& e) {
                std::cerr << "Error updating weather for " << city << ": " << e.what() << std::endl;
            }
            });
    }
}

// Render the main window
void WeatherApp::renderMainWindow() {
    // Custom window style
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Weather App", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Top menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Add City")) {
                showAddCityPopup = true;
            }
            if (ImGui::MenuItem("Settings")) {
                showSettingsPopup = true;
            }
            if (ImGui::MenuItem("Refresh All")) {
                refreshWeather();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                isRunning.store(false);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Show Favorites", nullptr, &showFavorites)) {
                // toggle favorites view
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Search bar with custom style - FIXED to add cities directly
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 12)); // Larger padding for search field
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.25f, 0.30f, 0.9f));
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 120); // Make room for button
    static char searchBuffer[256] = "";
    if (ImGui::InputText("##Search", searchBuffer, sizeof(searchBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        // Add city directly when hitting Enter
        if (strlen(searchBuffer) > 0) {
            addCity(searchBuffer);
            selectedCity = searchBuffer;  // Select the city immediately
            searchQuery = "";  // Clear search filter after adding
        }
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.45f, 0.70f, 0.8f));
    if (ImGui::Button("Search", ImVec2(100, 0))) { // Wider search button
        // Add city directly when clicking Search button
        if (strlen(searchBuffer) > 0) {
            addCity(searchBuffer);
            selectedCity = searchBuffer;  // Select the city immediately
            searchQuery = "";  // Clear search filter after adding
        }
    }
    ImGui::PopStyleColor();

    // Main layout
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 320); // Wider left column

    // Left column - city list
    renderCityList();

    ImGui::NextColumn();

    // Right column - weather details
    if (!selectedCity.empty()) {
        renderWeatherDetails();

        if (showForecast) {
            renderForecast();
        }
    }
    else {
        // Show welcome screen
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::SetCursorPos(ImVec2(ImGui::GetColumnWidth() * 0.5f - 150, ImGui::GetContentRegionAvail().y * 0.4f));
        ImGui::Text("Welcome to Weather App");
        ImGui::SetCursorPosX(ImGui::GetColumnWidth() * 0.5f - 100);
        ImGui::Text("Select a city to view details");
        ImGui::PopFont();
    }

    ImGui::Columns(1);

    // Popups
    if (showAddCityPopup) {
        renderAddCityPopup();
    }

    if (showSettingsPopup) {
        renderSettingsPopup();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

// Render the city list
void WeatherApp::renderCityList() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("Cities", ImVec2(0, 0), true);

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("City List");
    if (showFavorites) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "(Favorites)");
    }
    ImGui::PopFont();

    ImGui::Separator();

    std::vector<std::string> cities;
    if (showFavorites) {
        cities = favoriteCities.getAllFavorites();
    }
    else {
        cities = weatherData.getAllCities();
    }

    // Filter by search query - keeping this for filtering existing cities
    if (!searchQuery.empty()) {
        std::vector<std::string> filteredCities;
        std::copy_if(cities.begin(), cities.end(), std::back_inserter(filteredCities),
            [this](const std::string& city) {
                return city.find(searchQuery) != std::string::npos;
            });
        cities = filteredCities;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 10)); // More space between city buttons
    for (const auto& city : cities) {
        bool isFav = favoriteCities.isFavorite(city);
        bool isSelected = city == selectedCity;

        // Button with special styling for selected city - larger and more prominent
        ImGui::PushStyleColor(ImGuiCol_Button, isSelected ?
            ImVec4(0.25f, 0.50f, 0.80f, 1.00f) :
            ImVec4(0.15f, 0.25f, 0.40f, 0.80f));

        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.60f, 1.00f, 0.90f));

        // Taller button for better visibility
        if (ImGui::Button(city.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 50))) {
            selectCity(city);
        }

        ImGui::PopStyleColor(2);

        // Star for favorites - larger and more visible
        if (isFav) {
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 35); // Position further from edge
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use larger font
            ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "★");
            ImGui::PopFont();
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
            ImGui::OpenPopup(("##CityContextMenu_" + city).c_str());
        }

        if (ImGui::BeginPopup(("##CityContextMenu_" + city).c_str())) {
            if (ImGui::MenuItem(isFav ? "Remove from Favorites" : "Add to Favorites")) {
                toggleFavorite(city);
            }
            if (ImGui::MenuItem("Refresh")) {
                addCity(city);
            }
            ImGui::EndPopup();
        }
    }
    ImGui::PopStyleVar();

    if (cities.empty()) {
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Larger font for messages
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "No cities found.");

        if (!searchQuery.empty()) {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Try a different search term.");
        }
        else if (!showFavorites) {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Add a city using the menu.");
        }
        else {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Add favorites from the city list.");
        }
        ImGui::PopFont();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// Render weather details
void WeatherApp::renderWeatherDetails() {
    WeatherInfo info;
    bool hasWeather = weatherData.getCurrentWeather(selectedCity, info);

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("WeatherDetails", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.6f), true);

    if (!hasWeather) {
        // Loading message with large font
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.0f, 1.0f), "Loading weather data for %s...", selectedCity.c_str());
        ImGui::PopFont();

        ImGui::Spacing();
        ImGui::Spacing();

        // Prominent refresh button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.52f, 0.80f, 1.00f));
        if (ImGui::Button("Refresh", ImVec2(150, 50))) {
            addCity(selectedCity);
        }
        ImGui::PopStyleColor();

        ImGui::EndChild();
        ImGui::PopStyleVar();
        return;
    }

    // City title with large font
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("%s, %s", info.cityName.c_str(), info.countryCode.c_str());
    ImGui::PopFont();

    ImGui::Separator();

    // Main info section - two columns
    ImGui::Columns(2, nullptr, false);

    // Left column - temperature and conditions
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    // Larger temperature display
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.1f°C", info.temperature);
    ImGui::PopFont();

    ImGui::Text("Feels like: %.1f°C", info.feelsLike);
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "%s", info.weatherMain.c_str());
    ImGui::Text("%s", info.weatherDescription.c_str());

    ImGui::NextColumn();

    // Right column - weather icon and action buttons
    // Show appropriate weather icon based on condition
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    if (info.weatherMain == "Clear") {
        ImGui::Text("☀️");  // sun
    }
    else if (info.weatherMain == "Clouds") {
        ImGui::Text("☁️");  // cloud
    }
    else if (info.weatherMain == "Rain") {
        ImGui::Text("🌧️");  // rain
    }
    else if (info.weatherMain == "Snow") {
        ImGui::Text("❄️");  // snow
    }
    else if (info.weatherMain == "Thunderstorm") {
        ImGui::Text("⚡");  // thunderstorm
    }
    else if (info.weatherMain == "Drizzle") {
        ImGui::Text("🌦️");  // drizzle
    }
    else if (info.weatherMain == "Mist" || info.weatherMain == "Fog") {
        ImGui::Text("🌫️");  // fog
    }
    else {
        ImGui::Text("🌤️");  // default
    }
    ImGui::PopFont();

    ImGui::Spacing();

    // Action buttons with improved styling
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.6f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.7f, 0.9f));

    if (ImGui::Button("Refresh", ImVec2(130, 40))) {
        addCity(selectedCity);
    }

    ImGui::Spacing();

    bool isFav = favoriteCities.isFavorite(selectedCity);
    if (ImGui::Button(isFav ? "★ Remove Favorite" : "☆ Add Favorite", ImVec2(180, 40))) {
        toggleFavorite(selectedCity);
    }

    ImGui::Spacing();

    if (ImGui::Button(showForecast ? "Hide Forecast" : "Show Forecast", ImVec2(150, 40))) {
        showForecast = !showForecast;
    }

    ImGui::PopStyleColor(2);

    ImGui::Columns(1);

    ImGui::Separator();

    // Additional details in a grid layout
    ImGui::Columns(2, nullptr, false);

    ImGui::Text("Min / Max:");
    ImGui::Text("Humidity:");
    ImGui::Text("Pressure:");
    ImGui::Text("Wind:");
    ImGui::Text("Sunrise:");
    ImGui::Text("Sunset:");

    ImGui::NextColumn();

    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "%.1f°C / %.1f°C", info.tempMin, info.tempMax);
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "%.1f%%", info.humidity);
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "%.1f hPa", info.pressure);
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "%.1f m/s at %.1f°", info.windSpeed, info.windDeg);

    // Format sunrise and sunset times
    auto formatTime = [](long long timestamp) {
        std::time_t time = timestamp;
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%H:%M");
        return ss.str();
        };

    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "%s", formatTime(info.sunrise).c_str());
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "%s", formatTime(info.sunset).c_str());

    ImGui::Columns(1);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Last updated: %s", info.lastUpdated.c_str());

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// Render forecast
void WeatherApp::renderForecast() {
    std::vector<ForecastInfo> forecast;
    bool hasForecast = weatherData.getForecast(selectedCity, forecast);

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("Forecast", ImVec2(0, 0), true);

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("5-Day Forecast for %s", selectedCity.c_str());
    ImGui::PopFont();

    ImGui::Separator();

    if (!hasForecast || forecast.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.0f, 1.0f), "Loading forecast data...");
        ImGui::EndChild();
        ImGui::PopStyleVar();
        return;
    }

    // Group forecast by days
    std::unordered_map<std::string, std::vector<ForecastInfo>> dailyForecasts;

    for (const auto& item : forecast) {
        std::time_t time = item.dateTime;
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d");
        dailyForecasts[ss.str()].push_back(item);
    }

    // Improved styling for forecast panels
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 12));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.15f, 0.35f, 0.6f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.25f, 0.45f, 0.7f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.20f, 0.40f, 0.65f, 1.0f));

    for (const auto& pair : dailyForecasts) {
        // Display day in format: Day name - date
        std::time_t time = pair.second[0].dateTime;
        std::stringstream headerSS;
        headerSS << std::put_time(std::localtime(&time), "%A, %d %B");

        // Collapsing headers for each day
        if (ImGui::CollapsingHeader(headerSS.str().c_str())) {
            ImGui::Columns(4, nullptr, false);

            // Column headers
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Time");
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Temp");
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Condition");
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Details");
            ImGui::NextColumn();

            ImGui::Separator();

            for (const auto& item : pair.second) {
                std::time_t itemTime = item.dateTime;
                std::stringstream ss;
                ss << std::put_time(std::localtime(&itemTime), "%H:%M");

                // Time column
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", ss.str().c_str());

                // Temperature column
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 1.0f, 1.0f), "%.1f°C", item.temperature);

                // Weather condition column
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "%s", item.weatherDescription.c_str());

                // Details column - humidity and wind
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), "💧%.0f%% 💨%.1f m/s",
                    item.humidity, item.windSpeed);

                ImGui::NextColumn();
            }

            ImGui::Columns(1);
        }
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// Render add city popup
void WeatherApp::renderAddCityPopup() {
    ImGui::SetNextWindowSize(ImVec2(450, 220), ImGuiCond_Once);
    ImGui::OpenPopup("Add City");

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));

    if (ImGui::BeginPopupModal("Add City", &showAddCityPopup)) {
        static char cityInput[256] = "";

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::Text("Enter city name:");
        ImGui::PopFont();

        // Larger input field
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));
        ImGui::InputText("##CityInput", cityInput, sizeof(cityInput));
        ImGui::PopStyleVar();

        ImGui::Spacing();
        ImGui::Spacing();

        // More prominent buttons
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.52f, 0.80f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.62f, 0.90f, 1.00f));
        if (ImGui::Button("Add", ImVec2(150, 50)) && strlen(cityInput) > 0) {
            addCity(cityInput);
            selectedCity = cityInput;  // Select the city immediately
            showAddCityPopup = false;
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.6f));
        if (ImGui::Button("Cancel", ImVec2(150, 50))) {
            showAddCityPopup = false;
        }
        ImGui::PopStyleColor();

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

// Render settings popup
// Render settings popup
void WeatherApp::renderSettingsPopup() {
    ImGui::SetNextWindowSize(ImVec2(450, 220), ImGuiCond_Once);
    ImGui::OpenPopup("Settings");

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

    if (ImGui::BeginPopupModal("Settings", &showSettingsPopup)) {
        static char apiKeyInput[256] = "";

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::Text("OpenWeatherMap API Key:");
        ImGui::PopFont();

        // Larger input field
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));
        ImGui::InputText("##ApiKeyInput", apiKeyInput, sizeof(apiKeyInput));
        ImGui::PopStyleVar();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // More prominent buttons
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.52f, 0.80f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.62f, 0.90f, 1.00f));
        if (ImGui::Button("Save", ImVec2(150, 50))) {
            // במקום להקצות אובייקט חדש, נעדכן את מפתח ה-API
            if (strlen(apiKeyInput) > 0) {
                weatherApi.updateApiKey(apiKeyInput);
                refreshWeather();  // רענון עם המפתח החדש
            }
            showSettingsPopup = false;
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.6f));
        if (ImGui::Button("Cancel", ImVec2(150, 50))) {
            showSettingsPopup = false;
        }
        ImGui::PopStyleColor();

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
}

// Select a city to display
void WeatherApp::selectCity(const std::string& cityName) {
    selectedCity = cityName;

    // If city isn't in the data, add it
    WeatherInfo info;
    if (!weatherData.getCurrentWeather(cityName, info)) {
        addCity(cityName);
    }
}

// Add a city and fetch its weather data - IMPROVED
void WeatherApp::addCity(const std::string& cityName) {
    if (cityName.empty()) {
        return;
    }

    // Print status to console
    std::cout << "Adding city: " << cityName << std::endl;

    threadPool.enqueue([this, cityName]() {
        try {
            auto weatherFuture = weatherApi.getCurrentWeather(cityName);
            auto forecastFuture = weatherApi.getForecast(cityName);

            auto weather = weatherFuture.get();
            auto forecast = forecastFuture.get();

            weatherData.updateCurrentWeather(weather);
            weatherData.updateForecast(cityName, forecast);

            // No need to update selectedCity here as it's done when calling the function
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding city " << cityName << ": " << e.what() << std::endl;
        }
        });
}

// Refresh weather data for all cities
void WeatherApp::refreshWeather() {
    updateWeatherData();
}

// Toggle favorite status for a city
void WeatherApp::toggleFavorite(const std::string& cityName) {
    if (favoriteCities.isFavorite(cityName)) {
        favoriteCities.removeFavorite(cityName);
    }
    else {
        favoriteCities.addFavorite(cityName);
    }
}

// Set the search query
void WeatherApp::setSearchQuery(const std::string& query) {
    searchQuery = query;
}
void WeatherAPI::updateApiKey(const std::string& newApiKey) {
    this->apiKey = newApiKey;
}