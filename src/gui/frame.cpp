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

namespace CryptoToysPP::Gui {
    MainFrame::MainFrame() :
        wxFrame(nullptr, wxID_ANY, "wxWebView + Optimized Resource Protocol") {
        InitWebView();
    }

    void MainFrame::InitWebView() {
        wxString backend = wxWebViewBackendDefault;
        if (wxWebView::IsBackendAvailable(wxWebViewBackendEdge)) {
            backend = wxWebViewBackendEdge;
        } else if (wxWebView::IsBackendAvailable(wxWebViewBackendWebKit)) {
            backend = wxWebViewBackendWebKit;
        }

        webview =
                wxWebView::New(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                               wxSize(800, 600), backend, wxBORDER_NONE);

        if (!webview) {
            spdlog::error("无法创建WebView组件，请检查配置");
            Close(true);
            return;
        }

        webview->RegisterHandler(
                wxSharedPtr<wxWebViewHandler>(new Route::SchemeHandler()));
        webview->SetUserAgent(
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                "(KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36");
        webview->Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED,
                      &MainFrame::OnScriptMessage, this, webview->GetId());
        webview->AddScriptMessageHandler("wx");
        webview->Bind(wxEVT_WEBVIEW_ERROR, &MainFrame::OnWebViewError, this);
        webview->Bind(wxEVT_WEBVIEW_LOADED, &MainFrame::OnWebViewLoaded, this);
        webview->LoadURL("app://index.html");
        webview->EnableContextMenu(true);
        webview->EnableAccessToDevTools(true);

        SetSize(800, 600);
        Center();
        Show();
    }

    void MainFrame::OnScriptMessage(wxWebViewEvent &evt) {
        wxString handler = evt.GetMessageHandler();
        if (handler == "wx") {
            try {
                std::string requestJson = evt.GetString().ToUTF8().data();
                nlohmann::json request = nlohmann::json::parse(requestJson);
                int requestId = request.value("__id", -1);
                std::string response = route.ProcessRequest(request);

                wxString script = wxString::Format(
                        "window.wxApi.resolveInvoke('%s', %d);",
                        EscapeJavascriptString(response), requestId);
                webview->RunScript(script);
            } catch (const std::exception &e) {
                nlohmann::json errorResponse = {{"code", 500},
                                                {"message", e.what()},
                                                {"data",
                                                 nlohmann::json::object()}};

                int requestId = -1;
                try {
                    nlohmann::json request = nlohmann::json::parse(
                            evt.GetString().ToUTF8().data());
                    requestId = request.value("__id", -1);
                } catch (...) {
                }

                wxString errorScript =
                        wxString::Format("window.wxApi.rejectInvoke('%s', %d);",
                                         EscapeJavascriptString(
                                                 errorResponse.dump()),
                                         requestId);
                webview->RunScript(errorScript);
            }
        }
    }

    void MainFrame::OnWebViewError(wxWebViewEvent &evt) {
        spdlog::error("WebView错误: {}", evt.GetString().ToStdString());
    }

    void MainFrame::OnWebViewLoaded(wxWebViewEvent &evt) {
        spdlog::info("资源加载成功: {}", evt.GetURL().ToStdString());
    }

    wxString MainFrame::EscapeJavascriptString(const std::string &str) {
        wxString escaped = wxString::FromUTF8(str);
        escaped.Replace("\\", "\\\\");
        escaped.Replace("\"", "\\\"");
        escaped.Replace("\'", "\\\'");
        escaped.Replace("\n", "\\n");
        escaped.Replace("\r", "\\r");
        escaped.Replace("\t", "\\t");
        return escaped;
    }
} // namespace CryptoToysPP
