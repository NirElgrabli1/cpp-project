# הגדרת המקור לImGui כולל backends
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)  

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# מציאת OpenGL
find_package(OpenGL REQUIRED)

# הוספת כל הקבצים לפרויקט
add_executable(WeatherApp 
    src/Main.cpp 
    src/WeatherApp.cpp
    src/WeatherData.cpp
    src/WeatherAPI.cpp
    src/FavoriteCities.cpp
    ${IMGUI_SOURCES}
)

# קישור לספריות
target_link_libraries(WeatherApp
    ${OPENGL_LIBRARIES}
    glfw3
)

# הוספת תיקיות Include
target_include_directories(WeatherApp PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${OPENGL_INCLUDE_DIR}
)
