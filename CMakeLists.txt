cmake_minimum_required(VERSION 3.30)
project(CryptoToysPP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

find_package(wxWidgets 3.3 CONFIG REQUIRED COMPONENTS
        core
        base
        webview
)

find_package(nlohmann_json 3.12 REQUIRED)

find_package(spdlog 1.15 REQUIRED)

add_executable(${PROJECT_NAME}
        src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
        ${wxWidgets_LIBRARIES}
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)

