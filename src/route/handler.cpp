/* clang-format off */
/*
 * @file handler.cpp
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
#include "handler.h"
#include "resources/resources.h"
#include <map>
#include <wx/filename.h>
#include <wx/mstream.h>

namespace CryptoToysPP::Route {
    SchemeHandler::SchemeHandler() : wxWebViewHandler("app") {
        spdlog::debug("Custom scheme handler initialized for 'app' protocol");
    }

    wxFSFile *SchemeHandler::GetFile(const wxString &uri) {
        const std::string uriStr = uri.ToStdString();
        spdlog::info("Processing resource request: {}", uriStr);

        try {
            // Security validation of resource path
            ValidationResult pathResult = ValidateResourcePath(uri);
            if (!pathResult.valid) {
                spdlog::error("Path validation failed: {} - {}", uriStr,
                              pathResult.message);
                return nullptr;
            }

            const std::string &resourceKey = pathResult.safePath;
            spdlog::debug("Validated resource key: {}", resourceKey);

            // Retrieve resource descriptor
            ResourceDescriptor descriptor = GetResourceDescriptor(resourceKey);
            if (!descriptor.valid) {
                spdlog::error("Resource descriptor error: {}",
                              descriptor.message);
                return nullptr;
            }

            // Boundary validation
            if (!CheckResourceBounds(descriptor)) {
                spdlog::error("Resource bounds violation: {} [offset={}, "
                              "length={}, totalSize={}]",
                              resourceKey, descriptor.offset, descriptor.length,
                              Resources::RESOURCE_DATA.size);
                return nullptr;
            }

            // Create secure memory stream
            return CreateSecureMemoryStream(resourceKey, descriptor);

        } catch (const std::exception &e) {
            spdlog::error("Resource handling exception: {} - {}", uriStr,
                          e.what());
        } catch (...) {
            spdlog::error("Unknown exception during resource handling: {}",
                          uriStr);
        }
        return nullptr;
    }

    SchemeHandler::ValidationResult
    SchemeHandler::ValidateResourcePath(const wxString &uri) {
        ValidationResult result;
        wxString path = uri.AfterFirst(':');

        // Normalize path
        path = path.Trim(false).Trim(true);
        while (path.StartsWith("/"))
            path = path.Mid(1);

        if (path.IsEmpty()) {
            result.message = "Empty resource path";
            return result;
        }

        // Security checks: Prevent path traversal attacks
        if (path.Contains("..") || path.Contains("//")) {
            result.message = "Path contains illegal sequence";
            return result;
        }

        // Validate file extension
        const wxString ext = wxFileName(path).GetExt().Lower();
        static const std::vector<wxString> allowedExtensions = {"html", "js",
                                                                "css",  "png",
                                                                "jpg",  "jpeg",
                                                                "gif",  "json",
                                                                "ico",  "svg"};

        if (std::ranges::find(allowedExtensions, ext) ==
            allowedExtensions.end()) {
            result.message = "Unsupported resource type: " + ext.ToStdString();
            return result;
        }

        // Standardize path format
        path.Replace("\\", "/", true);
        result.safePath = "/" + path.ToStdString();
        result.valid = true;

        spdlog::debug("Path validated: {}", result.safePath);
        return result;
    }

    SchemeHandler::ResourceDescriptor
    SchemeHandler::GetResourceDescriptor(const std::string &key) {
        ResourceDescriptor descriptor{};
        std::lock_guard<std::mutex> lock(resource_map_mutex);

        const auto it = Resources::RESOURCE_MAP.find(key);
        if (it == Resources::RESOURCE_MAP.end()) {
            descriptor.message = "Resource not found: " + key;
            return descriptor;
        }

        descriptor.offset = it->second.first;
        descriptor.length = it->second.second;
        descriptor.valid = true;

        spdlog::debug("Resource descriptor: offset={}, length={}",
                      descriptor.offset, descriptor.length);
        return descriptor;
    }

    bool
    SchemeHandler::CheckResourceBounds(const ResourceDescriptor &descriptor) {
        // Validate offset
        if (descriptor.offset >= Resources::RESOURCE_DATA.size) {
            spdlog::error("Offset exceeds resource size: {} >= {}",
                          descriptor.offset, Resources::RESOURCE_DATA.size);
            return false;
        }

        // Validate length
        if (descriptor.length == 0) {
            spdlog::error("Zero-length resource");
            return false;
        }

        // Check boundary overflow
        if (descriptor.offset + descriptor.length >
            Resources::RESOURCE_DATA.size) {
            spdlog::error("Resource bounds exceeded: {}+{} > {}",
                          descriptor.offset, descriptor.length,
                          Resources::RESOURCE_DATA.size);
            return false;
        }

        return true;
    }

    wxFSFile *SchemeHandler::CreateSecureMemoryStream(
            const std::string &key,
            const ResourceDescriptor &descriptor) {

        // Access resource data pointer
        const uint8_t *dataPtr =
                Resources::RESOURCE_DATA.data + descriptor.offset;

        // Create memory input stream
        auto stream = std::make_unique<wxMemoryInputStream>(
                reinterpret_cast<const char *>(dataPtr),
                static_cast<size_t>(descriptor.length));

        // Verify stream integrity
        if (!stream->IsOk() ||
            stream->GetSize() != static_cast<size_t>(descriptor.length)) {
            spdlog::error("Stream creation failed for {}", key);
            throw std::runtime_error("Resource stream creation failed");
        }

        // Determine MIME type
        wxString mimeType = GetMimeType(key);
        spdlog::debug("MIME type for {}: {}", key, mimeType.ToStdString());

        // Create file object
        return new wxFSFile(stream.release(), "app:/" + key, mimeType,
                            wxEmptyString, wxDateTime::Now());
    }

    wxString SchemeHandler::GetMimeType(const std::string &path) {
        // MIME type mapping by file extension
        static const std::map<std::string, wxString> mimeMap =
                {{".html", "text/html"},
                 {".htm", "text/html"},
                 {".js", "application/javascript"},
                 {".css", "text/css"},
                 {".png", "image/png"},
                 {".jpg", "image/jpeg"},
                 {".jpeg", "image/jpeg"},
                 {".gif", "image/gif"},
                 {".json", "application/json"},
                 {".ico", "image/x-icon"},
                 {".svg", "image/svg+xml"}};

        // Match file extension
        for (const auto &[ext, mime] : mimeMap) {
            if (path.find(ext) != std::string::npos) {
                return mime;
            }
        }

        return "application/octet-stream";
    }
} // namespace CryptoToysPP::Route
