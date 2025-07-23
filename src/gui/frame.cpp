/* clang-format off */
/*
 * @file frame.cpp
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
#include "frame.h"
#include "route/handler.h"
#include "base/base64.h"
#include <spdlog/spdlog.h>
namespace CryptoToysPP::Gui {
    MainFrame::MainFrame() :
        wxFrame(nullptr, wxID_ANY, "wxWebView + Optimized Resource Protocol") {
        InitWebView();
        spdlog::debug("MainFrame 初始化完成");
    }

    void MainFrame::InitWebView() {
        spdlog::debug("开始初始化 WebView");
        // 选择最佳的后端
        wxString backend = wxWebViewBackendDefault;
        if (wxWebView::IsBackendAvailable(wxWebViewBackendEdge)) {
            backend = wxWebViewBackendEdge;
            spdlog::debug("使用 Edge WebView 后端");
        } else if (wxWebView::IsBackendAvailable(wxWebViewBackendWebKit)) {
            backend = wxWebViewBackendWebKit;
            spdlog::debug("使用 WebKit WebView 后端");
        } else {
            spdlog::debug("使用默认 WebView 后端");
        }

        // 创建 WebView 组件
        webview =
                wxWebView::New(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                               wxSize(800, 600), backend, wxBORDER_NONE);

        if (!webview) {
            spdlog::error("无法创建 WebView 组件，请检查配置");
            Close(true);
            return;
        }
        spdlog::debug("WebView 创建成功");

        // 注册自定义协议处理器
        webview->RegisterHandler(
                wxSharedPtr<wxWebViewHandler>(new Route::SchemeHandler()));
        spdlog::debug("注册自定义协议处理器");

        // 设置用户代理
        webview->SetUserAgent(
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                "(KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36");
        spdlog::debug("设置 UserAgent");

        // 绑定事件处理函数
        webview->Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED,
                      &MainFrame::OnScriptMessage, this, webview->GetId());
        webview->AddScriptMessageHandler("CryptoToysPP");
        spdlog::debug("添加脚本消息处理器: CryptoToysPP");

        webview->Bind(wxEVT_WEBVIEW_ERROR, &MainFrame::OnWebViewError, this);
        webview->Bind(wxEVT_WEBVIEW_LOADED, &MainFrame::OnWebViewLoaded, this);

        // 加载初始页面
        spdlog::debug("加载应用首页");
        webview->LoadURL("app://index.html");

        // 启用开发者工具
        webview->EnableContextMenu(true);
        webview->EnableAccessToDevTools(true);
        spdlog::debug("启用上下文菜单和开发者工具");

        // 设置窗口属性
        SetSize(800, 600);
        Center();
        Show();
        spdlog::debug("主窗口显示完成");
    }

    void MainFrame::OnScriptMessage(wxWebViewEvent &evt) {
        spdlog::debug("收到 JS 消息");
        const wxString &handler = evt.GetMessageHandler();
        if (handler == "CryptoToysPP") {
            spdlog::debug("开始处理 CryptoToysPP 消息");
            try {
                // 获取并解码请求数据
                std::string base64Payload = evt.GetString().ToStdString();
                spdlog::info("Base64 原始请求: {}", base64Payload);
                std::string requestJson = Base::Base64::Decode(base64Payload);
                spdlog::info("Json 原始请求: {}", requestJson);

                std::string requestId;
                nlohmann::json request;
                // 提取请求ID
                try {
                    request = nlohmann::json::parse(requestJson);
                    requestId = request.value("__id", "");
                    spdlog::info("提取请求ID: {}", requestId);
                } catch (const std::exception &e) {
                    spdlog::error("解析请求数据异常: {}", e.what());
                    ErrResp(requestId, e.what());
                    return;
                } catch (...) {
                    spdlog::error("解析请求数据未知异常");
                    ErrResp(requestId, "解析请求数据未知异常");
                    return;
                }

                // 处理请求
                try {
                    spdlog::info("开始处理请求: {}", requestId);
                    nlohmann::json response = route.ProcessRequest(request);
                    spdlog::info("完成处理请求: {}", requestId);
                    OkResp(requestId, response);
                } catch (const std::exception &e) {
                    spdlog::error("请求处理异常: {} - {}", requestId, e.what());
                    ErrResp(requestId, e.what());
                }
            } catch (const std::exception &e) {
                spdlog::error("请求解析异常: {}", e.what());
                ErrResp("", e.what());
            }
        }
    }

    // 发送正常响应
    void MainFrame::OkResp(const std::string &requestId,
                           const nlohmann::json &response) {
        spdlog::info("开始发送正常响应, 请求ID: {}", requestId);
        std::string responseJson = response.dump();
        spdlog::info("JSon 原始响应内容: {}", responseJson);
        std::string base64Response = Base::Base64::Encode(responseJson);
        spdlog::info("Base64 加密响应内容: {}", base64Response);
        wxString script =
                wxString::Format("window.rest.resolveInvoke('%s', '%s');",
                                 base64Response, requestId);
        webview->RunScriptAsync(script);
        spdlog::info("完成发送正常响应, 请求ID: {}", requestId);
    }

    // 发送错误响应
    void MainFrame::ErrResp(const std::string &requestId,
                            const std::string &message) {
        spdlog::error("开始发送错误响应, 请求ID: {}", requestId);
        nlohmann::json response = {{"code", 500},
                                   {"message", message},
                                   {"data", nlohmann::json::object()}};
        std::string responseJson = response.dump();
        spdlog::error("JSon 原始响应内容: {}", responseJson);
        std::string base64Response = Base::Base64::Decode(responseJson);
        spdlog::error("Base64 加密响应内容: {}", base64Response);
        wxString errorScript =
                wxString::Format("window.rest.rejectInvoke('%s', '%s');",
                                 base64Response, requestId);
        webview->RunScriptAsync(errorScript);
        spdlog::error("完成发送错误响应, 请求ID: {}", requestId);
    }

    void MainFrame::OnWebViewError(wxWebViewEvent &evt) {
        wxString errorStr = evt.GetString();
        spdlog::error("WebView错误: {}", errorStr.ToUTF8().data());
        spdlog::debug("错误类型: {}", evt.GetInt());
        spdlog::debug("错误URL: {}", evt.GetURL().ToUTF8().data());
    }

    void MainFrame::OnWebViewLoaded(wxWebViewEvent &evt) {
        const wxString &urlStr = evt.GetURL();
        spdlog::info("页面加载成功: {}", urlStr.ToUTF8().data());
        spdlog::debug("加载状态: {}", evt.GetInt());
    }
} // namespace CryptoToysPP::Gui
