cmake_minimum_required(VERSION 3.30)
project(CryptoToysPP)

# 设置 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# 跨平台 Vcpkg 配置
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

# wxWidgets配置 - 优先使用系统安装版本
set(wxBUILD_SHARED ON CACHE BOOL "Build as shared libraries")
set(wxBUILD_MONOLITHIC OFF CACHE BOOL "Disable monolithic build")

find_package(wxWidgets 3.3 COMPONENTS webview QUIET)
if(wxWidgets_FOUND)
    message(STATUS "Using system-installed wxWidgets ${wxWidgets_VERSION}")
    set(WXWEBVIEW_TARGET wx::webview)

    # 获取系统安装的wxWidgets库目录(Win/macOS专用)
    if(WIN32 OR APPLE)
        get_target_property(_wx_lib_dir wx::base LIBRARY_OUTPUT_DIRECTORY)
        if(_wx_lib_dir AND EXISTS "${_wx_lib_dir}")
            set(wx_runtime_dir "${_wx_lib_dir}")
            message(STATUS "Detected wxWidgets library dir: ${wx_runtime_dir}")
        else()
            message(STATUS "Using default wxWidgets runtime location")
            set(wx_runtime_dir "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
        endif()
    endif()
else()
    message(STATUS "wxWidgets 3.3 not found, building from source")
    include(FetchContent)

    # 构建选项配置
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

    # 获取wxWidgets源码
    FetchContent_Declare(
            wxWidgets
            GIT_REPOSITORY "https://github.com/wxWidgets/wxWidgets.git"
            GIT_TAG "v3.3.1"
            CMAKE_ARGS
            -DwxBUILD_SHARED=${wxBUILD_SHARED}
            -DwxBUILD_MONOLITHIC=${wxBUILD_MONOLITHIC}
            -DwxBUILD_OPTIONS="${wxBUILD_OPTIONS}"
    )

    FetchContent_MakeAvailable(wxWidgets)
    set(WXWEBVIEW_TARGET wx::webview)

    # 设置自构建wxWidgets运行时目录
    set(wx_runtime_dir "$<TARGET_FILE_DIR:wx::core>")
endif()

# 改进的运行时目录处理
if(NOT DEFINED wx_runtime_dir)
    if(WIN32)
        set(wx_runtime_dir "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    elseif(APPLE)
        set(wx_runtime_dir "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Frameworks")
        set(CMAKE_MACOSX_RPATH ON)
    else()
        set(wx_runtime_dir "${CMAKE_INSTALL_PREFIX}/lib")
    endif()
endif()

# 包含源文件目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# 源文件收集
message(STATUS "Collecting source files from: ${CMAKE_SOURCE_DIR}/src")
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

# 链接所有依赖项
target_link_libraries(${PROJECT_NAME}
        nlohmann_json::nlohmann_json
        cryptopp::cryptopp
        spdlog::spdlog
        ${WXWEBVIEW_TARGET}
)

# 改进的运行时文件复制
if(WIN32 AND wxBUILD_SHARED)
    # 只复制实际存在的库目标
    if(TARGET cryptopp::cryptopp)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMENT "Copying Crypto++ DLL"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:cryptopp::cryptopp>"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                VERBATIM
        )
    endif()

    if(TARGET spdlog::spdlogshared)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMENT "Copying spdlog DLL"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:spdlog::spdlogshared>"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                VERBATIM
        )
    endif()

    # 复制整个wxWidgets DLL目录
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMENT "Copying wxWidgets runtime directory"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${wx_runtime_dir}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            VERBATIM
    )
endif()


# 安装目标
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        BUNDLE DESTINATION .
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
