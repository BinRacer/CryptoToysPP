/* clang-format off */
/*
 * @file route.h
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
#ifndef ROUTE_H
#define ROUTE_H
#include <queue>
#include <chrono>
#include <functional>
#include <nlohmann/json.hpp>
namespace CryptoToysPP::Route {
    using HandlerFunc = std::function<nlohmann::json(const nlohmann::json &)>;
    struct PairHash {
        size_t operator()(const std::pair<std::string, std::string> &p) const {
            // 使用黄金比例常数进行哈希组合
            constexpr size_t Golden = 0x9e3779b97f4a7c15;

            // 计算两个字符串的独立哈希值
            const size_t h1 = std::hash<std::string>{}(p.first);
            const size_t h2 = std::hash<std::string>{}(p.second);

            // 使用黄金比例常数进行组合
            return h1 ^ (h2 + Golden + (h1 << 6) + (h1 >> 2));
        }
    };
    class Route {
    private:
        std::unordered_map<std::pair<std::string, std::string>,
                           HandlerFunc,
                           PairHash>
                routes;
        // 限流设置
        static constexpr int MAX_REQUESTS = 100;
        static constexpr auto TIME_WINDOW = std::chrono::seconds(1);
        std::unordered_map<std::string,
                           std::queue<std::chrono::steady_clock::time_point>>
                rateLimits{};

    public:
        Route();

        nlohmann::json BaseEncode(const nlohmann::json &data);

        nlohmann::json BaseDecode(const nlohmann::json &data);

        nlohmann::json SimpleEncode(const nlohmann::json &data);

        nlohmann::json SimpleDecode(const nlohmann::json &data);

        nlohmann::json HashEncode(const nlohmann::json &data);

        nlohmann::json AesEncode(const nlohmann::json &data);

        nlohmann::json AesDecode(const nlohmann::json &data);

        void Add(const std::string &method,
                 const std::string &path,
                 const HandlerFunc &handler);

        nlohmann::json ProcessRequest(const nlohmann::json &request);

    private:
        bool CheckRateLimit(const std::string &path);

        nlohmann::json MakeOkResp(int code, const nlohmann::json &data);

        nlohmann::json MakeErrResp(int code, const std::string &message);
    };
} // namespace CryptoToysPP::Route

#endif // ROUTE_H
