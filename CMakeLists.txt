# =====================================================================================
# CryptoToysPP CMake Configuration
#   - C++20 support
#   - Cross-platform dependency management
#   - Custom runtime dependency handling
#   - Pre-build resource generation
# =====================================================================================
cmake_minimum_required(VERSION 3.30)
project(CryptoToysPP)

# C++20 standard requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ─── Vcpkg Dependency Management ───────────────────────────────────────────────
set(USING_VCPKG OFF CACHE BOOL "Enable Vcpkg dependency management")
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
    set(USING_VCPKG ON)

    # Auto-detect target platform
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

# ─── Core Dependencies ─────────────────────────────────────────────────────────
find_package(nlohmann_json 3.12 CONFIG REQUIRED)
find_package(cryptopp 8.9 CONFIG REQUIRED)
find_package(spdlog 1.15 CONFIG REQUIRED)

# ─── Python Build Helpers ──────────────────────────────────────────────────────
find_package(Python COMPONENTS Interpreter REQUIRED)

# ─── Resource Generation (Pre-Build) ───────────────────────────────────────────
set(GENERATE_RESOURCES_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/generate_resources.py")

# Verify script existence
if(NOT EXISTS ${GENERATE_RESOURCES_SCRIPT})
    message(FATAL_ERROR "Resource generation script not found: ${GENERATE_RESOURCES_SCRIPT}")
endif()

# Configure resource output directory
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/src/resources")
set(RESOURCE_HEADER "${RESOURCES_DIR}/resources.h")
set(RESOURCE_SOURCE "${RESOURCES_DIR}/resources.cpp")

# Add pre-build custom command for resource generation
add_custom_command(
        OUTPUT ${RESOURCE_HEADER} ${RESOURCE_SOURCE}
        COMMAND ${Python_EXECUTABLE} ${GENERATE_RESOURCES_SCRIPT}
        DEPENDS ${GENERATE_RESOURCES_SCRIPT}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating resource files: ${RESOURCE_HEADER} and ${RESOURCE_SOURCE}"
        VERBATIM
)

# Create virtual target to ensure resource generation
add_custom_target(generate_resources ALL
        DEPENDS ${RESOURCE_HEADER} ${RESOURCE_SOURCE}
)

# ─── wxWidgets Configuration ───────────────────────────────────────────────────
set(wxBUILD_SHARED ON CACHE BOOL "Shared library build")
set(wxBUILD_MONOLITHIC OFF CACHE BOOL "Monolithic build disabled")

find_package(wxWidgets 3.3 COMPONENTS webview QUIET)
if(wxWidgets_FOUND)
    message(STATUS "Using system-installed wxWidgets ${wxWidgets_VERSION}")
    set(WXWEBVIEW_TARGET wx::webview)

    # Determine library directory (Windows/macOS)
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
    message(STATUS "Building wxWidgets from source")
    include(FetchContent)

    set(wxBUILD_OPTIONS
            "--disable-all"     # Minimal components
            "--enable-webview"  # WebView support
            "--enable-unicode"  # Unicode encoding
    )

    # Platform-specific build options
    if(WIN32)
        list(APPEND wxBUILD_OPTIONS "--with-msw")
    elseif(APPLE)
        list(APPEND wxBUILD_OPTIONS "--with-osx_cocoa")
    else()
        list(APPEND wxBUILD_OPTIONS "--with-gtk=3")
    endif()

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
    set(wx_runtime_dir "$<TARGET_FILE_DIR:wx::core>")
endif()

# ─── Runtime Directory Handling ────────────────────────────────────────────────
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

# ─── Source Collection ─────────────────────────────────────────────────────────
include_directories(${CMAKE_SOURCE_DIR}/src ${RESOURCES_DIR})

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

# ─── Windows Application Icon ────────────────────────────────────────────────
if(WIN32)
    # Verify icon resource existence
    set(APP_ICON_RC "${CMAKE_SOURCE_DIR}/icons/CryptoToysPP.windows.rc")
    set(APP_ICO_FILE "${CMAKE_SOURCE_DIR}/icons/CryptoToysPP.windows.ico")

    if(EXISTS ${APP_ICON_RC} AND EXISTS ${APP_ICO_FILE})
        # Add resource file to source collection
        list(APPEND SOURCE_FILES ${APP_ICON_RC})
        message(STATUS "Added Windows icon resource file: ${APP_ICON_RC}")

        # Copy icon file to build directory (required for resource compilation)
        configure_file(${APP_ICO_FILE} ${CMAKE_CURRENT_BINARY_DIR}/CryptoToysPP.windows.ico COPYONLY)
    else()
        message(WARNING "Missing Windows icon files in icons directory")
    endif()
endif()

# Add generated resources to source collection
list(APPEND SOURCE_FILES
        ${RESOURCE_HEADER}
        ${RESOURCE_SOURCE}
)

# ─── Executable Target ─────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Ensure resource generation completes before build
add_dependencies(${PROJECT_NAME} generate_resources)

# Windows-specific configuration
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
            LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()

# ─── Dependency Linking ────────────────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME}
        nlohmann_json::nlohmann_json
        cryptopp::cryptopp
        spdlog::spdlog
        ${WXWEBVIEW_TARGET}
)

# ─── Runtime File Copying ──────────────────────────────────────────────────────
if(WIN32 AND wxBUILD_SHARED)
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

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMENT "Copying wxWidgets runtime files"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${wx_runtime_dir}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            VERBATIM
    )
endif()

# ─── Build Output Copying ──────────────────────────────────────────────────────
# Identify current build configuration
if(CMAKE_BUILD_TYPE)
    set(CURRENT_BUILD_TYPE ${CMAKE_BUILD_TYPE})
else()
    set(CURRENT_BUILD_TYPE $<CONFIG>)
endif()

set(COPY_SCRIPT_PATH "${CMAKE_SOURCE_DIR}/scripts/copy_build_output.py")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMENT "Copying build output to project directory"
        COMMAND "${Python_EXECUTABLE}" "${COPY_SCRIPT_PATH}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        "${CURRENT_BUILD_TYPE}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        VERBATIM
)

# ─── Installation Target ───────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        BUNDLE DESTINATION .
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
