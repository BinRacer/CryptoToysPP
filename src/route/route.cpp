/* clang-format off */
/*
 * @file route.cpp
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
#include "route.h"
#include "base/Base16.h"
#include "base/Base32.h"
#include "base/Base58.h"
#include "base/Base62.h"
#include "base/Base64.h"
#include "base/base85.h"
#include "base/base91.h"
#include "base/base92.h"
#include "base/base100.h"
#include <spdlog/spdlog.h>
namespace CryptoToysPP::Route {
    Route::Route() {
        spdlog::debug("Initializing route handlers...");

        // 注册API端点
        Add("POST", "/api/base/encode", [this](const nlohmann::json &data) {
            return BaseEncode(data);
        });
        Add("POST", "/api/base/decode", [this](const nlohmann::json &data) {
            return BaseDecode(data);
        });
    }

    nlohmann::json Route::BaseEncode(const nlohmann::json &data) {
        std::string encoded;
        const int bits = data.value("bits", 64);
        const std::string inputText = data.value("inputText", std::string());
        switch (bits) {
            case 16:
                encoded = Base::Base16::Encode(inputText);
                break;
            case 32:
                encoded = Base::Base32::Encode(inputText);
                break;
            case 58:
                encoded = Base::Base58::Encode(inputText);
                break;
            case 62:
                encoded = Base::Base62::Encode(inputText);
                break;
            case 64:
                encoded = Base::Base64::Encode(inputText);
                break;
            case 6464:
                encoded = Base::Base64::EncodeURL(inputText);
                break;
            case 85:
                encoded = Base::Base85::Encode(inputText);
                break;
            case 91:
                encoded = Base::Base91::Encode(inputText);
                break;
            case 92:
                encoded = Base::Base92::Encode(inputText);
                break;
            case 100:
                encoded = Base::Base100::Encode(inputText);
                break;
            default:
                break;
        }
        return encoded;
    }

    nlohmann::json Route::BaseDecode(const nlohmann::json &data) {
        std::string decoded;
        const int bits = data.value("bits", 64);
        const std::string inputText = data.value("inputText", std::string());
        switch (bits) {
            case 16:
                decoded = Base::Base16::Decode(inputText);
                break;
            case 32:
                decoded = Base::Base32::Decode(inputText);
                break;
            case 58:
                decoded = Base::Base58::Decode(inputText);
                break;
            case 62:
                decoded = Base::Base62::Decode(inputText);
                break;
            case 64:
                decoded = Base::Base64::Decode(inputText);
                break;
            case 6464:
                decoded = Base::Base64::DecodeURL(inputText);
                break;
            case 85:
                decoded = Base::Base85::Decode(inputText);
                break;
            case 91:
                decoded = Base::Base91::Decode(inputText);
                break;
            case 92:
                decoded = Base::Base92::Decode(inputText);
                break;
            case 100:
                decoded = Base::Base100::Decode(inputText);
                break;
            default:
                break;
        }
        return decoded;
    }

    void Route::Add(const std::string &method,
                    const std::string &path,
                    const HandlerFunc &handler) {
        const auto key = std::make_pair(method, path);
        if (routes.contains(key)) {
            spdlog::warn("Duplicate route handler registered for: {} - {}",
                         method, path);
        }
        routes[key] = handler;
        spdlog::debug("Registered handler for {} - {}", method, path);
    }

    nlohmann::json Route::ProcessRequest(const nlohmann::json &request) {
        if (!request.is_object()) {
            spdlog::error("Invalid request: must be a JSON object");
            return MakeErrResp(400, "Invalid request format");
        }
        // 验证必需字段
        if (!request.contains("__id")) {
            spdlog::error("Missing required field: '__id'");
            return MakeErrResp(400, "Required field '__id' is missing");
        }

        if (!request.contains("method")) {
            spdlog::error("Missing required field: 'method'");
            return MakeErrResp(400, "Required field 'method' is missing");
        }

        if (!request.contains("path")) {
            spdlog::error("Missing required field: 'path'");
            return MakeErrResp(400, "Required field 'path' is missing");
        }
        const std::string requestId = request.value("__id", "");
        try {
            const std::string method = request.value("method", "");
            const std::string path = request.value("path", "");
            // 提取请求数据
            const nlohmann::json data =
                    request.value("data", nlohmann::json::object());
            spdlog::info("[{}] Processing request for {} - {} : {}", requestId,
                         method, path, data.dump());
            // API频率限制
            if (!CheckRateLimit(path)) {
                spdlog::warn(
                        "[{}] Rate limit exceeded for {} - {} (429 Too Many "
                        "Requests)",
                        requestId, method, path);
                return MakeErrResp(429, "Too many requests");
            }
            // 路由分发
            const auto key = std::make_pair(method, path);
            if (!routes.contains(key)) {
                spdlog::warn("[{}] API endpoint not found: {} - {}", requestId,
                             method, path);
                return MakeErrResp(404, "API endpoint not found");
            }
            const auto response = routes[key](data);
            spdlog::info("[{}] Successfully processed {} - {} : {}", requestId,
                         method, path, response.dump());
            return MakeOkResp(200, response);
        } catch (const std::exception &e) {
            spdlog::error("[{}] Request processing error: {}", requestId,
                          e.what());
            return MakeErrResp(500, std::string("Internal error: ") + e.what());
        } catch (...) {
            spdlog::error("[{}] Unknown error during request processing",
                          requestId);
            return MakeErrResp(500, "Unknown internal error");
        }
    }

    bool Route::CheckRateLimit(const std::string &path) {
        auto now = std::chrono::steady_clock::now();
        auto &timePoints = rateLimits[path]; // 自动创建新队列

        // 清除过期记录 (修复类型问题)
        while (!timePoints.empty()) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                    now - timePoints.front());
            if (duration <= TIME_WINDOW)
                break; // 检查是否在时间窗口内
            timePoints.pop();
        }

        // 检查请求次数限制
        if (timePoints.size() >= MAX_REQUESTS) {
            return false;
        }

        timePoints.push(now);
        return true;
    }

    nlohmann::json Route::MakeOkResp(int code, const nlohmann::json &data) {
        return {{"code", code}, {"message", ""}, {"data", data}};
    }

    nlohmann::json Route::MakeErrResp(int code, const std::string &message) {
        return {{"code", code},
                {"message", message},
                {"data", nlohmann::json::object()}};
    }
} // namespace CryptoToysPP::Route
