# CMakeLists.txt
# Weather App - CMake Build Configuration
# 
# This file configures the build process for the Weather App project.
# It specifies dependencies, compiler settings, and links libraries.
#
# Requirements:
# - CMake 3.12 or higher
# - C++17 compatible compiler
# - OpenGL and GLFW for GUI (optional)
# - External libraries: httplib, json

cmake_minimum_required(VERSION 3.12)
project(WeatherApp VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/json/include
    ${CMAKE_SOURCE_DIR}/external/httplib
)

# Source files
set(SOURCES
    src/main.cpp
    src/WeatherApp.cpp
    src/WeatherAPI.cpp
)

# Header files
set(HEADERS
    include/WeatherApp.h
    include/WeatherAPI.h
)

# Add Windows-specific dependencies
if(WIN32)
    set(PLATFORM_LIBS ws2_32)
endif()

# Define executable
add_executable(WeatherApp ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(WeatherApp PRIVATE ${PLATFORM_LIBS})

# Additional compiler options
if(MSVC)
    # Windows/MSVC-specific flags
    target_compile_options(WeatherApp PRIVATE /W4)
else()
    # GCC/Clang flags
    target_compile_options(WeatherApp PRIVATE -Wall -Wextra)
endif()

# Install targets
install(TARGETS WeatherApp 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Copy necessary config files to build directory
configure_file(${CMAKE_SOURCE_DIR}/config/api_key.txt 
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/api_key.txt COPYONLY)

# Print configuration summary
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Optional GUI components
option(BUILD_GUI "Build with GUI support" OFF)
if(BUILD_GUI)
    # Find required packages for GUI
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    
    # Add imgui as a library
    file(GLOB IMGUI_SOURCES 
        ${CMAKE_SOURCE_DIR}/external/imgui/*.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
    )
    
    add_library(imgui STATIC ${IMGUI_SOURCES})
    target_include_directories(imgui PUBLIC 
        ${CMAKE_SOURCE_DIR}/external/imgui
        ${CMAKE_SOURCE_DIR}/external/imgui/backends
    )
    
    # Add GUI source
    list(APPEND SOURCES src/GUI.cpp)
    list(APPEND HEADERS include/GUI.h)
    
    # Link GUI libraries
    target_link_libraries(WeatherApp PRIVATE
        imgui
        OpenGL::GL
        glfw
    )
    
    target_compile_definitions(WeatherApp PRIVATE USE_GUI)
endif()