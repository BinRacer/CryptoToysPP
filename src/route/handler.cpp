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
#include <wx/filename.h>
#include <wx/mstream.h>

namespace CryptoToysPP::Route {
    SchemeHandler::SchemeHandler() : wxWebViewHandler("app") {
    }

    wxFSFile *SchemeHandler::GetFile(const wxString &uri) {
        try {
            // 解析请求路径并执行安全验证
            ValidationResult pathResult = ValidateResourcePath(uri);
            if (!pathResult.valid) {
                spdlog::error("路径验证失败: {} - {}", uri.ToStdString(),
                              pathResult.message);
                return nullptr;
            }

            const std::string key = pathResult.safePath;
            spdlog::info("资源请求: {}", key);

            // 安全获取资源描述符
            ResourceDescriptor descriptor = GetResourceDescriptor(key);
            if (!descriptor.valid) {
                spdlog::error("获取资源描述符失败: {}", descriptor.message);
                return nullptr;
            }

            // 边界检查和零拷贝访问
            if (!CheckResourceBounds(descriptor)) {
                spdlog::error(
                        "资源边界检查失败: {} [偏移={}, 长度={}, 总大小={}]",
                        key, descriptor.offset, descriptor.length,
                        Resources::RESOURCE_DATA.size);
                return nullptr;
            }

            // 安全创建内存流
            return CreateSecureMemoryStream(key, descriptor);
        } catch (const std::exception &e) {
            spdlog::error("资源处理异常: {} - {}", uri.ToStdString(), e.what());
            return nullptr;
        }
    }

    SchemeHandler::ValidationResult
    SchemeHandler::ValidateResourcePath(const wxString &uri) {
        ValidationResult result;
        wxString path = uri.AfterFirst(':');

        // 移除多余斜杠
        path = path.Trim(false).Trim(true);
        while (path.StartsWith("/"))
            path = path.Mid(1);
        if (path.IsEmpty()) {
            result.message = "请求路径为空";
            return result;
        }

        // 阻止目录遍历攻击
        if (path.Contains("..") || path.Contains("//")) {
            result.message = "路径包含非法序列";
            return result;
        }

        // 检查扩展名白名单
        wxString ext = wxFileName(path).GetExt().Lower();
        static const std::vector<wxString> allowedExts = {"html", "js",
                                                          "css",  "png",
                                                          "jpg",  "jpeg",
                                                          "gif",  "json",
                                                          "ico"};
        if (std::ranges::find(allowedExts, ext) == allowedExts.end()) {
            result.message = "不支持的资源类型: " + ext.ToStdString();
            return result;
        }

        // 修正路径处理: 分开替换操作和字符串构建
        path.Replace("\\", "/", true);              // 先执行替换操作
        result.safePath = "/" + path.ToStdString(); // 然后转换为字符串
        result.valid = true;
        return result;
    }

    SchemeHandler::ResourceDescriptor
    SchemeHandler::GetResourceDescriptor(const std::string &key) {
        ResourceDescriptor descriptor{};
        auto lock = std::lock_guard<std::mutex>(resource_map_mutex);

        auto it = Resources::RESOURCE_MAP.find(key);
        if (it == Resources::RESOURCE_MAP.end()) {
            descriptor.message = "资源未找到: " + key;
            return descriptor;
        }

        descriptor.offset = it->second.first;
        descriptor.length = it->second.second;
        descriptor.valid = true;
        return descriptor;
    }

    bool
    SchemeHandler::CheckResourceBounds(const ResourceDescriptor &descriptor) {
        if (descriptor.offset >= Resources::RESOURCE_DATA.size) {
            return false;
        }
        if (descriptor.length == 0) {
            return false;
        }
        if (descriptor.offset + descriptor.length >
            Resources::RESOURCE_DATA.size) {
            return false;
        }
        return true;
    }

    wxFSFile *SchemeHandler::CreateSecureMemoryStream(
            const std::string &key,
            const ResourceDescriptor &descriptor) {
        // 安全获取数据指针
        const uint8_t *data_ptr =
                Resources::RESOURCE_DATA.data + descriptor.offset;

        // 创建具有边界保护的输入流
        auto stream = std::make_unique<wxMemoryInputStream>(
                reinterpret_cast<const char *>(data_ptr),
                static_cast<size_t>(descriptor.length));

        // 验证流完整性
        if (!stream->IsOk() || stream->GetSize() != descriptor.length) {
            throw std::runtime_error("资源流创建失败");
        }

        return new wxFSFile(stream.release(), "app:/" + key, GetMimeType(key),
                            wxEmptyString, wxDateTime::Now());
    }

    wxString SchemeHandler::GetMimeType(const std::string &path) {
        if (path.find(".html") != std::string::npos)
            return "text/html";
        if (path.find(".js") != std::string::npos)
            return "application/javascript";
        if (path.find(".css") != std::string::npos)
            return "text/css";
        if (path.find(".png") != std::string::npos)
            return "image/png";
        if (path.find(".jpg") != std::string::npos)
            return "image/jpeg";
        if (path.find(".json") != std::string::npos)
            return "application/json";
        if (path.find(".ico") != std::string::npos)
            return "image/x-icon";
        return "application/octet-stream";
    }
} // namespace CryptoToysPP::Route
