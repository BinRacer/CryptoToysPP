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
namespace CryptoToysPP::Algorithm::Advance {
    class RSA {
    public:
        // Result structure for operation outputs
        struct Result {
            std::string data;  // Primary data payload
            bool success;      // Operation status flag
            std::string error; // Error description if failed
        };

        // Supported key size specifications
        enum class KeySize : unsigned {
            KS_512 = 512,        // 512-bit key
            KS_1024 = 1024,      // 1024-bit key (default)
            KS_2048 = 2048,      // 2048-bit key
            KS_3072 = 3072,      // 3072-bit key
            KS_4096 = 4096,      // 4096-bit key
            KS_UNKNOWN = KS_4096 // Fallback size
        };

        // PEM encoding format types
        enum class PEMFormatType {
            PKCS,   // PEM header: BEGIN PUBLIC/PRIVATE KEY
            RSA,    // PEM header: BEGIN RSA PUBLIC/PRIVATE KEY
            UNKNOWN // Unspecified format
        };

        // RSA padding schemes
        enum class PaddingScheme {
            PKCS1v15,    // PKCS #1 v1.5 padding
            OAEP_SHA1,   // OAEP with SHA-1 digest
            OAEP_SHA256, // OAEP with SHA-256 digest
            OAEP_SHA512, // OAEP with SHA-512 digest
            NO_PADDING,  // Raw encryption mode
            UNKNOWN      // Unspecified scheme
        };

        // Utility converters
        static KeySize IntToKeySize(int keySize);
        static PEMFormatType StringToPEMFormatType(const std::string &format);
        static std::string PEMFormatTypeToString(PEMFormatType format);
        static PaddingScheme StringToPaddingScheme(const std::string &padding);
        static std::string PaddingSchemeToString(PaddingScheme paddingScheme);

        // Core cryptographic operations
        [[nodiscard]] static std::pair<Result, Result>
        GenerateKeyPair(KeySize keySize = KeySize::KS_2048,
                        PEMFormatType format = PEMFormatType::PKCS);

        [[nodiscard]] static Result
        Encrypt(const std::string &plainText,
                const std::string &pubKeyStr,
                PEMFormatType format = PEMFormatType::PKCS,
                PaddingScheme padding = PaddingScheme::OAEP_SHA256);

        [[nodiscard]] static Result
        Decrypt(const std::string &cipherText,
                const std::string &privKeyStr,
                PEMFormatType format = PEMFormatType::PKCS,
                PaddingScheme padding = PaddingScheme::OAEP_SHA256);

    private:
        // Internal PEM handlers
        static Result EncodePEM(const CryptoPP::RSA::PrivateKey &key,
                                PEMFormatType format);
        static Result EncodePEM(const CryptoPP::RSA::PublicKey &key,
                                PEMFormatType format);
        static Result LoadPublicKey(const std::string &keyStr,
                                    CryptoPP::RSA::PublicKey &key,
                                    PEMFormatType format);
        static Result LoadPrivateKey(const std::string &keyStr,
                                     CryptoPP::RSA::PrivateKey &key,
                                     PEMFormatType format);
    };
} // namespace CryptoToysPP::Algorithm::Advance
#endif // RSA_H
