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
#include <spdlog/spdlog.h>
#include <wx/base64.h>
using json = nlohmann::json;
namespace CryptoToysPP::Gui {
    MainFrame::MainFrame() :
        wxFrame(nullptr, wxID_ANY, "wxWebView + Optimized Resource Protocol") {

        spdlog::info("MainFrame 构造函数开始");
        InitWebView();
        spdlog::info("MainFrame 初始化完成");
    }

    void MainFrame::InitWebView() {
        spdlog::info("初始化 WebView");

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

        webview =
                wxWebView::New(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                               wxSize(800, 600), backend, wxBORDER_NONE);

        if (!webview) {
            spdlog::error("无法创建 WebView 组件，请检查配置");
            Close(true);
            return;
        }
        spdlog::debug("WebView 创建成功");

        webview->RegisterHandler(
                wxSharedPtr<wxWebViewHandler>(new Route::SchemeHandler()));
        spdlog::debug("注册自定义协议处理器");

        webview->SetUserAgent(
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                "(KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36");
        spdlog::debug("设置 UserAgent");

        webview->Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED,
                      &MainFrame::OnScriptMessage, this, webview->GetId());
        webview->AddScriptMessageHandler("CryptoToysPP");
        spdlog::debug("添加脚本消息处理器: CryptoToysPP");

        webview->Bind(wxEVT_WEBVIEW_ERROR, &MainFrame::OnWebViewError, this);
        webview->Bind(wxEVT_WEBVIEW_LOADED, &MainFrame::OnWebViewLoaded, this);

        spdlog::debug("加载 URL: app://index.html");
        webview->LoadURL("app://index.html");

        webview->EnableContextMenu(true);
        webview->EnableAccessToDevTools(true);
        spdlog::debug("启用上下文菜单和开发者工具");

        SetSize(800, 600);
        Center();
        Show();
        spdlog::info("主窗口显示完成");
    }

    void MainFrame::OnScriptMessage(wxWebViewEvent &evt) {
        spdlog::debug("收到脚本消息");

        wxString handler = evt.GetMessageHandler();
        if (handler == "CryptoToysPP") {
            spdlog::debug("处理 CryptoToysPP 消息");

            try {
                // 获取Base64编码的请求数据
                wxString base64Payload = evt.GetString();

                // 关键修复：正确解码Base64数据
                wxMemoryBuffer buffer = wxBase64Decode(base64Payload);
                wxString decodedPayload =
                        wxString::FromUTF8(static_cast<const char *>(
                                                   buffer.GetData()),
                                           buffer.GetDataLen());

                std::string requestJson = decodedPayload.ToUTF8().data();
                spdlog::trace("原始请求 JSON: {}", requestJson);

                std::string requestId = "";

                // 提取请求ID
                try {
                    json request = json::parse(requestJson);
                    requestId = request.value("__id", "");
                    spdlog::debug("提取请求 ID: {}", requestId);
                    spdlog::trace("请求数据: {}", request.dump());
                } catch (...) {
                    spdlog::warn("无法提取请求 ID");
                }

                // 直接处理请求
                spdlog::debug("直接处理请求 ID: {}", requestId);

                try {
                    json request = json::parse(requestJson);
                    spdlog::trace("解析请求 JSON 成功");

                    spdlog::debug("处理请求路径: {}",
                                  request.value("path", ""));
                    json response = route.ProcessRequest(request);
                    spdlog::debug("请求处理完成");

                    // 将响应转换为Base64编码
                    std::string responseJson = response.dump();

                    // 关键修复：使用 wxMemoryBuffer 包装数据
                    wxMemoryBuffer buffer;
                    buffer.AppendData(responseJson.data(), responseJson.size());
                    wxString base64Response = wxBase64Encode(buffer);

                    // 发送响应 - 使用Base64编码
                    wxString script = wxString::Format(
                            "window.rest.resolveInvoke('%s', '%s');",
                            base64Response, requestId);

                    spdlog::trace("执行脚本: {}", script.ToStdString());
                    webview->RunScriptAsync(script);
                    spdlog::debug("响应脚本已执行");
                } catch (const std::exception &e) {
                    spdlog::error("请求处理异常: {}", e.what());

                    json errorResponse = {{"code", 500},
                                          {"message", e.what()},
                                          {"data", json::object()}};

                    // 将错误响应转换为Base64编码
                    std::string errorJson = errorResponse.dump();

                    // 关键修复：使用 wxMemoryBuffer 包装数据
                    wxMemoryBuffer errorBuffer;
                    errorBuffer.AppendData(errorJson.data(), errorJson.size());
                    wxString base64Error = wxBase64Encode(errorBuffer);

                    // 发送错误响应 - 使用Base64编码
                    wxString errorScript = wxString::Format(
                            "window.rest.rejectInvoke('%s', '%s');",
                            base64Error, requestId);

                    spdlog::trace("执行错误脚本: {}",
                                  errorScript.ToStdString());
                    webview->RunScriptAsync(errorScript);
                    spdlog::debug("错误响应脚本已执行");
                }
            } catch (const std::exception &e) {
                spdlog::error("请求解析异常: {}", e.what());

                json errorResponse = {{"code", 500},
                                      {"message", e.what()},
                                      {"data", json::object()}};

                std::string requestId = "";
                try {
                    wxString base64Payload = evt.GetString();
                    wxMemoryBuffer buffer = wxBase64Decode(base64Payload);
                    wxString decodedPayload =
                            wxString::FromUTF8(static_cast<const char *>(
                                                       buffer.GetData()),
                                               buffer.GetDataLen());
                    std::string requestJson = decodedPayload.ToUTF8().data();
                    json request = json::parse(requestJson);
                    requestId = request.value("__id", "");
                } catch (...) {
                    spdlog::warn("无法提取请求 ID");
                }

                spdlog::debug("直接处理错误响应 ID: {}", requestId);

                // 将错误响应转换为Base64编码
                std::string errorJson = errorResponse.dump();

                // 关键修复：使用 wxMemoryBuffer 包装数据
                wxMemoryBuffer errorBuffer;
                errorBuffer.AppendData(errorJson.data(), errorJson.size());
                wxString base64Error = wxBase64Encode(errorBuffer);

                wxString errorScript = wxString::Format(
                        "window.rest.rejectInvoke('%s', '%s');", base64Error,
                        requestId);

                spdlog::trace("执行错误脚本: {}", errorScript.ToStdString());
                webview->RunScriptAsync(errorScript);
                spdlog::debug("错误响应脚本已执行");
            }
        }
    }

    void MainFrame::OnWebViewError(wxWebViewEvent &evt) {
        wxString errorStr = evt.GetString();
        spdlog::error("WebView错误: {}", errorStr.ToUTF8().data());
        spdlog::debug("错误类型: {}", evt.GetInt());
        spdlog::debug("错误URL: {}", evt.GetURL().ToUTF8().data());
    }

    void MainFrame::OnWebViewLoaded(wxWebViewEvent &evt) {
        wxString urlStr = evt.GetURL();
        spdlog::info("HTML页面加载成功: {}", urlStr.ToUTF8().data());
        spdlog::debug("加载状态: {}", evt.GetInt());
    }
} // namespace CryptoToysPP::Gui
