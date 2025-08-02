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
#include <wx/filename.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace CryptoToysPP::Gui {
    // Helper function: Convert string to spdlog log level
    spdlog::level::level_enum string_to_level(const std::string &level_str) {
        static const std::map<std::string, spdlog::level::level_enum>
                level_map = {{"trace", spdlog::level::trace},
                             {"debug", spdlog::level::debug},
                             {"info", spdlog::level::info},
                             {"warn", spdlog::level::warn},
                             {"error", spdlog::level::err},
                             {"critical", spdlog::level::critical},
                             {"off", spdlog::level::off}};

        auto it = level_map.find(level_str);
        return level_map.contains(level_str)
                ? level_map.at(level_str)
                : spdlog::level::info; // Default to info
    }

    bool App::OnInit() {
        // Configuration file and log path settings
        const wxString configDir =
                wxGetCwd() + wxFileName::GetPathSeparator() + "config";
        const wxString configPath =
                configDir + wxFileName::GetPathSeparator() + "log_config.ini";

        const wxString logDir =
                wxGetCwd() + wxFileName::GetPathSeparator() + "logs";
        const wxString logPath =
                logDir + wxFileName::GetPathSeparator() + "app.log";

        // Ensure the configuration directory exists
        if (!wxDirExists(configDir)) {
            if (!wxMkdir(configDir)) {
                std::cerr << "Failed to create config directory: " << configDir
                          << std::endl;
            }
        }

        // Ensure the log directory exists
        if (!wxDirExists(logDir)) {
            if (!wxMkdir(logDir)) {
                std::cerr << "Failed to create log directory: " << logDir
                          << std::endl;
                return false;
            }
        }

        // If config file doesn't exist, create default config file
        if (!wxFileExists(configPath)) {
            wxFile configFile;
            if (configFile.Create(configPath, true) && configFile.IsOpened()) {
                const std::string defaultConfig = "log_level=info\n";
                configFile.Write(defaultConfig.c_str(), defaultConfig.size());
                configFile.Close();
                std::cout << "Created default config file with log_level=info: "
                          << configPath << std::endl;
            } else {
                std::cerr << "Failed to create config file: " << configPath
                          << std::endl;
            }
        }

        // Read log level from configuration file
        spdlog::level::level_enum log_level =
                spdlog::level::info; // Default level
        try {
            std::ifstream config_file(configPath.ToStdString());
            if (config_file.is_open()) {
                std::string line;
                while (std::getline(config_file, line)) {
                    // Simple INI format parsing: key=value
                    size_t pos = line.find('=');
                    if (pos != std::string::npos &&
                        line.substr(0, pos) == "log_level") {
                        std::string level_str = line.substr(pos + 1);
                        // Convert to lowercase and remove whitespace
                        level_str.erase(std::ranges::remove_if(level_str,
                                                               ::isspace)
                                                .begin(),
                                        level_str.end());
                        std::ranges::transform(level_str, level_str.begin(),
                                               ::tolower);

                        log_level = string_to_level(level_str);
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
            std::cerr << "Error reading config file: " << e.what() << std::endl;
        }

        try {
            // Initialize logging system
            std::shared_ptr<spdlog::sinks::sink> file_sink =
                    std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                            logPath.ToStdString(), 1024 * 1024 * 100, 10);

            const auto console_sink =
                    std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            std::vector<spdlog::sink_ptr> sinks{file_sink, console_sink};
            auto logger = std::make_shared<spdlog::logger>("main_logger",
                                                           sinks.begin(),
                                                           sinks.end());

            // Configure log format (using the level read from config file)
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
            logger->set_level(log_level);
            spdlog::set_default_logger(logger);

            // Configure log flushing policy
            spdlog::flush_on(spdlog::level::info); // Immediate flush for
                                                   // important levels
            spdlog::flush_every(
                    std::chrono::seconds(5)); // Automatic flush every 5 seconds

            spdlog::info("Logging system initialized successfully");
            spdlog::debug("Log file path: {}", logPath.ToStdString());
            spdlog::debug("Current log level: {}",
                          spdlog::level::to_string_view(log_level));
            spdlog::debug("Log flush policy: immediate for INFO+ level, every "
                          "5 seconds");

        } catch (const spdlog::spdlog_ex &ex) {
            std::cerr << "Log initialization failed: " << ex.what()
                      << std::endl;
            return false;
        }

        // Create main application frame
        return new MainFrame();
    }

    int App::OnExit() {
        spdlog::info("Application exiting...");
        spdlog::shutdown(); // Automatically flush all logs
        return wxApp::OnExit();
    }
} // namespace CryptoToysPP::Gui
