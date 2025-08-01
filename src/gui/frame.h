/* clang-format off */
/*
 * @file frame.h
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
#ifndef FRAME_H
#define FRAME_H
#include "route/route.h"
#include <wx/wx.h>
#include <wx/webview.h>
namespace CryptoToysPP::Gui {
    class MainFrame : public wxFrame {
    private:
        wxWebView *webview = nullptr;
        Route::Route route;
        static constexpr int WINDOW_WIDTH = 1024;
        static constexpr int WINDOW_HEIGHT = 768;
    public:
        MainFrame();

        void InitWebView();

        void OnScriptMessage(wxWebViewEvent &evt);

        void OnWebViewError(wxWebViewEvent &evt);

        void OnWebViewLoaded(wxWebViewEvent &evt);

    private:
        void SendOkResp(const std::string &requestId,
                    const nlohmann::json &response);

        void SendErrResp(const std::string &requestId, const std::string &message);
    };
} // namespace CryptoToysPP::Gui
#endif // FRAME_H
