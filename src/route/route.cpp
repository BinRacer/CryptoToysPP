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

namespace CryptoToysPP::Route {
    Route::Route() {
        Add("/api/base/encode", [this](const nlohmann::json &) {
            return handleGetUser();
        });
        Add("/api/getTime", [this](const nlohmann::json &) {
            return handleGetTime();
        });
        Add("/api/calculate", [this](const nlohmann::json &data) {
            return handleCalculate(data);
        });
    }

    void Route::Add(const std::string &path, const HandlerFunc &handler) {
        routes[path] = handler;
    }

    std::string Route::ProcessRequest(const nlohmann::json &request) {
        nlohmann::json response;
        try {
            if (!request.contains("path")) {
                return ErrResp(400, "缺少必要参数: path");
            }

            std::string path = request["path"].get<std::string>();
            nlohmann::json data =
                    request.value("data", nlohmann::json::object());

            // API访问频率限制（基本实现）
            if (!CheckRateLimit(path)) {
                return ErrResp(429, "请求过于频繁");
            }

            auto it = routes.find(path);
            if (it != routes.end()) {
                response = it->second(data);
            } else {
                response = ErrResp(404, "未找到API: " + path);
            }
        } catch (const std::exception &e) {
            response = ErrResp(500, "内部错误: " + std::string(e.what()));
        } catch (...) {
            response = ErrResp(500, "未知内部错误");
        }

        // 关键修复：确保使用UTF-8编码
        return response.dump();
    }

    nlohmann::json Route::ErrResp(int code, const std::string &message) {
        return {{"code", code},
                {"message", message},
                {"data", nlohmann::json::object()}};
    }

    bool Route::CheckRateLimit(const std::string &path) {
        auto now = std::chrono::steady_clock::now();
        auto &record = rateLimits[path];

        // 清除过期记录
        while (!record.empty() &&
               std::chrono::duration_cast<std::chrono::seconds>(now -
                                                                record.front())
                               .count() > TIME_WINDOW) {
            record.pop();
        }

        // 检查次数限制
        if (record.size() >= MAX_REQUESTS) {
            return false;
        }

        record.push(now);
        return true;
    }

    nlohmann::json Route::handleGetUser() {
        // 关键修复：使用UTF-8编码的中文字符
        return {{"code", 200},
                {"message", "Success"},
                {"data",
                 {{"name", "张三"},
                  {"age", 30},
                  {"email", "zhangsan@example.com"}}}};
    }

    nlohmann::json Route::handleGetTime() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        char time_buf[64];
        std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S",
                      std::localtime(&now_time));

        return {{"code", 200},
                {"message", "Success"},
                {"data",
                 {{"timestamp", static_cast<long>(now_time)},
                  {"iso_time", time_buf}}}};
    }

    nlohmann::json Route::handleCalculate(const nlohmann::json &data) {
        int a = data.at("a").get<int>();
        int b = data.at("b").get<int>();

        return {{"code", 200},
                {"message", "Success"},
                {"data", {{"operation", "addition"}, {"result", a + b}}}};
    }
} // namespace CryptoToysPP::Route
