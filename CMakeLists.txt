cmake_minimum_required(VERSION 3.25)
project(tvpaint-rpc C CXX)

# Set C++ standard version requirement
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Output build/compile_commands.json for clangd support
option(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Microsoft Visual C++
if(MSVC)
    # Export symbols from a .dll that are not in any of its object files but are added by the linker from dependencies
    # See: https://cmake.org/cmake/help/latest/prop_tgt/WINDOWS_EXPORT_ALL_SYMBOLS.html
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Use static boost libraries
set(Boost_USE_STATIC_LIBS ON)

# Find dependencies
find_package(TVPaintAnimationSDK REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(websocketpp REQUIRED)
find_package(spdlog REQUIRED)

# Search for source files and configure library
file(GLOB SOURCES "src/*.cpp" "src/*.hpp")
add_library(${PROJECT_NAME} SHARED ${SOURCES} library.def)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE TVPaintAnimationSDK::TVPaintAnimationSDK)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE websocketpp::websocketpp)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

# Package the plugin into the TVPaint .plugin bundle structure
create_plugin_bundle(${PROJECT_NAME})
