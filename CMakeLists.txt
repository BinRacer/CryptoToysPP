cmake_minimum_required(VERSION 3.30)
project(CryptoToysPP)

# 设置 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# 跨平台 Vcpkg 配置 (可选)
set(USING_VCPKG OFF)
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
    set(USING_VCPKG ON)

    # 自动检测目标平台
    if(WIN32)
        set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "Vcpkg target triplet")
    elseif(APPLE)
        set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "Vcpkg target triplet")
    else()
        set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "Vcpkg target triplet")
    endif()

    message(STATUS "Using Vcpkg for dependency management")
else()
    message(STATUS "Vcpkg not found, using system package manager")
endif()

# 查找核心依赖
find_package(nlohmann_json 3.12 CONFIG REQUIRED)
find_package(cryptopp 8.9 CONFIG REQUIRED)
find_package(spdlog 1.15 CONFIG REQUIRED)

# 启用 FetchContent
include(FetchContent)

# wxWidgets 配置 (仅包含 webview)
set(wxBUILD_SHARED ON CACHE BOOL "Build as shared libraries")
set(wxBUILD_MONOLITHIC OFF CACHE BOOL "Disable monolithic build")

set(wxBUILD_OPTIONS
        "--disable-all"     # 禁用所有可选组件
        "--enable-webview"  # 启用 WebView
        "--enable-unicode"  # Unicode 支持
)

if(WIN32)
    list(APPEND wxBUILD_OPTIONS "--with-msw")
elseif(APPLE)
    list(APPEND wxBUILD_OPTIONS "--with-osx_cocoa")
else()
    list(APPEND wxBUILD_OPTIONS "--with-gtk=3")
endif()

# 声明 wxWidgets 项目
FetchContent_Declare(
        wxWidgets
        GIT_REPOSITORY "https://github.com/wxWidgets/wxWidgets.git"
        GIT_TAG "v3.3.1"
        CMAKE_ARGS
        -DwxBUILD_SHARED=${wxBUILD_SHARED}
        -DwxBUILD_MONOLITHIC=${wxBUILD_MONOLITHIC}
        -DwxBUILD_OPTIONS="${wxBUILD_OPTIONS}"
)

# 使用 webview 单一目标
set(wxWidgets_TARGET "wx::webview")

# 启用并配置 wxWidgets
FetchContent_MakeAvailable(wxWidgets)

# 包含源文件目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# 源文件收集
file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/src/*.h"
        "${CMAKE_SOURCE_DIR}/src/*.hpp"
        "${CMAKE_SOURCE_DIR}/src/*.hxx"
        "${CMAKE_SOURCE_DIR}/src/*.hh"
        "${CMAKE_SOURCE_DIR}/src/*.c"
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.cxx"
        "${CMAKE_SOURCE_DIR}/src/*.cc"
)

# 添加可执行目标
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Windows专用配置
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
            LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()

# 仅链接 webview 组件
if(TARGET wx::webview)
    target_link_libraries(${PROJECT_NAME}
            nlohmann_json::nlohmann_json
            cryptopp::cryptopp
            spdlog::spdlog
            wx::webview
    )
else()
    message(FATAL_ERROR "wx::webview 目标未找到")
endif()

# 运行时文件处理
if(wxWidgets_ADDED AND wxBUILD_SHARED)
    set(wx_runtime_dir "$<TARGET_FILE_DIR:wx::webview>")
    if(EXISTS "${wx_runtime_dir}")
        message(STATUS "复制 wxWidgets 运行时文件从: ${wx_runtime_dir}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMENT "复制 wxWidgets 运行时文件"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${wx_runtime_dir}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                VERBATIM
        )
    else()
        message(WARNING "无法定位 wxWidgets 运行时目录: ${wx_runtime_dir}")
    endif()
endif()
