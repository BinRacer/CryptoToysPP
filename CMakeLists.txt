cmake_minimum_required(VERSION 3.30)
project(CryptoToysPP)

set(CMAKE_CXX_STANDARD 17)

find_package(wxWidgets 3.3 CONFIG REQUIRED COMPONENTS
        core
        base
        webview
)

find_package(nlohmann_json 3.12 REQUIRED)

add_executable(${PROJECT_NAME}
        src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
        ${wxWidgets_LIBRARIES}
        nlohmann_json::nlohmann_json
)

