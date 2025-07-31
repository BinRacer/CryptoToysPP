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
#include "algorithm/base/base64.h"
#include <spdlog/spdlog.h>
namespace CryptoToysPP::Gui {
    MainFrame::MainFrame() :
        wxFrame(nullptr,
                wxID_ANY,
                "CryptoToysPP - Encoding, Encryption and Decryption Toolbox") {
        InitWebView();
        spdlog::debug("MainFrame initialized");
    }

    void MainFrame::InitWebView() {
        spdlog::debug("Initializing WebView...");

        // Selecting WebView backend
        wxString backend = wxWebViewBackendDefault;
        if (wxWebView::IsBackendAvailable(wxWebViewBackendEdge)) {
            backend = wxWebViewBackendEdge;
            spdlog::debug("Selected backend: Edge WebView");
        } else if (wxWebView::IsBackendAvailable(wxWebViewBackendWebKit)) {
            backend = wxWebViewBackendWebKit;
            spdlog::debug("Selected backend: WebKit");
        } else {
            spdlog::debug("Using default backend");
        }

        // Creating WebView component
        webview = wxWebView::New(this, wxID_ANY, "", wxDefaultPosition,
                                 wxSize(WINDOW_WIDTH, WINDOW_HEIGHT), backend,
                                 wxBORDER_NONE);

        if (!webview) {
            spdlog::error("WebView creation failed");
            Close(true);
            return;
        }
        spdlog::info("WebView created successfully");

        // Configuring WebView features
        webview->RegisterHandler(
                wxSharedPtr<wxWebViewHandler>(new Route::SchemeHandler()));
        spdlog::debug("Registered custom scheme handler");

        webview->SetUserAgent(
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                "(KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36");
        spdlog::debug("User agent configured");

        // Binding event handlers
        webview->Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED,
                      &MainFrame::OnScriptMessage, this);
        webview->AddScriptMessageHandler("CryptoToysPP");
        spdlog::info("Registered script handler: CryptoToysPP");

        webview->Bind(wxEVT_WEBVIEW_ERROR, &MainFrame::OnWebViewError, this);
        webview->Bind(wxEVT_WEBVIEW_LOADED, &MainFrame::OnWebViewLoaded, this);

        // Initial configuration
        webview->LoadURL("app://index.html");
        spdlog::info("Loading initial page: app://index.html");

        webview->EnableContextMenu(true);
        webview->EnableAccessToDevTools(true);
        spdlog::debug("Developer tools enabled");

        SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        Center();
        Show();
        spdlog::info("Main window displayed");
    }

    void MainFrame::OnScriptMessage(wxWebViewEvent &evt) {
        if (evt.GetMessageHandler() != "CryptoToysPP")
            return;

        spdlog::debug("Processing CryptoToysPP message");
        std::string requestId;

        try {
            // Decoding and parsing the request
            std::string base64Payload = evt.GetString().ToStdString();
            spdlog::debug("Pre-Base64 request data: {}", base64Payload);
            std::string requestJson =
                    Algorithm::Base::Base64::Decode(base64Payload);
            spdlog::debug("Post-Base64 request data: {}", requestJson);

            const auto request = nlohmann::json::parse(requestJson);
            requestId = request.value("__id", "");
            // Routing the request
            const nlohmann::json response = route.ProcessRequest(request);
            SendOkResp(requestId, response);
        } catch (const nlohmann::json::parse_error &e) {
            spdlog::error("[{}] JSON parse error: {}", requestId, e.what());
            SendErrResp(requestId,
                        "JSON format error: " + std::string(e.what()));
        } catch (const nlohmann::json::type_error &e) {
            spdlog::error("[{}] JSON type error: {}", requestId, e.what());
            SendErrResp(requestId, "Data type error: " + std::string(e.what()));
        } catch (const std::exception &e) {
            spdlog::error("[{}] Processing error: {}", requestId, e.what());
            SendErrResp(requestId,
                        "Processing failed: " + std::string(e.what()));
        } catch (...) {
            spdlog::error("[{}] Unknown error occurred", requestId);
            SendErrResp(requestId, "Unknown error");
        }
    }

    void MainFrame::SendOkResp(const std::string &requestId,
                               const nlohmann::json &response) {
        // Extracting raw response content
        std::string responseJson = response.dump();
        spdlog::debug("[{}] Pre-Base64 response data: {}", requestId,
                      responseJson);

        // Performing Base64 encoding
        std::string base64Response =
                Algorithm::Base::Base64::Encode(responseJson);
        spdlog::debug("[{}] Post-Base64 response data: {}", requestId,
                      base64Response);

        // Sending response
        const wxString script =
                wxString::Format("window.rest.resolveInvoke('%s', '%s');",
                                 base64Response, requestId);

        webview->RunScriptAsync(script);
        spdlog::debug("[{}] Success response sent", requestId);
    }

    void MainFrame::SendErrResp(const std::string &requestId,
                                const std::string &message) {
        // Constructing error response
        const nlohmann::json response = {{"code", 500},
                                         {"message", message},
                                         {"data", nlohmann::json::object()}};

        // Extracting raw response content
        std::string responseJson = response.dump();
        spdlog::error("[{}] Pre-Base64 error response: {}", requestId,
                      responseJson);

        // Performing Base64 encoding
        std::string base64Response =
                Algorithm::Base::Base64::Encode(responseJson);
        spdlog::error("[{}] Post-Base64 error response: {}", requestId,
                      base64Response);

        // Sending error response
        const wxString script =
                wxString::Format("window.rest.rejectInvoke('%s', '%s');",
                                 base64Response, requestId);

        webview->RunScriptAsync(script);
        spdlog::error("[{}] Error response sent", requestId);
    }

    void MainFrame::OnWebViewError(wxWebViewEvent &evt) {
        spdlog::error("WebView error: [type={}] [url={}] {}", evt.GetInt(),
                      evt.GetURL().ToStdString(),
                      evt.GetString().ToStdString());
    }

    void MainFrame::OnWebViewLoaded(wxWebViewEvent &evt) {
        spdlog::debug("Page loaded: {} [status={}]", evt.GetURL().ToStdString(),
                     evt.GetInt());
    }
} // namespace CryptoToysPP::Gui
