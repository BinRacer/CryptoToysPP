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
        src/algorithm/base/base16.cpp
        src/algorithm/base/base16.h
        src/algorithm/base/base32.cpp
        src/algorithm/base/base32.h
        src/algorithm/base/base58.cpp
        src/algorithm/base/base58.h
        src/algorithm/base/base62.cpp
        src/algorithm/base/base62.h
        src/algorithm/base/base64.cpp
        src/algorithm/base/base64.h
        src/algorithm/base/base85.cpp
        src/algorithm/base/base85.h
        src/algorithm/base/base91.cpp
        src/algorithm/base/base91.h
        src/algorithm/base/base92.cpp
        src/algorithm/base/base92.h
        src/algorithm/base/base100.cpp
        src/algorithm/base/base100.h
        src/algorithm/simple/uucode.cpp
        src/algorithm/simple/uucode.h
        src/algorithm/simple/xxcode.cpp
        src/algorithm/simple/xxcode.h
        src/algorithm/simple/vigenere.cpp
        src/algorithm/simple/vigenere.h
        src/algorithm/hash/hash.cpp
        src/algorithm/hash/hash.h
        src/algorithm/advance/aes.cpp
        src/algorithm/advance/aes.h
        src/algorithm/advance/rsa.cpp
        src/algorithm/advance/rsa.h
)

target_link_libraries(${PROJECT_NAME}
        ${wxWidgets_LIBRARIES}
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        cryptopp
)

