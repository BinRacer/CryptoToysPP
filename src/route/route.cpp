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

#include <spdlog/spdlog.h>

namespace CryptoToysPP::Route {
    Route::Route() {
        spdlog::debug("Initializing route handlers...");

        // 注册API端点
        Add("/api/base/encode", [this](const nlohmann::json &) {
            return handleGetUser();
        });
    }

    void Route::Add(const std::string &path, const HandlerFunc &handler) {
        if (routes.find(path) != routes.end()) {
            spdlog::warn("Duplicate route handler registered for: {}", path);
        }
        routes[path] = handler;
        spdlog::debug("Registered handler for {}", path);
    }

    std::string Route::ProcessRequest(const nlohmann::json &request) {
        if (!request.is_object()) {
            spdlog::error("Invalid request: must be a JSON object");
            return ErrResp(400, "Invalid request format").dump();
        }

        try {
            // 验证必需字段
            if (!request.contains("path")) {
                spdlog::error("Missing required field: 'path'");
                return ErrResp(400, "Required field 'path' is missing").dump();
            }

            const std::string path = request["path"].get<std::string>();
            spdlog::info("Processing request for: {}", path);

            // 提取请求数据
            nlohmann::json data =
                    request.value("data", nlohmann::json::object());
            spdlog::debug("Request data: {}", data.dump());

            // API频率限制
            if (!CheckRateLimit(path)) {
                spdlog::warn(
                        "Rate limit exceeded for {} (429 Too Many Requests)",
                        path);
                return ErrResp(429, "Too many requests").dump();
            }

            // 路由分发
            if (routes.find(path) != routes.end()) {
                const auto response = routes[path](data);
                spdlog::info("Successfully processed {}", path);
                return response.dump(); // UTF-8确保
            }

            spdlog::warn("API endpoint not found: {}", path);
            return ErrResp(404, "API endpoint not found").dump();

        } catch (const std::exception &e) {
            spdlog::error("Request processing error: {}", e.what());
            return ErrResp(500, std::string("Internal error: ") + e.what())
                    .dump();
        } catch (...) {
            spdlog::error("Unknown error during request processing");
            return ErrResp(500, "Unknown internal error").dump();
        }
    }

    nlohmann::json Route::ErrResp(int code, const std::string &message) {
        return {{"code", code},
                {"message", message},
                {"data", nlohmann::json::object()}};
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

    nlohmann::json Route::handleGetUser() {
        return {{"code", 200},
                {"message", "Success"},
                {"data",
                 {{"name", "张三"},
                  {"age", 30},
                  {"email", "zhangsan@example.com"},
                  {"department", "Engineering"},
                  {"employeeId", "EMP007"}}}};
    }
} // namespace CryptoToysPP::Route
