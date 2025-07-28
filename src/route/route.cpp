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
#include "algorithm/base/Base16.h"
#include "algorithm/base/Base32.h"
#include "algorithm/base/Base58.h"
#include "algorithm/base/Base62.h"
#include "algorithm/base/Base64.h"
#include "algorithm/base/base85.h"
#include "algorithm/base/base91.h"
#include "algorithm/base/base92.h"
#include "algorithm/base/base100.h"
#include "algorithm/simple/uucode.h"
#include "algorithm/simple/xxcode.h"
#include "algorithm/simple/vigenere.h"
#include "algorithm/hash/hash.h"
#include "algorithm/advance/aes.h"
#include "algorithm/advance/rsa.h"
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
        Add("POST", "/api/simple/encode", [this](const nlohmann::json &data) {
            return SimpleEncode(data);
        });
        Add("POST", "/api/simple/decode", [this](const nlohmann::json &data) {
            return SimpleDecode(data);
        });
        Add("POST", "/api/hash/encode", [this](const nlohmann::json &data) {
            return HashEncode(data);
        });
        Add("POST", "/api/aes/encrypt", [this](const nlohmann::json &data) {
            return AesEncrypt(data);
        });
        Add("POST", "/api/aes/decrypt", [this](const nlohmann::json &data) {
            return AesDecrypt(data);
        });
        Add("POST", "/api/rsa/generate", [this](const nlohmann::json &data) {
            return RsaGenerate(data);
        });
        Add("POST", "/api/rsa/encrypt", [this](const nlohmann::json &data) {
            return RsaEncrypt(data);
        });
        Add("POST", "/api/rsa/decrypt", [this](const nlohmann::json &data) {
            return RsaDecrypt(data);
        });
    }

    nlohmann::json Route::BaseEncode(const nlohmann::json &data) {
        std::string encoded;
        const int bits = data.value("bits", 0);
        const std::string inputText = data.value("inputText", std::string());
        switch (bits) {
            case 16:
                encoded = Algorithm::Base::Base16::Encode(inputText);
                break;
            case 32:
                encoded = Algorithm::Base::Base32::Encode(inputText);
                break;
            case 58:
                encoded = Algorithm::Base::Base58::Encode(inputText);
                break;
            case 62:
                encoded = Algorithm::Base::Base62::Encode(inputText);
                break;
            case 64:
                encoded = Algorithm::Base::Base64::Encode(inputText);
                break;
            case 6464:
                encoded = Algorithm::Base::Base64::EncodeURL(inputText);
                break;
            case 85:
                encoded = Algorithm::Base::Base85::Encode(inputText);
                break;
            case 91:
                encoded = Algorithm::Base::Base91::Encode(inputText);
                break;
            case 92:
                encoded = Algorithm::Base::Base92::Encode(inputText);
                break;
            case 100:
                encoded = Algorithm::Base::Base100::Encode(inputText);
                break;
            default:
                break;
        }
        return encoded;
    }

    nlohmann::json Route::BaseDecode(const nlohmann::json &data) {
        std::string decoded;
        const int bits = data.value("bits", 0);
        const std::string inputText = data.value("inputText", std::string());
        switch (bits) {
            case 16:
                decoded = Algorithm::Base::Base16::Decode(inputText);
                break;
            case 32:
                decoded = Algorithm::Base::Base32::Decode(inputText);
                break;
            case 58:
                decoded = Algorithm::Base::Base58::Decode(inputText);
                break;
            case 62:
                decoded = Algorithm::Base::Base62::Decode(inputText);
                break;
            case 64:
                decoded = Algorithm::Base::Base64::Decode(inputText);
                break;
            case 6464:
                decoded = Algorithm::Base::Base64::DecodeURL(inputText);
                break;
            case 85:
                decoded = Algorithm::Base::Base85::Decode(inputText);
                break;
            case 91:
                decoded = Algorithm::Base::Base91::Decode(inputText);
                break;
            case 92:
                decoded = Algorithm::Base::Base92::Decode(inputText);
                break;
            case 100:
                decoded = Algorithm::Base::Base100::Decode(inputText);
                break;
            default:
                break;
        }
        return decoded;
    }

    nlohmann::json Route::SimpleEncode(const nlohmann::json &data) {
        std::string encoded;
        const std::string whichCode = data.value("whichCode", std::string());
        const std::string inputText = data.value("inputText", std::string());
        if (whichCode == "uu") {
            encoded = Algorithm::Simple::UUCode::Encode(inputText);
        } else if (whichCode == "xx") {
            encoded = Algorithm::Simple::XXCode::Encode(inputText);
        } else if (whichCode == "vigenere") {
            const std::string key = data.value("key", std::string());
            encoded = Algorithm::Simple::Vigenere::Encode(inputText, key);
        }
        return encoded;
    }

    nlohmann::json Route::SimpleDecode(const nlohmann::json &data) {
        std::string decoded;
        const std::string whichCode = data.value("whichCode", std::string());
        const std::string inputText = data.value("inputText", std::string());
        if (whichCode == "uu") {
            decoded = Algorithm::Simple::UUCode::Decode(inputText);
        } else if (whichCode == "xx") {
            decoded = Algorithm::Simple::XXCode::Decode(inputText);
        } else if (whichCode == "vigenere") {
            const std::string key = data.value("key", std::string());
            decoded = Algorithm::Simple::Vigenere::Decode(inputText, key);
        }
        return decoded;
    }

    nlohmann::json Route::HashEncode(const nlohmann::json &data) {
        std::string encoded;
        const std::string whichCode = data.value("whichCode", std::string());
        const std::string inputText = data.value("inputText", std::string());
        if (whichCode == "md2") {
            encoded = Algorithm::Hash::MD2(inputText);
        } else if (whichCode == "md4") {
            encoded = Algorithm::Hash::MD4(inputText);
        } else if (whichCode == "md5") {
            encoded = Algorithm::Hash::MD5(inputText);
        } else if (whichCode == "sha1") {
            encoded = Algorithm::Hash::SHA1(inputText);
        } else if (whichCode == "sha224") {
            encoded = Algorithm::Hash::SHA224(inputText);
        } else if (whichCode == "sha256") {
            encoded = Algorithm::Hash::SHA256(inputText);
        } else if (whichCode == "sha384") {
            encoded = Algorithm::Hash::SHA384(inputText);
        } else if (whichCode == "sha512") {
            encoded = Algorithm::Hash::SHA512(inputText);
        } else if (whichCode == "sha3-224") {
            encoded = Algorithm::Hash::SHA3_224(inputText);
        } else if (whichCode == "sha3-256") {
            encoded = Algorithm::Hash::SHA3_256(inputText);
        } else if (whichCode == "sha3-384") {
            encoded = Algorithm::Hash::SHA3_384(inputText);
        } else if (whichCode == "sha3-512") {
            encoded = Algorithm::Hash::SHA3_512(inputText);
        }
        return encoded;
    }

    nlohmann::json Route::AesEncrypt(const nlohmann::json &data) {
        const std::string plaintext = data.value("inputText", std::string());
        const std::string keyFormat = data.value("keyFormat", std::string());
        const std::string key = (keyFormat == "hex")
                ?Algorithm::Advance::AES::HexToString(data.value("key", std::string()))
                : data.value("key", std::string());
        const std::string ivFormat = data.value("ivFormat", std::string());
        const std::string iv = (ivFormat == "hex")
                ? Algorithm::Advance::AES::HexToString(data.value("iv", std::string()))
                : data.value("iv", std::string());
        Algorithm::Advance::AES::AESMode mode = Algorithm::Advance::AES::StringToAESMode(
                data.value("mode", std::string()));
        Algorithm::Advance::AES::PaddingScheme padding =
                Algorithm::Advance::AES::StringToPaddingScheme(
                        data.value("padding", std::string()));
        Algorithm::Advance::AES::KeyBits keyBits =
                Algorithm::Advance::AES::IntToKeyBits(data.value("keyBits", 0));
        Algorithm::Advance::AES::EncodingFormat outputEncoding =
                Algorithm::Advance::AES::StringToEncodingFormat(
                        data.value("encoding", std::string()));
        auto result = Algorithm::Advance::AES::Encrypt(plaintext, mode, padding, keyBits,
                                            key, iv, outputEncoding);
        if (result.success) {
            return result.data;
        } else {
            spdlog::error("AES Encrypt error: {}", result.error);
            return result.error;
        }
    }

    nlohmann::json Route::AesDecrypt(const nlohmann::json &data) {
        const std::string ciphertext = data.value("inputText", std::string());
        const std::string keyFormat = data.value("keyFormat", std::string());
        const std::string key = (keyFormat == "hex")
                ? Algorithm::Advance::AES::HexToString(data.value("key", std::string()))
                : data.value("key", std::string());
        const std::string ivFormat = data.value("ivFormat", std::string());
        const std::string iv = (ivFormat == "hex")
                ? Algorithm::Advance::AES::HexToString(data.value("iv", std::string()))
                : data.value("iv", std::string());
        Algorithm::Advance::AES::AESMode mode = Algorithm::Advance::AES::StringToAESMode(
                data.value("mode", std::string()));
        Algorithm::Advance::AES::PaddingScheme padding =
                Algorithm::Advance::AES::StringToPaddingScheme(
                        data.value("padding", std::string()));
        Algorithm::Advance::AES::KeyBits keyBits =
                Algorithm::Advance::AES::IntToKeyBits(data.value("keyBits", 0));
        Algorithm::Advance::AES::EncodingFormat inputEncoding =
                Algorithm::Advance::AES::StringToEncodingFormat(
                        data.value("encoding", std::string()));
        auto result = Algorithm::Advance::AES::Decrypt(ciphertext, mode, padding, keyBits,
                                            key, iv, inputEncoding);
        if (result.success) {
            return result.data;
        } else {
            spdlog::error("AES Decrypt error: {}", result.error);
            return result.error;
        }
    }

    nlohmann::json Route::RsaGenerate(const nlohmann::json &data) {
        Algorithm::Advance::RSA::KeySize keySize =
                Algorithm::Advance::RSA::IntToKeySize(data.value("keySize", 0));
        Algorithm::Advance::RSA::PEMFormatType pemFormat =
                Algorithm::Advance::RSA::StringToPEMFormatType(
                        data.value("pemType", std::string()));

        const auto [publicKey, privateKey] =
                Algorithm::Advance::RSA::GenerateKeyPair(keySize, pemFormat);
        if (publicKey.success && privateKey.success) {
            return {{"publicKey", publicKey.data},
                    {"privateKey", privateKey.data}};
        }
        return {{"publicKey", publicKey.error},
                {"privateKey", privateKey.error}};
    }

    nlohmann::json Route::RsaEncrypt(const nlohmann::json &data) {
        std::string plaintext = data.value("inputText", std::string());
        std::string pubKeyStr = data.value("publicKey", std::string());
        Algorithm::Advance::RSA::PEMFormatType pemFormat =
                Algorithm::Advance::RSA::StringToPEMFormatType(
                        data.value("pemType", std::string()));
        Algorithm::Advance::RSA::PaddingScheme paddingScheme =
                Algorithm::Advance::RSA::StringToPaddingScheme(
                        data.value("paddingScheme", std::string()));
        auto result = Algorithm::Advance::RSA::Encrypt(plaintext, pubKeyStr, pemFormat,
                                            paddingScheme);
        if (result.success) {
            return result.data;
        } else {
            spdlog::error("RSA Encrypt error: {}", result.error);
            return result.error;
        }
    }

    nlohmann::json Route::RsaDecrypt(const nlohmann::json &data) {
        std::string cipherText = data.value("inputText", std::string());
        std::string privKeyStr = data.value("privateKey", std::string());
        Algorithm::Advance::RSA::PEMFormatType pemFormat =
                Algorithm::Advance::RSA::StringToPEMFormatType(
                        data.value("pemType", std::string()));
        Algorithm::Advance::RSA::PaddingScheme paddingScheme =
                Algorithm::Advance::RSA::StringToPaddingScheme(
                        data.value("paddingScheme", std::string()));
        auto result = Algorithm::Advance::RSA::Decrypt(cipherText, privKeyStr, pemFormat,
                                            paddingScheme);
        if (result.success) {
            return result.data;
        } else {
            spdlog::error("RSA Decrypt error: {}", result.error);
            return result.error;
        }
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
        const std::string requestId = request.value("__id", std::string());
        try {
            const std::string method = request.value("method", std::string());
            const std::string path = request.value("path", std::string());
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
        return {{"code", code}, {"message", std::string()}, {"data", data}};
    }

    nlohmann::json Route::MakeErrResp(int code, const std::string &message) {
        return {{"code", code},
                {"message", message},
                {"data", nlohmann::json::object()}};
    }
} // namespace CryptoToysPP::Route
