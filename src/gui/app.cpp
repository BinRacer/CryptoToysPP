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
        const wxString logDir =
                wxGetCwd() + wxFileName::GetPathSeparator() + "logs";
        if (!wxDirExists(logDir))
            wxMkdir(logDir);

        const wxString logPath =
                logDir + wxFileName::GetPathSeparator() + "app.log";
        try {
            const auto file_sink =
                    std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                            logPath.ToStdString(), 1024 * 1024 * 5, 3);
            const auto console_sink =
                    std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            std::vector<spdlog::sink_ptr> sinks{file_sink, console_sink};
            const auto logger = std::make_shared<spdlog::logger>("main_logger",
                                                                 sinks.begin(),
                                                                 sinks.end());
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
            logger->set_level(spdlog::level::trace);
            spdlog::set_default_logger(logger);
            spdlog::debug("日志系统初始化成功");
        } catch (const spdlog::spdlog_ex &ex) {
            std::cerr << "日志初始化失败: " << ex.what() << std::endl;
            return false;
        }

#ifdef __WXOSX__
        wxWebView::SetBackend(wxWebViewBackendWebKit);
#endif

        return new MainFrame();
    }

    int App::OnExit() {
        spdlog::shutdown();
        return wxApp::OnExit();
    }
} // namespace CryptoToysPP::Gui
