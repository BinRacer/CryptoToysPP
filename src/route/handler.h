/* clang-format off */
/*
 * @file handler.h
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
#ifndef HANDLER_H
#define HANDLER_H
#include <wx/webview.h>
#include <wx/filesys.h>
#include <spdlog/spdlog.h>

namespace CryptoToysPP::Route {
    // Enhanced secure protocol handler
    class SchemeHandler : public wxWebViewHandler {
    private:
        std::mutex resource_map_mutex{};
        struct ValidationResult {
            bool valid = false;
            std::string safePath;
            std::string message;
        };

        struct ResourceDescriptor {
            bool valid = false;
            size_t offset;
            size_t length;
            std::string message;
        };

    public:
        SchemeHandler();

        wxFSFile *GetFile(const wxString &uri) override;

    private:
        // Resource path security validation function
        static ValidationResult ValidateResourcePath(const wxString &uri);

        // Secure resource descriptor retrieval
        ResourceDescriptor GetResourceDescriptor(const std::string &key);

        // Resource boundary safety check
        static bool CheckResourceBounds(const ResourceDescriptor &descriptor);

        // Secure memory stream creation
        static wxFSFile *
        CreateSecureMemoryStream(const std::string &key,
                                 const ResourceDescriptor &descriptor);

        static wxString GetMimeType(const std::string &path);
    };
} // namespace CryptoToysPP::Route

#endif // HANDLER_H
