# =====================================================================================
# CryptoToysPP CMake Configuration (Optimized)
#   - Only copies executables and dependencies to dist directory
#   - Full cross-platform support
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

# ─── Resource Generation ──────────────────────────────────────────────────────
set(GENERATE_RESOURCES_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/generate_resources.py")
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/src/resources")
set(RESOURCE_HEADER "${RESOURCES_DIR}/resources.h")
set(RESOURCE_SOURCE "${RESOURCES_DIR}/resources.cpp")

if(NOT EXISTS ${GENERATE_RESOURCES_SCRIPT})
    message(FATAL_ERROR "Resource generation script not found: ${GENERATE_RESOURCES_SCRIPT}")
endif()

add_custom_command(
        OUTPUT ${RESOURCE_HEADER} ${RESOURCE_SOURCE}
        COMMAND ${Python_EXECUTABLE} ${GENERATE_RESOURCES_SCRIPT}
        DEPENDS ${GENERATE_RESOURCES_SCRIPT}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating resource files"
        VERBATIM
)

add_custom_target(generate_resources ALL
        DEPENDS ${RESOURCE_HEADER} ${RESOURCE_SOURCE}
)

# ─── wxWidgets Configuration ──────────────────────────────────────────────────
find_package(wxWidgets 3.4 CONFIG QUIET)

if(wxWidgets_FOUND AND TARGET wx::webview)
    message(STATUS "Using system-installed wxWidgets with wx::webview target")
    set(WXWEBVIEW_TARGET wx::webview)
    set(wx_runtime_dir "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
else()
    if(wxWidgets_FOUND)
        message(STATUS "wxWidgets found but missing webview component, building from source")
    else()
        message(STATUS "wxWidgets not found, building from source with webview")
    endif()

    include(FetchContent)

    # Configure build options
    set(wxBUILD_SHARED ON CACHE BOOL "Shared library build")
    set(wxBUILD_MONOLITHIC OFF CACHE BOOL "Disable monolithic build")

    set(wxBUILD_OPTIONS
            "--disable-all"
            "--enable-webview"
            "--enable-unicode"
    )

    # Platform-specific options
    if(WIN32)
        list(APPEND wxBUILD_OPTIONS "--with-msw")
    elseif(APPLE)
        list(APPEND wxBUILD_OPTIONS "--with-osx_cocoa")
    else()
        list(APPEND wxBUILD_OPTIONS "--with-gtk=3")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0)
        list(APPEND wxBUILD_OPTIONS "--with-webkit=system")
    endif()

    FetchContent_Declare(
            wxWidgets
            GIT_REPOSITORY "https://github.com/wxWidgets/wxWidgets.git"
            GIT_TAG "v3.3.1"
            CMAKE_ARGS
            -DwxBUILD_SHARED=${wxBUILD_SHARED}
            -DwxBUILD_MONOLITHIC=${wxBUILD_MONOLITHIC}
            -DwxBUILD_OPTIONS="${wxBUILD_OPTIONS}"
            -DwxUSE_WEBVIEW=ON
    )

    FetchContent_MakeAvailable(wxWidgets)

    if(TARGET wx::webview)
        set(WXWEBVIEW_TARGET wx::webview)
        set(wx_runtime_dir "$<TARGET_FILE_DIR:wx::webview>")
    else()
        message(FATAL_ERROR "Failed to build wxWebView component from source")
    endif()
endif()

# Fallback for runtime directory
if(NOT DEFINED wx_runtime_dir)
    set(wx_runtime_dir "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
endif()

# ─── Source Collection ────────────────────────────────────────────────────────
include_directories(${CMAKE_SOURCE_DIR}/src ${RESOURCES_DIR})
file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/src/*.h*"
        "${CMAKE_SOURCE_DIR}/src/*.c*"
)

# Platform-specific resources
if(WIN32)
    set(APP_ICON_RC "${CMAKE_SOURCE_DIR}/icons/CryptoToysPP.windows.rc")
    set(APP_ICO_FILE "${CMAKE_SOURCE_DIR}/icons/CryptoToysPP.windows.ico")

    if(EXISTS ${APP_ICON_RC} AND EXISTS ${APP_ICO_FILE})
        list(APPEND SOURCE_FILES ${APP_ICON_RC})
        configure_file(${APP_ICO_FILE} ${CMAKE_CURRENT_BINARY_DIR}/CryptoToysPP.windows.ico COPYONLY)
        message(STATUS "Added Windows icon resources")
    else()
        message(WARNING "Missing Windows icon files in icons directory")
    endif()
endif()

# Add generated resources
list(APPEND SOURCE_FILES ${RESOURCE_HEADER} ${RESOURCE_SOURCE})

# ─── Executable Target ────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
add_dependencies(${PROJECT_NAME} generate_resources)

# Platform-specific executable config
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
            LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()

# ─── Dependency Linking ───────────────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME}
        nlohmann_json::nlohmann_json
        cryptopp::cryptopp
        spdlog::spdlog
        ${WXWEBVIEW_TARGET}
)

# ─── Post-Build Operations ─────────────────────────────────────────────────────
# Unified runtime handling for all platforms
set(RUNTIME_TARGETS)
if(TARGET cryptopp::cryptopp)
    list(APPEND RUNTIME_TARGETS cryptopp::cryptopp)
endif()
if(TARGET spdlog::spdlogshared)
    list(APPEND RUNTIME_TARGETS spdlog::spdlogshared)
endif()
if(TARGET ${WXWEBVIEW_TARGET})
    list(APPEND RUNTIME_TARGETS ${WXWEBVIEW_TARGET})
endif()

if(RUNTIME_TARGETS OR DEFINED wx_runtime_dir)
    # Create destination directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMENT "Preparing runtime environment"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )

    # Copy all runtime libraries
    foreach(target IN LISTS RUNTIME_TARGETS)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMENT "Copying ${target} runtime files"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${target}>"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )

        # For macOS, handle framework dependencies
        if(APPLE AND TARGET ${target})
            get_target_property(target_type ${target} TYPE)
            if(target_type STREQUAL "SHARED_LIBRARY")
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                        COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change
                        "$<TARGET_SONAME_FILE:${target}>"
                        "@executable_path/$<TARGET_FILE_NAME:${target}>"
                        "$<TARGET_FILE:${PROJECT_NAME}>"
                )
            endif()
        endif()
    endforeach()

    # Handle wxWidgets runtime directories (for all platforms)
    if(DEFINED wx_runtime_dir)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMENT "Copying wxWidgets runtime files"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${wx_runtime_dir}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )
    endif()
endif()

# macOS specific bundle handling
if(APPLE)
    # Copy wxWidgets frameworks if built from source
    if(DEFINED wx_runtime_dir AND NOT wxWidgets_FOUND)
        set_source_files_properties("${wx_runtime_dir}" PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
    endif()
endif()

# Linux specific - update linker paths
if(LINUX)
    if(TARGET spdlog::spdlogshared)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMENT "Updating Linux linker paths"
                COMMAND sh -c "patchelf --set-rpath \\$ORIGIN:$ORIGIN/../lib $<TARGET_FILE:${PROJECT_NAME}>"
        )
    endif()
endif()

# Build output copying to dist directory
set(COPY_SCRIPT_PATH "${CMAKE_SOURCE_DIR}/scripts/copy_build_output.py")

if(EXISTS ${COPY_SCRIPT_PATH})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMENT "Copying build output to dist directory"
            COMMAND "${Python_EXECUTABLE}" "${COPY_SCRIPT_PATH}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            "$<IF:$<CONFIG:>,Release,$<CONFIG>>"
            VERBATIM
    )
endif()

# ─── Installation Target ──────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        BUNDLE DESTINATION .
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)