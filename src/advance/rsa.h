/* clang-format off */
/*
 * @file rsa.h
 * @date 2025-07-27
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
#ifndef RSA_H
#define RSA_H
#include <string>
#include <utility>
#include <cryptopp/rsa.h>
namespace CryptoToysPP::Advance {
    class RSA {
    public:
        // 结果结构体
        struct Result {
            std::string data; // 主要数据
            bool success;
            std::string error;
        };

        // 密钥位数枚举
        enum class KeySize : unsigned {
            KS_1024 = 1024,
            KS_2048 = 2048,
            KS_3072 = 3072,
            KS_4096 = 4096,
            KS_UNKNOWN,
        };

        // PEM 格式类型
        enum class PEMFormatType {
            PKCS, // BEGIN PUBLIC KEY/PRIVATE KEY
            RSA,  // BEGIN RSA PUBLIC KEY/RSA PRIVATE KEY
            UNKNOWN,
        };

        // RSA填充方案
        enum class PaddingScheme {
            PKCS1v15,    // PKCS#1 v1.5填充
            OAEP_SHA1,   // OAEP填充，SHA-1哈希
            OAEP_SHA256, // OAEP填充，SHA-256哈希
            OAEP_SHA512, // OAEP填充，SHA-512哈希
            NO_PADDING,  // 无填充模式
            UNKNOWN,
        };

        static KeySize IntToKeySize(int keySize);

        static PEMFormatType StringToPEMFormatType(const std::string &format);

        static std::string PEMFormatTypeToString(PEMFormatType format);

        static PaddingScheme StringToPaddingScheme(const std::string &padding);

        static std::string PaddingSchemeToString(PaddingScheme paddingScheme);

        // 生成RSA密钥对
        [[nodiscard]] static std::pair<Result, Result>
        GenerateKeyPair(KeySize keySize = KeySize::KS_2048,
                        PEMFormatType format = PEMFormatType::PKCS);

        // 使用公钥加密
        [[nodiscard]] static Result
        Encrypt(std::string plainText,
                std::string pubKeyStr,
                PEMFormatType format = PEMFormatType::PKCS,
                PaddingScheme padding = PaddingScheme::OAEP_SHA256);

        // 使用私钥解密
        [[nodiscard]] static Result
        Decrypt(std::string cipherText,
                std::string privKeyStr,
                PEMFormatType format = PEMFormatType::PKCS,
                PaddingScheme padding = PaddingScheme::OAEP_SHA256);

    private:
        // PEM格式编码 - 私钥
        static Result EncodePEM(const CryptoPP::RSA::PrivateKey &key,
                                PEMFormatType format);

        // PEM格式编码 - 公钥
        static Result EncodePEM(const CryptoPP::RSA::PublicKey &key,
                                PEMFormatType format);

        // 加载公钥
        static Result LoadPublicKey(std::string keyStr,
                                    CryptoPP::RSA::PublicKey &key,
                                    PEMFormatType format);

        // 加载私钥
        static Result LoadPrivateKey(std::string keyStr,
                                     CryptoPP::RSA::PrivateKey &key,
                                     PEMFormatType format);
    };
} // namespace CryptoToysPP::Advance
#endif // RSA_H
