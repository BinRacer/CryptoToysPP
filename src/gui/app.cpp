/* clang-format off */
/*
 * @file app.cpp
 * @date 2025-07-22
 * @license MIT License
 *
 * Copyright (c) 2025 BinRacer <native.lab@outlook.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* clang-format on */
#include "app.h"
#include "frame.h"
#include <fstream>
#include <map>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace CryptoToysPP::Gui {
    // Helper function to get user home directory
    wxString GetUserConfigPath() {
        wxString path = wxGetHomeDir();

        // Append .CryptoToysPP directory
        wxString sep = wxFileName::GetPathSeparator();
        path += sep + ".CryptoToysPP";
        return path;
    }

    // Helper function to convert string to spdlog level using C++20 views
    spdlog::level::level_enum string_to_level(std::string_view level_str_view) {
        // Create a view with no whitespace and lowercase characters
        auto transformed = level_str_view | std::views::filter([](char c) {
                               return !std::isspace(c);
                           }) |
                std::views::transform([](char c) {
                               return std::tolower(c);
                           });

        // Convert view to string for matching
        std::string level_str(transformed.begin(), transformed.end());

        // Determine log level using string comparisons
        if (level_str == "trace")
            return spdlog::level::trace;
        if (level_str == "debug")
            return spdlog::level::debug;
        if (level_str == "info")
            return spdlog::level::info;
        if (level_str == "warn")
            return spdlog::level::warn;
        if (level_str == "err")
            return spdlog::level::err;
        if (level_str == "critical")
            return spdlog::level::critical;
        return spdlog::level::info; // Default level
    }

    bool App::OnInit() {
        // Get user-specific config base directory
        const wxString baseDir = GetUserConfigPath();

        // Ensure base directory exists
        if (!wxDirExists(baseDir) && !wxMkdir(baseDir)) {
            wxLogError("Failed to create base directory: %s", baseDir);
            return false;
        }

        // Setup config paths
        wxString sep = wxFileName::GetPathSeparator();
        const wxString configDir = baseDir + sep + "config";
        const wxString configPath = configDir + sep + "log_config.ini";

        // Setup log paths
        const wxString logDir = baseDir + sep + "logs";
        const wxString logPath = logDir + sep + "app.log";

        // Ensure config directory exists
        if (!wxDirExists(configDir)) {
            if (!wxMkdir(configDir)) {
                wxLogError("Failed to create config directory: %s", configDir);
            }
        }

        // Ensure log directory exists (critical for application)
        if (!wxDirExists(logDir)) {
            if (!wxMkdir(logDir)) {
                wxLogError("Failed to create log directory: %s", logDir);
                return false;
            }
        }

        // Create default config file if it doesn't exist
        if (!wxFileExists(configPath)) {
            wxFile configFile;
            if (configFile.Create(configPath, true) && configFile.IsOpened()) {
                const std::string defaultConfig = "log_level=info\n";
                configFile.Write(defaultConfig.c_str(), defaultConfig.size());
                configFile.Close();
                wxLogMessage(
                        "Created default config file with log_level=info: %s",
                        configPath);
            } else {
                wxLogError("Failed to create config file: %s", configPath);
            }
        }

        // Read log level from configuration file with C++20 features
        spdlog::level::level_enum log_level =
                spdlog::level::info; // Default level
        try {
            std::ifstream config_file(configPath.ToStdString());
            if (config_file.is_open()) {
                std::string line;
                while (std::getline(config_file, line)) {
                    // Use string_view to avoid unnecessary memory allocation
                    std::string_view line_view = line;

                    // Find key-value separator
                    size_t pos = line_view.find('=');
                    if (pos != std::string_view::npos &&
                        line_view.substr(0, pos) == "log_level") {

                        // Directly use substring view
                        std::string_view level_str_view =
                                line_view.substr(pos + 1);

                        // Convert using C++20 string processing
                        log_level = string_to_level(level_str_view);
                        break;
                    }
                }
                config_file.close();
                spdlog::info("Loaded log configuration from: {}",
                             configPath.ToStdString());
            } else {
                spdlog::warn("Configuration file could not be opened. Using "
                             "default log settings.");
            }
        } catch (const std::exception &e) {
            wxLogError("Error reading config file: {}", e.what());
        }

        try {
            // Initialize logging system with file and console sinks
            auto file_sink =
                    std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                            logPath.ToStdString(), 1024 * 1024 * 100,
                            10); // 100MB per file, 10 files max
            auto console_sink =
                    std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            std::vector<spdlog::sink_ptr> sinks{file_sink, console_sink};
            auto logger = std::make_shared<spdlog::logger>("main_logger",
                                                           sinks.begin(),
                                                           sinks.end());

            // Configure log format and level
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
            logger->set_level(log_level);
            spdlog::set_default_logger(logger);

            // Configure flush policy
            spdlog::flush_on(
                    spdlog::level::warn); // Immediate flush for warning+ levels
            spdlog::flush_every(std::chrono::seconds(5)); // Periodic flush

            spdlog::info("Logging system initialized successfully");
            spdlog::debug("Log file path: {}", logPath.ToStdString());
            spdlog::debug("Current log level: {}",
                          spdlog::level::to_string_view(log_level));

        } catch (const spdlog::spdlog_ex &ex) {
            wxLogError("Log initialization failed: {}", ex.what());
            return false;
        }

        // Create main application window
        auto frame = new MainFrame();
        frame->Show();

        return true;
    }

    int App::OnExit() {
        spdlog::info("Application exiting...");
        spdlog::shutdown(); // Automatically flush all logs
        return wxApp::OnExit();
    }
} // namespace CryptoToysPP::Gui
