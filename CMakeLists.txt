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

# ─── Build Configuration Handling ──────────────────────────────────────────────
# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug or Release)" FORCE)
endif()

# ─── Vcpkg Dependency Management ───────────────────────────────────────────────
set(USING_VCPKG OFF CACHE BOOL "Enable Vcpkg dependency management")
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
    set(USING_VCPKG ON)

    # Auto-detect target platform
    if(WIN32)
        set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "Vcpkg target triplet")
        message(STATUS "Using Vcpkg with Windows target triplet")
    elseif(APPLE)
        set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "Vcpkg target triplet")
        message(STATUS "Using Vcpkg with macOS target triplet")
    else()
        set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "Vcpkg target triplet")
        message(STATUS "Using Vcpkg with Linux target triplet")
    endif()
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
find_package(wxWidgets 3.3 CONFIG QUIET)

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

    # Platform-specific webview configurations
    if(WIN32)
        set(wxBUILD_OPTIONS "--with-msw")
        set(WXWEBVIEW_DEPENDENCY "")
    elseif(APPLE)
        set(wxBUILD_OPTIONS "--with-osx_cocoa")
        set(WXWEBVIEW_DEPENDENCY "")
    else()
        set(wxBUILD_OPTIONS "--with-gtk=3")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0)
        list(APPEND wxBUILD_OPTIONS "--with-webkit=system")
        set(WXWEBVIEW_DEPENDENCY webkit2gtk-4.0)
    endif()

    list(APPEND wxBUILD_OPTIONS "--disable-all" "--enable-webview" "--enable-unicode")

    if(APPLE)
        FetchContent_Declare(
                wxWidgets
                GIT_REPOSITORY "https://github.com/wxWidgets/wxWidgets.git"
                GIT_TAG "v3.3.1"
                PATCH_COMMAND
                bash -c "patch -p1 < \"${CMAKE_SOURCE_DIR}/patches/macOS/fix_memset_s.patch\""
                CMAKE_ARGS
                -DwxBUILD_SHARED=${wxBUILD_SHARED}
                -DwxBUILD_MONOLITHIC=${wxBUILD_MONOLITHIC}
                -DwxBUILD_OPTIONS="${wxBUILD_OPTIONS}"
                -DwxUSE_WEBVIEW=ON
        )
    else()
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
    endif()

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
    # Windows icon resources
    set(APP_ICON_RC "${CMAKE_SOURCE_DIR}/icons/windows/CryptoToysPP.rc")
    set(APP_ICO_FILE "${CMAKE_SOURCE_DIR}/icons/windows/CryptoToysPP.ico")

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

# ─── Platform-Specific Configuration ───────────────────────────────────────────
# Windows configuration
if(WIN32)
    # Set Windows-specific properties
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
            LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )

    # macOS configuration
elseif(APPLE)
    # macOS App Bundle configuration
    set(MACOSX_BUNDLE_BUNDLE_NAME "CryptoToysPP")
    set(MACOSX_BUNDLE_ICON_FILE "CryptoToysPP.icns")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.BinRacer.${PROJECT_NAME}")
    set(MACOSX_BUNDLE_INFO_STRING "CryptoToysPP")

    # Configure Info.plist template
    configure_file(
            "${CMAKE_SOURCE_DIR}/icons/macOS/Info.plist.in"
            "${CMAKE_BINARY_DIR}/Info.plist"
    )

    # Set macOS-specific properties
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_BINARY_DIR}/Info.plist"
            RESOURCE "${CMAKE_SOURCE_DIR}/icons/macOS/CryptoToysPP.icns"
    )

    # macOS icons validation and configuration
    set(APP_ICON_PATH "${CMAKE_SOURCE_DIR}/icons/macOS/CryptoToysPP.icns")
    if(NOT EXISTS ${APP_ICON_PATH})
        message(WARNING "macOS app icon not found at: ${APP_ICON_PATH}")
    else()
        # Ensure icon is copied to bundle resources with proper name
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMENT "Copying macOS icon to bundle"
                COMMAND ${CMAKE_COMMAND} -E copy
                "${APP_ICON_PATH}"
                "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Resources/${MACOSX_BUNDLE_ICON_FILE}"
        )
    endif()

    # macOS RPATH configuration
    set(ADJUST_RPATH_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/macOS/adjust_exe_rpath.sh")
    if(EXISTS ${ADJUST_RPATH_SCRIPT})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMENT "Adjusting macOS executable RPATH"
                COMMAND bash ${ADJUST_RPATH_SCRIPT}
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>"
                VERBATIM
        )
    else()
        message(WARNING "RPATH adjustment script not found at: ${ADJUST_RPATH_SCRIPT}")
    endif()

    # Linux configuration
else()
    # Add Linux-specific configuration here if needed
    message(STATUS "Configuring for Linux platform")
endif()

# ─── Dependency Linking ───────────────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME}
        nlohmann_json::nlohmann_json
        cryptopp::cryptopp
        spdlog::spdlog
        ${WXWEBVIEW_TARGET}
)

# ─── Static Library Detection Function ────────────────────────────────────────
function(is_static_library target result_var)
    get_target_property(type ${target} TYPE)
    if(type STREQUAL "STATIC_LIBRARY")
        set(${result_var} TRUE PARENT_SCOPE)
    else()
        set(${result_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

# ─── Runtime Library Handling ─────────────────────────────────────────────────
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

    # Copy dynamic libraries and modules (skip static libraries)
    foreach(target IN LISTS RUNTIME_TARGETS)
        is_static_library(${target} is_static)
        if(NOT is_static)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMENT "Copying ${target} runtime files"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:${target}>"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            )

            # Handle macOS framework dependencies
            if(APPLE)
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
        endif()
    endforeach()

    # Handle wxWidgets runtime directories
    if(DEFINED wx_runtime_dir)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMENT "Copying wxWidgets runtime files"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${wx_runtime_dir}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )
    endif()

    # macOS bundle cleaning
    if(APPLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMENT "Cleaning macOS bundle artifacts"
                COMMAND find "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/MacOS"
                -name "*.a" -delete
                COMMAND rm -f "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/MacOS/wxWidgets*.cmake"
                COMMAND rm -rf "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/MacOS/wx"
        )
    endif()
endif()

# ─── Build Output Handling ────────────────────────────────────────────────────
set(COPY_SCRIPT_PATH "${CMAKE_SOURCE_DIR}/scripts/copy_build_output.py")

if(EXISTS ${COPY_SCRIPT_PATH})
    # Determine the correct source directory for copying
    if(APPLE)
        set(COPY_SOURCE_DIR "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>")
    else()
        set(COPY_SOURCE_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMENT "Copying build output to dist directory"
            COMMAND "${Python_EXECUTABLE}" "${COPY_SCRIPT_PATH}"
            "${COPY_SOURCE_DIR}"
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
