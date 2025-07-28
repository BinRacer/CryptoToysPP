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

if(WIN32)
    # 生成manifest文件
    file(WRITE "${CMAKE_BINARY_DIR}/app.manifest"
            [[<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <dependency>
    <dependentAssembly>
      <assemblyIdentity
        type="win32"
        name="Microsoft.Windows.Common-Controls"
        version="6.0.0.0"
        processorArchitecture="*"
        publicKeyToken="6595b64144ccf1df"
      />
    </dependentAssembly>
  </dependency>
  <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
    <security>
      <requestedPrivileges>
        <requestedExecutionLevel level="asInvoker"></requestedExecutionLevel>
      </requestedPrivileges>
    </security>
  </trustInfo>
  <compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1">
    <application>
      <supportedOS Id="{e2011457-1546-43c5-a5fe-008deee3d3f0}"></supportedOS>
      <supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}"></supportedOS>
      <supportedOS Id="{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}"></supportedOS>
      <supportedOS Id="{1f676c76-80e1-4239-95bb-83d0f6d0da78}"></supportedOS>
      <supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"></supportedOS>
    </application>
  </compatibility>
</assembly>]])

    # 创建资源描述
    file(WRITE "${CMAKE_BINARY_DIR}/manifest.rc" "1 24 \"app.manifest\"\n")

    # 嵌入资源
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/manifest.rc")
endif()

target_link_libraries(${PROJECT_NAME}
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        cryptopp
        ${wxWidgets_LIBRARIES}
)

