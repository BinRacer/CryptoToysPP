/* clang-format off */
/*
 * @file rsa.cpp
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
#include "rsa.h"
#include <cryptopp/base64.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/rsa.h>
#include <spdlog/spdlog.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <ranges>
namespace CryptoToysPP::Algorithm::Advance {
    // Helper function: Convert CryptoPP::Integer to hexadecimal string
    std::string integer_to_hex(const CryptoPP::Integer &num,
                               size_t max_bytes = 16) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        size_t byteCount = num.MinEncodedSize(CryptoPP::Integer::UNSIGNED);
        size_t printCount = std::min(byteCount, max_bytes);

        for (size_t i = 0; i < printCount; ++i) {
            oss << std::setw(2) << static_cast<unsigned>(num.GetByte(i));
            if (i < printCount - 1)
                oss << " ";
        }

        if (byteCount > max_bytes) {
            oss << " ... (" << (byteCount - max_bytes) << " more bytes)";
        }

        return oss.str();
    }

    // Helper function: Convert binary data to hexadecimal string
    std::string to_hex_string(const std::string &data, size_t max_len = 32) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        size_t len = std::min(data.size(), max_len);
        for (size_t i = 0; i < len; ++i) {
            oss << std::setw(2)
                << static_cast<unsigned>(static_cast<unsigned char>(data[i]));
            if (i < len - 1)
                oss << " ";
        }
        if (data.size() > max_len) {
            oss << " ... (" << (data.size() - max_len) << " more bytes)";
        }
        return oss.str();
    }

    RSA::KeySize RSA::IntToKeySize(int keySize) {
        auto size = KeySize::KS_UNKNOWN;
        switch (keySize) {
            case 512:
                size = KeySize::KS_512;
                break;
            case 1024:
                size = KeySize::KS_1024;
                break;
            case 2048:
                size = KeySize::KS_2048;
                break;
            case 3072:
                size = KeySize::KS_3072;
                break;
            case 4096:
                size = KeySize::KS_4096;
                break;
            default:
                break;
        }
        return size;
    }

    RSA::PEMFormatType RSA::StringToPEMFormatType(const std::string &format) {
        auto pemFormat = PEMFormatType::UNKNOWN;
        if (format == "PKCS")
            pemFormat = PEMFormatType::PKCS;
        else if (format == "RSA")
            pemFormat = PEMFormatType::RSA;
        return pemFormat;
    }

    std::string RSA::PEMFormatTypeToString(PEMFormatType format) {
        switch (format) {
            case PEMFormatType::PKCS:
                return "PKCS";
            case PEMFormatType::RSA:
                return "RSA";
            default:
                return "UNKNOWN";
        }
    }

    RSA::PaddingScheme RSA::StringToPaddingScheme(const std::string &padding) {
        auto paddingScheme = PaddingScheme::UNKNOWN;
        if (padding == "PKCS1v15")
            paddingScheme = PaddingScheme::PKCS1v15;
        else if (padding == "OAEP_SHA1")
            paddingScheme = PaddingScheme::OAEP_SHA1;
        else if (padding == "OAEP_SHA256")
            paddingScheme = PaddingScheme::OAEP_SHA256;
        else if (padding == "OAEP_SHA512")
            paddingScheme = PaddingScheme::OAEP_SHA512;
        else if (padding == "NO_PADDING")
            paddingScheme = PaddingScheme::NO_PADDING;
        return paddingScheme;
    }

    std::string RSA::PaddingSchemeToString(PaddingScheme paddingScheme) {
        switch (paddingScheme) {
            case PaddingScheme::PKCS1v15:
                return "PKCS1v15";
            case PaddingScheme::OAEP_SHA1:
                return "OAEP_SHA1";
            case PaddingScheme::OAEP_SHA256:
                return "OAEP_SHA256";
            case PaddingScheme::OAEP_SHA512:
                return "OAEP_SHA512";
            case PaddingScheme::NO_PADDING:
                return "NO_PADDING";
            default:
                return "UNKNOWN";
        }
    }

    std::pair<RSA::Result, RSA::Result>
    RSA::GenerateKeyPair(KeySize keySize, PEMFormatType format) {
        Result publicKeyResult, privateKeyResult;

        spdlog::debug("Generating RSA key pair, size: {} bits",
                     static_cast<unsigned>(keySize));

        try {
            CryptoPP::AutoSeededRandomPool rng{};
            CryptoPP::RSA::PrivateKey privKey;
            privKey.GenerateRandomWithKeySize(rng,
                                              static_cast<unsigned>(keySize));
            CryptoPP::RSA::PublicKey pubKey;
            pubKey.Initialize(privKey.GetModulus(), privKey.GetPublicExponent());

            publicKeyResult = EncodePEM(pubKey, format);
            privateKeyResult = EncodePEM(privKey, format);

            if (!publicKeyResult.success || !privateKeyResult.success) {
                spdlog::error("Key generation failed during PEM encoding");
                publicKeyResult.success = privateKeyResult.success = false;
            } else {
                spdlog::debug("Key pair generated successfully. Public key "
                             "format: {}, Private key format: {}",
                             PEMFormatTypeToString(format),
                             PEMFormatTypeToString(format));
            }
        } catch (const CryptoPP::Exception &e) {
            std::string error =
                    "Key generation failed: " + std::string(e.what());
            spdlog::error(error);
            privateKeyResult = {"", false, error};
            publicKeyResult = {"", false, error};
        } catch (const std::exception &e) {
            std::string error =
                    "Key generation failed: " + std::string(e.what());
            spdlog::error(error);
            privateKeyResult = {"", false, error};
            publicKeyResult = {"", false, error};
        }

        return std::make_pair(publicKeyResult, privateKeyResult);
    }

    RSA::Result RSA::EncodePEM(const CryptoPP::RSA::PublicKey &key,
                               PEMFormatType format) {
        Result result;

        try {
            CryptoPP::ByteQueue queue;
            key.Save(queue); // Using Save instead of DEREncodePublicKey

            std::string base64;
            CryptoPP::Base64Encoder encoder(new CryptoPP::StringSink(base64),
                                            true, 64);

            queue.CopyTo(encoder);
            encoder.MessageEnd();

            // Remove all newline characters
            base64.erase(std::ranges::remove(base64, '\n').begin(),
                         base64.end());
            base64.erase(std::ranges::remove(base64, '\r').begin(),
                         base64.end());

            switch (format) {
                case PEMFormatType::PKCS:
                    result.data = "-----BEGIN PUBLIC KEY-----\n" + base64 +
                            "\n-----END PUBLIC KEY-----";
                    break;
                case PEMFormatType::RSA:
                    result.data = "-----BEGIN RSA PUBLIC KEY-----\n" + base64 +
                            "\n-----END RSA PUBLIC KEY-----";
                    break;
                default:
                    result.error = "Invalid PEM format type";
                    result.success = false;
                    return result;
            }
            result.success = true;
        } catch (const CryptoPP::Exception &e) {
            result.error =
                    "Public key encoding failed: " + std::string(e.what());
            result.success = false;
        }
        return result;
    }

    RSA::Result RSA::EncodePEM(const CryptoPP::RSA::PrivateKey &key,
                               PEMFormatType format) {
        Result result;

        try {
            CryptoPP::ByteQueue queue;
            key.Save(queue); // Using Save instead of DEREncodePrivateKey

            std::string base64;
            CryptoPP::Base64Encoder encoder(new CryptoPP::StringSink(base64),
                                            true, 64);

            queue.CopyTo(encoder);
            encoder.MessageEnd();

            // Remove all newline characters
            base64.erase(std::ranges::remove(base64, '\n').begin(),
                         base64.end());
            base64.erase(std::ranges::remove(base64, '\r').begin(),
                         base64.end());

            switch (format) {
                case PEMFormatType::PKCS:
                    result.data = "-----BEGIN PRIVATE KEY-----\n" + base64 +
                            "\n-----END PRIVATE KEY-----";
                    break;
                case PEMFormatType::RSA:
                    result.data = "-----BEGIN RSA PRIVATE KEY-----\n" + base64 +
                            "\n-----END RSA PRIVATE KEY-----";
                    break;
                default:
                    result.error = "Invalid PEM format type";
                    result.success = false;
                    return result;
            }
            result.success = true;
        } catch (const CryptoPP::Exception &e) {
            result.error =
                    "Private key encoding failed: " + std::string(e.what());
            result.success = false;
        }
        return result;
    }

    RSA::Result RSA::LoadPublicKey(const std::string &keyStr,
                                   CryptoPP::RSA::PublicKey &key,
                                   PEMFormatType format) {
        Result result;

        static const std::string BEGIN_MARKER = "-----BEGIN ";
        static const std::string END_MARKER = "-----END ";

        try {
            spdlog::debug("Loading public key, format: {}",
                          PEMFormatTypeToString(format));

            // Locate BEGIN marker
            size_t beginPos = keyStr.find(BEGIN_MARKER);
            if (beginPos == std::string::npos) {
                spdlog::error("Missing BEGIN marker in public key");
                result.error = "Missing BEGIN marker";
                result.success = false;
                return result;
            }

            // Find end of BEGIN marker
            size_t beginEnd =
                    keyStr.find("-----", beginPos + BEGIN_MARKER.size());
            if (beginEnd == std::string::npos) {
                spdlog::error("Invalid BEGIN marker in public key");
                result.error = "Invalid BEGIN marker";
                result.success = false;
                return result;
            }
            beginEnd += 5; // Skip "-----"

            // Locate END marker
            size_t endPos = keyStr.find(END_MARKER, beginEnd);
            if (endPos == std::string::npos) {
                spdlog::error("Missing END marker in public key");
                result.error = "Missing END marker";
                result.success = false;
                return result;
            }

            // Find end of END marker
            size_t endEnd = keyStr.find("-----", endPos + END_MARKER.size());
            if (endEnd == std::string::npos) {
                spdlog::error("Invalid END marker in public key");
                result.error = "Invalid END marker";
                result.success = false;
                return result;
            }

            // Extract content between BEGIN and END
            std::string base64 = keyStr.substr(beginEnd, endPos - beginEnd);

            // Remove non-Base64 characters (including newlines/spaces)
            base64.erase(std::ranges::remove_if(base64,
                                                [](char c) {
                                                    return !(std::isalnum(c) ||
                                                             c == '+' ||
                                                             c == '/' ||
                                                             c == '=');
                                                })
                                 .begin(),
                         base64.end());

            // Validate Base64 length (must be multiple of 4)
            if (base64.size() % 4 != 0) {
                // Add padding characters
                size_t padding = 4 - (base64.size() % 4);
                base64.append(padding, '=');
            }

            // Base64 decoding
            std::string der;
            CryptoPP::StringSource ss(base64, true,
                                      new CryptoPP::Base64Decoder(
                                              new CryptoPP::StringSink(der)));

            // Validate DER length
            if (der.empty()) {
                spdlog::error("DER decoding produced no data");
                result.error = "DER decoding failed";
                result.success = false;
                return result;
            }

            // Use BERDecode instead of Load
            CryptoPP::StringSource derSource(der, true);
            key.BERDecode(derSource); // Critical fix: using BERDecode

            // Validate key parameters
            const CryptoPP::Integer &n = key.GetModulus();
            const CryptoPP::Integer &e = key.GetPublicExponent();
            if (n.IsZero() || e.IsZero()) {
                spdlog::error("Invalid public key parameters after loading");
                result.error = "Invalid public key parameters";
                result.success = false;
                return result;
            }

            spdlog::debug(
                    "Public key loaded successfully. Modulus size: {} bits",
                    n.BitCount());
            result.success = true;
        } catch (const CryptoPP::Exception &e) {
            result.error =
                    "Public key loading failed: " + std::string(e.what());
            result.success = false;
        } catch (const std::exception &e) {
            result.error =
                    "Public key loading failed: " + std::string(e.what());
            result.success = false;
        }

        return result;
    }

    RSA::Result RSA::LoadPrivateKey(const std::string &keyStr,
                                    CryptoPP::RSA::PrivateKey &key,
                                    PEMFormatType format) {
        Result result;

        static const std::string BEGIN_MARKER = "-----BEGIN ";
        static const std::string END_MARKER = "-----END ";

        try {
            spdlog::debug("Loading private key, format: {}",
                          PEMFormatTypeToString(format));

            // Locate BEGIN marker
            size_t beginPos = keyStr.find(BEGIN_MARKER);
            if (beginPos == std::string::npos) {
                spdlog::error("Missing BEGIN marker in private key");
                result.error = "Missing BEGIN marker";
                result.success = false;
                return result;
            }

            // Find end of BEGIN marker
            size_t beginEnd =
                    keyStr.find("-----", beginPos + BEGIN_MARKER.size());
            if (beginEnd == std::string::npos) {
                spdlog::error("Invalid BEGIN marker in private key");
                result.error = "Invalid BEGIN marker";
                result.success = false;
                return result;
            }
            beginEnd += 5; // Skip "-----"

            // Locate END marker
            size_t endPos = keyStr.find(END_MARKER, beginEnd);
            if (endPos == std::string::npos) {
                spdlog::error("Missing END marker in private key");
                result.error = "Missing END marker";
                result.success = false;
                return result;
            }

            // Find end of END marker
            size_t endEnd = keyStr.find("-----", endPos + END_MARKER.size());
            if (endEnd == std::string::npos) {
                spdlog::error("Invalid END marker in private key");
                result.error = "Invalid END marker";
                result.success = false;
                return result;
            }

            // Extract content between BEGIN and END
            std::string base64 = keyStr.substr(beginEnd, endPos - beginEnd);

            // Remove non-Base64 characters (including newlines/spaces)
            base64.erase(std::ranges::remove_if(base64,
                                                [](char c) {
                                                    return !(std::isalnum(c) ||
                                                             c == '+' ||
                                                             c == '/' ||
                                                             c == '=');
                                                })
                                 .begin(),
                         base64.end());

            // Validate Base64 length (must be multiple of 4)
            if (base64.size() % 4 != 0) {
                // Add padding characters
                size_t padding = 4 - (base64.size() % 4);
                base64.append(padding, '=');
            }

            // Base64 decoding
            std::string der;
            CryptoPP::StringSource ss(base64, true,
                                      new CryptoPP::Base64Decoder(
                                              new CryptoPP::StringSink(der)));

            // Validate DER length
            if (der.empty()) {
                spdlog::error("DER decoding produced no data");
                result.error = "DER decoding failed";
                result.success = false;
                return result;
            }

            // Use BERDecode instead of Load
            CryptoPP::StringSource derSource(der, true);
            key.BERDecode(derSource); // Critical fix: using BERDecode

            // Validate key parameters
            const CryptoPP::Integer &n = key.GetModulus();
            const CryptoPP::Integer &e = key.GetPublicExponent();
            const CryptoPP::Integer &d = key.GetPrivateExponent();
            if (n.IsZero() || e.IsZero() || d.IsZero()) {
                spdlog::error("Invalid private key parameters after loading");
                result.error = "Invalid private key parameters";
                result.success = false;
                return result;
            }

            spdlog::debug(
                    "Private key loaded successfully. Modulus size: {} bits",
                    n.BitCount());
            result.success = true;
        } catch (const CryptoPP::Exception &e) {
            result.error =
                    "Private key loading failed: " + std::string(e.what());
            result.success = false;
        } catch (const std::exception &e) {
            result.error =
                    "Private key loading failed: " + std::string(e.what());
            result.success = false;
        }

        return result;
    }

    RSA::Result RSA::Encrypt(const std::string &plainText,
                             const std::string &pubKeyStr,
                             PEMFormatType format,
                             PaddingScheme padding) {
        Result result;
        CryptoPP::RSA::PublicKey pubKey;

        result = LoadPublicKey(pubKeyStr, pubKey, format);
        if (!result.success)
            return result;

        spdlog::debug("Encrypting {} bytes with padding scheme: {}",
                     plainText.size(), PaddingSchemeToString(padding));

        try {
            const size_t modulusSize = pubKey.GetModulus().ByteCount();
            std::string cipherText;

            CryptoPP::AutoSeededRandomPool rng;
            switch (padding) {
                case PaddingScheme::PKCS1v15: {
                    CryptoPP::RSAES_PKCS1v15_Encryptor encryptor(pubKey);
                    CryptoPP::StringSource ss(plainText, true,
                                              new CryptoPP::PK_EncryptorFilter(
                                                      rng, encryptor,
                                                      new CryptoPP::StringSink(
                                                              cipherText)));
                    break;
                }
                case PaddingScheme::OAEP_SHA1: {
                    CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(pubKey);
                    CryptoPP::StringSource ss(plainText, true,
                                              new CryptoPP::PK_EncryptorFilter(
                                                      rng, encryptor,
                                                      new CryptoPP::StringSink(
                                                              cipherText)));
                    break;
                }
                case PaddingScheme::OAEP_SHA256: {
                    CryptoPP::RSAES<CryptoPP::OAEP<CryptoPP::SHA256>>::Encryptor
                            encryptor(pubKey);
                    CryptoPP::StringSource ss(plainText, true,
                                              new CryptoPP::PK_EncryptorFilter(
                                                      rng, encryptor,
                                                      new CryptoPP::StringSink(
                                                              cipherText)));
                    break;
                }
                case PaddingScheme::OAEP_SHA512: {
                    CryptoPP::RSAES<CryptoPP::OAEP<CryptoPP::SHA512>>::Encryptor
                            encryptor(pubKey);
                    CryptoPP::StringSource ss(plainText, true,
                                              new CryptoPP::PK_EncryptorFilter(
                                                      rng, encryptor,
                                                      new CryptoPP::StringSink(
                                                              cipherText)));
                    break;
                }
                case PaddingScheme::NO_PADDING: {
                    // Ensure data length equals modulus size
                    if (plainText.size() > modulusSize) {
                        result.error = "Data too large for NO_PADDING mode";
                        result.success = false;
                        return result;
                    }

                    // Left-pad with zero bytes
                    std::string padded(modulusSize - plainText.size(), 0);
                    padded.append(plainText);

                    CryptoPP::Integer
                            m(reinterpret_cast<const CryptoPP::byte *>(
                                      padded.data()),
                              padded.size());
                    CryptoPP::Integer c = pubKey.ApplyFunction(m);
                    cipherText.resize(modulusSize);
                    c.Encode(reinterpret_cast<CryptoPP::byte *>(
                                     cipherText.data()),
                             modulusSize);
                    break;
                }
                default:
                    result.error = "Unsupported padding scheme";
                    result.success = false;
                    return result;
            }

            // Base64 encoding (without line breaks)
            std::string base64Result;
            CryptoPP::StringSource
                    source(cipherText, true,
                           new CryptoPP::Base64Encoder(new CryptoPP::StringSink(
                                                               base64Result),
                                                       false // no line breaks
                                                       ));

            result.data = base64Result;
            result.success = true;
            spdlog::debug("Encryption successful. Ciphertext size: {} bytes "
                         "(Base64: {} chars)",
                         cipherText.size(), base64Result.size());
        } catch (const CryptoPP::Exception &e) {
            result.error = "Encryption failed: " + std::string(e.what());
            result.success = false;
        }
        return result;
    }

    RSA::Result RSA::Decrypt(const std::string &cipherText,
                             const std::string &privKeyStr,
                             PEMFormatType format,
                             PaddingScheme padding) {
        Result result;
        CryptoPP::RSA::PrivateKey privKey;

        result = LoadPrivateKey(privKeyStr, privKey, format);
        if (!result.success)
            return result;

        spdlog::debug("Decrypting {} bytes with padding scheme: {}",
                     cipherText.size(), PaddingSchemeToString(padding));

        try {
            // Base64 decoding
            std::string binaryCipher;
            CryptoPP::StringSource stringSource(
                    cipherText, true,
                    new CryptoPP::Base64Decoder(
                            new CryptoPP::StringSink(binaryCipher)));

            const size_t modulusSize = privKey.GetModulus().ByteCount();
            std::string decryptedText;

            CryptoPP::AutoSeededRandomPool rng;
            switch (padding) {
                case PaddingScheme::PKCS1v15: {
                    CryptoPP::RSAES_PKCS1v15_Decryptor decryptor(privKey);
                    CryptoPP::StringSource ss(binaryCipher, true,
                                              new CryptoPP::PK_DecryptorFilter(
                                                      rng, decryptor,
                                                      new CryptoPP::StringSink(
                                                              decryptedText)));
                    break;
                }
                case PaddingScheme::OAEP_SHA1: {
                    CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(privKey);
                    CryptoPP::StringSource ss(binaryCipher, true,
                                              new CryptoPP::PK_DecryptorFilter(
                                                      rng, decryptor,
                                                      new CryptoPP::StringSink(
                                                              decryptedText)));
                    break;
                }
                case PaddingScheme::OAEP_SHA256: {
                    CryptoPP::RSAES<CryptoPP::OAEP<CryptoPP::SHA256>>::Decryptor
                            decryptor(privKey);
                    CryptoPP::StringSource ss(binaryCipher, true,
                                              new CryptoPP::PK_DecryptorFilter(
                                                      rng, decryptor,
                                                      new CryptoPP::StringSink(
                                                              decryptedText)));
                    break;
                }
                case PaddingScheme::OAEP_SHA512: {
                    CryptoPP::RSAES<CryptoPP::OAEP<CryptoPP::SHA512>>::Decryptor
                            decryptor(privKey);
                    CryptoPP::StringSource ss(binaryCipher, true,
                                              new CryptoPP::PK_DecryptorFilter(
                                                      rng, decryptor,
                                                      new CryptoPP::StringSink(
                                                              decryptedText)));
                    break;
                }
                case PaddingScheme::NO_PADDING: {
                    if (binaryCipher.size() != modulusSize) {
                        result.error = "Ciphertext size must equal modulus "
                                       "size in NO_PADDING mode";
                        result.success = false;
                        return result;
                    }
                    CryptoPP::Integer
                            c(reinterpret_cast<const CryptoPP::byte *>(
                                      binaryCipher.data()),
                              binaryCipher.size());
                    CryptoPP::Integer m = privKey.CalculateInverse(rng, c);
                    decryptedText.resize(modulusSize);
                    m.Encode(reinterpret_cast<CryptoPP::byte *>(
                                     decryptedText.data()),
                             modulusSize);

                    // Remove leading zeros
                    size_t startPos = decryptedText.find_first_not_of('\0');
                    if (startPos != std::string::npos) {
                        decryptedText = decryptedText.substr(startPos);
                    } else {
                        decryptedText.clear();
                    }
                    break;
                }
                default:
                    result.error = "Unsupported padding scheme";
                    result.success = false;
                    return result;
            }

            result.data = decryptedText;
            result.success = true;
            spdlog::debug("Decryption successful. Plaintext size: {} bytes",
                         decryptedText.size());
        } catch (const CryptoPP::Exception &e) {
            result.error = "Decryption failed: " + std::string(e.what());
            result.success = false;
        }
        return result;
    }
} // namespace CryptoToysPP::Algorithm::Advance
