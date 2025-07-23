cmake_minimum_required(VERSION 3.30)
project(CryptoToysPP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

find_package(wxWidgets 3.3 CONFIG REQUIRED COMPONENTS
        webview
)
find_package(nlohmann_json 3.12 REQUIRED)
find_package(cryptopp 8.9 REQUIRED)
find_package(spdlog 1.15 REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/resources/resources.h
        src/resources/resources.cpp
        src/route/handler.cpp
        src/route/handler.h
        src/route/route.cpp
        src/route/route.h
        src/gui/frame.cpp
        src/gui/frame.h
        src/gui/app.cpp
        src/gui/app.h
        src/base/base64.cpp
        src/base/base64.h
)

target_link_libraries(${PROJECT_NAME}
        ${wxWidgets_LIBRARIES}
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        cryptopp
)

