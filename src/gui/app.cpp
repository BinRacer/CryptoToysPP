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
#include <wx/filename.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace CryptoToysPP::Gui {
    bool App::OnInit() {
        // 创建日志目录
        const wxString logDir =
                wxGetCwd() + wxFileName::GetPathSeparator() + "logs";
        if (!wxDirExists(logDir)) {
            if (!wxMkdir(logDir)) {
                std::cerr << "Failed to create log directory: " << logDir
                          << std::endl;
                return false;
            }
        }

        const wxString logPath =
                logDir + wxFileName::GetPathSeparator() + "app.log";

        try {
            // 初始化日志系统
            std::shared_ptr<spdlog::sinks::sink> file_sink =
                    std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                            logPath.ToStdString(), 1024 * 1024 * 100, 10);

            const auto console_sink =
                    std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            std::vector<spdlog::sink_ptr> sinks{file_sink, console_sink};
            auto logger = std::make_shared<spdlog::logger>("main_logger",
                                                           sinks.begin(),
                                                           sinks.end());

            // 配置日志格式
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
            logger->set_level(spdlog::level::debug);
            spdlog::set_default_logger(logger);

            // 添加日志刷新配置
            spdlog::flush_on(spdlog::level::info); // 重要级别日志立即刷新
            spdlog::flush_every(std::chrono::seconds(5)); // 每5秒自动刷新

            spdlog::info("Logging system initialized successfully");
            spdlog::debug("Log file path: {}", logPath.ToStdString());
            spdlog::debug("Log flush policy: immediate for INFO+ level, every "
                          "5 seconds");

        } catch (const spdlog::spdlog_ex &ex) {
            std::cerr << "Log initialization failed: " << ex.what()
                      << std::endl;
            return false;
        }

// macOS 特定配置
#ifdef __WXOSX__
        wxWebView::SetBackend(wxWebViewBackendWebKit);
        spdlog::debug("macOS: Using WebKit backend");
#endif

        // 创建主框架
        return new MainFrame();
    }

    int App::OnExit() {
        spdlog::info("Application exiting...");
        spdlog::shutdown(); // 这会自动刷新所有日志
        return wxApp::OnExit();
    }
} // namespace CryptoToysPP::Gui
