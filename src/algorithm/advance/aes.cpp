/* clang-format off */
/*
 * @file aes.cpp
 * @date 2025-07-26
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
#include "aes.h"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/xts.h>
#include <cryptopp/ccm.h>
#include <cryptopp/eax.h>
#include <cryptopp/gcm.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>
#include <spdlog/spdlog.h>
namespace CryptoToysPP::Algorithm::Advance {
    std::string AES::AESModeToString(AESMode mode) {
        switch (mode) {
            case AESMode::ECB:
                return "ECB";
            case AESMode::CBC:
                return "CBC";
            case AESMode::OFB:
                return "OFB";
            case AESMode::CFB:
                return "CFB";
            case AESMode::XTS:
                return "XTS";
            case AESMode::CCM:
                return "CCM";
            case AESMode::EAX:
                return "EAX";
            case AESMode::GCM:
                return "GCM";
            default:
                return "UNKNOWN";
        }
    }

    AES::AESMode AES::StringToAESMode(const std::string &mode) {
        auto Mode = AESMode::UNKNOWN;
        if (mode == "ECB") {
            Mode = AESMode::ECB;
        } else if (mode == "CBC") {
            Mode = AESMode::CBC;
        } else if (mode == "OFB") {
            Mode = AESMode::OFB;
        } else if (mode == "CFB") {
            Mode = AESMode::CFB;
        } else if (mode == "XTS") {
            Mode = AESMode::XTS;
        } else if (mode == "CCM") {
            Mode = AESMode::CCM;
        } else if (mode == "EAX") {
            Mode = AESMode::EAX;
        } else if (mode == "GCM") {
            Mode = AESMode::GCM;
        }
        return Mode;
    }

    std::string AES::PaddingSchemeToString(PaddingScheme padding) {
        switch (padding) {
            case PaddingScheme::NONE:
                return "NONE";
            case PaddingScheme::ZEROS:
                return "ZEROS";
            case PaddingScheme::PKCS7:
                return "PKCS7";
            case PaddingScheme::ONE_AND_ZEROS:
                return "ONE_AND_ZEROS";
            case PaddingScheme::W3C:
                return "W3C";
            case PaddingScheme::DEFAULT:
                return "DEFAULT";
            default:
                return "UNKNOWN";
        }
    }

    AES::PaddingScheme AES::StringToPaddingScheme(const std::string &padding) {
        auto PaddingScheme = PaddingScheme::UNKNOWN;
        if (padding == "NONE") {
            PaddingScheme = PaddingScheme::NONE;
        } else if (padding == "ZEROS") {
            PaddingScheme = PaddingScheme::ZEROS;
        } else if (padding == "PKCS7") {
            PaddingScheme = PaddingScheme::PKCS7;
        } else if (padding == "ONE_AND_ZEROS") {
            PaddingScheme = PaddingScheme::ONE_AND_ZEROS;
        } else if (padding == "W3C") {
            PaddingScheme = PaddingScheme::W3C;
        } else if (padding == "DEFAULT") {
            PaddingScheme = PaddingScheme::DEFAULT;
        }
        return PaddingScheme;
    }

    AES::KeyBits AES::IntToKeyBits(int bits) {
        auto keyBits = KeyBits::BITS_UNKNOWN;
        switch (bits) {
            case 128:
                keyBits = KeyBits::BITS_128;
                break;
            case 192:
                keyBits = KeyBits::BITS_192;
                break;
            case 256:
                keyBits = KeyBits::BITS_256;
                break;
            default:
                break;
        }
        return keyBits;
    }

    AES::EncodingFormat AES::StringToEncodingFormat(const std::string &format) {
        auto encoding = EncodingFormat::UNKNOWN;
        if (format == "NONE") {
            encoding = EncodingFormat::NONE;
        } else if (format == "BASE64") {
            encoding = EncodingFormat::BASE64;
        } else if (format == "HEX") {
            encoding = EncodingFormat::HEX;
        }
        return encoding;
    }

    std::string AES::HexToString(const std::string &hex) {
        // Validate input length
        if (hex.empty() || hex.size() % 2 != 0)
            return "";

        // Pre-allocate result memory (avoid dynamic expansion)
        std::string result;
        result.resize(hex.size() / 2);

        // 256-byte static lookup table (O(1) time complexity)
        static constexpr uint8_t lookup[256] =
                {// 0-47: Invalid characters (default 0xFF)
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 // 48-57: '0'-'9' → 0-9
                 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                 // 58-64: Invalid characters
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 // 65-70: 'A'-'F' → 10-15
                 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                 // 71-96: Invalid characters
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 // 97-102: 'a'-'f' → 10-15
                 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                 // Remainder are invalid
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

        // Core conversion logic
        for (size_t i = 0; i < hex.size(); i += 2) {
            const uint8_t hi_val = lookup[static_cast<uint8_t>(hex[i])];
            const uint8_t lo_val = lookup[static_cast<uint8_t>(hex[i + 1])];

            // Invalid character detection (0xFF indicates invalid)
            if (hi_val == 0xFF || lo_val == 0xFF)
                return "";

            // Bitwise combination of high and low bits
            result[i / 2] = static_cast<char>((hi_val << 4) | lo_val);
        }
        return result;
    }


    bool AES::validateStreamingModePadding(AESMode mode,
                                           PaddingScheme padding) {
        const bool isStreamingMode =
                (mode == AESMode::OFB || mode == AESMode::CFB ||
                 mode == AESMode::XTS || mode == AESMode::EAX ||
                 mode == AESMode::GCM);
        return !(isStreamingMode && padding != PaddingScheme::NONE);
    }

    bool AES::initKey(CryptoPP::SecByteBlock &keyBlock,
                      const std::string &keyStr,
                      KeyBits keyBits,
                      AESMode mode) {
        int requiredLength = static_cast<int>(keyBits) / 8;

        // Special handling for XTS mode requiring double key length
        if (mode == AESMode::XTS) {
            requiredLength *= 2;
        }

        if (keyStr.empty()) {
            spdlog::error("Key string is empty");
            return false;
        }

        if (keyStr.size() != static_cast<size_t>(requiredLength)) {
            spdlog::error("Invalid key length: expected={}, actual={}",
                          requiredLength, keyStr.size());
            return false;
        }

        keyBlock =
                CryptoPP::SecByteBlock(reinterpret_cast<const CryptoPP::byte *>(
                                               keyStr.data()),
                                       requiredLength);
        return true;
    }

    bool AES::initIV(CryptoPP::SecByteBlock &ivBlock,
                     const std::string &ivStr,
                     AESMode mode,
                     bool isEncryption) {
        // ECB mode does not require IV
        if (mode == AESMode::ECB) {
            return true;
        }

        if (ivStr.empty()) {
            if (!isEncryption) {
                spdlog::error("IV required for decryption in {} mode",
                              AESModeToString(mode));
                return false;
            }

            // Automatically generate IV during encryption
            CryptoPP::AutoSeededRandomPool prng;

            // Special handling for CCM mode
            if (mode == AESMode::CCM) {
                ivBlock = CryptoPP::SecByteBlock(RECOMMENDED_CCM_IV_SIZE);
                spdlog::info("Generating recommended 12-byte IV for CCM mode");
            } else {
                ivBlock = CryptoPP::SecByteBlock(AES_BLOCK_SIZE);
            }

            prng.GenerateBlock(ivBlock, ivBlock.size());
        } else {
            // Special handling for CCM mode
            if (mode == AESMode::CCM) {
                if (ivStr.size() < CCM_MIN_IV_SIZE ||
                    ivStr.size() > CCM_MAX_IV_SIZE) {
                    spdlog::error("Invalid IV length for CCM mode: "
                                  "expected={}-{} bytes, actual={}",
                                  CCM_MIN_IV_SIZE, CCM_MAX_IV_SIZE,
                                  ivStr.size());
                    return false;
                }
                // Warn about non-recommended values even if valid
                if (ivStr.size() != RECOMMENDED_CCM_IV_SIZE) {
                    spdlog::warn("Using non-recommended IV length {} for CCM "
                                 "mode. Recommended is {} bytes.",
                                 ivStr.size(), RECOMMENDED_CCM_IV_SIZE);
                }
            }
            // Non-CCM modes maintain the original requirement
            else if (ivStr.size() != AES_BLOCK_SIZE) {
                spdlog::error(
                        "Invalid IV length for {} mode: expected=16, actual={}",
                        AESModeToString(mode), ivStr.size());
                return false;
            }

            // Convert input IV string to SecByteBlock
            ivBlock = CryptoPP::SecByteBlock(
                    reinterpret_cast<const CryptoPP::byte *>(ivStr.data()),
                    ivStr.size());
        }
        return true;
    }

    CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme
    AES::getPaddingScheme(PaddingScheme padding) {
        switch (padding) {
            case PaddingScheme::NONE:
                return CryptoPP::BlockPaddingSchemeDef::NO_PADDING;
            case PaddingScheme::ZEROS:
                return CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING;
            case PaddingScheme::PKCS7:
                return CryptoPP::BlockPaddingSchemeDef::PKCS_PADDING;
            case PaddingScheme::ONE_AND_ZEROS:
                return CryptoPP::BlockPaddingSchemeDef::ONE_AND_ZEROS_PADDING;
            case PaddingScheme::W3C:
                return CryptoPP::BlockPaddingSchemeDef::W3C_PADDING;
            case PaddingScheme::DEFAULT:
                return CryptoPP::BlockPaddingSchemeDef::DEFAULT_PADDING;
            default:
                return CryptoPP::BlockPaddingSchemeDef::NO_PADDING;
        }
    }

    std::string AES::processEncoding(const std::string &input,
                                     EncodingFormat encoding,
                                     bool decode) {
        if (encoding == EncodingFormat::BASE64) {
            if (decode) {
                std::string decoded;
                CryptoPP::Base64Decoder decoder;
                decoder.Put(reinterpret_cast<const CryptoPP::byte *>(
                                    input.data()),
                            input.size());
                decoder.MessageEnd();
                decoded.resize(decoder.MaxRetrievable());
                decoder.Get(reinterpret_cast<CryptoPP::byte *>(decoded.data()),
                            decoded.size());
                return decoded;
            } else {
                std::string encoded;
                CryptoPP::Base64Encoder encoder;
                encoder.Put(reinterpret_cast<const CryptoPP::byte *>(
                                    input.data()),
                            input.size());
                encoder.MessageEnd();
                encoded.resize(encoder.MaxRetrievable());
                encoder.Get(reinterpret_cast<CryptoPP::byte *>(encoded.data()),
                            encoded.size());
                return encoded;
            }
        } else if (encoding == EncodingFormat::HEX) {
            if (decode) {
                std::string decoded;
                CryptoPP::HexDecoder decoder;
                decoder.Put(reinterpret_cast<const CryptoPP::byte *>(
                                    input.data()),
                            input.size());
                decoder.MessageEnd();
                decoded.resize(decoder.MaxRetrievable());
                decoder.Get(reinterpret_cast<CryptoPP::byte *>(decoded.data()),
                            decoded.size());
                return decoded;
            } else {
                std::string encoded;
                CryptoPP::HexEncoder encoder;
                encoder.Put(reinterpret_cast<const CryptoPP::byte *>(
                                    input.data()),
                            input.size());
                encoder.MessageEnd();
                encoded.resize(encoder.MaxRetrievable());
                encoder.Get(reinterpret_cast<CryptoPP::byte *>(encoded.data()),
                            encoded.size());
                return encoded;
            }
        }
        return input;
    }

    AES::Result AES::Encrypt(const std::string &plaintext,
                             AESMode mode,
                             PaddingScheme padding,
                             KeyBits keyBits,
                             const std::string &key,
                             const std::string &iv,
                             EncodingFormat outputEncoding) {
        Result result;

        // Validate streaming mode padding requirements
        if (!validateStreamingModePadding(mode, padding)) {
            result.error = "Streaming modes require NO_PADDING";
            result.success = false;
            spdlog::error(result.error);
            return result;
        }

        try {
            CryptoPP::SecByteBlock keyBlock;
            if (!initKey(keyBlock, key, keyBits, mode)) {
                result.error = "Invalid key configuration";
                result.success = false;
                return result;
            }

            CryptoPP::SecByteBlock ivBlock;
            if (!initIV(ivBlock, iv, mode, true)) {
                result.error = "IV generation failed";
                result.success = false;
                return result;
            }

            auto cryptoPadding = getPaddingScheme(padding);
            std::string ciphertext;

            switch (mode) {
                case AESMode::ECB: {
                    CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption
                            encryptor(keyBlock, keyBlock.size());
                    CryptoPP::StringSource
                            ss(plaintext, true,
                               new CryptoPP::StreamTransformationFilter(
                                       encryptor,
                                       new CryptoPP::StringSink(ciphertext),
                                       cryptoPadding));
                    break;
                }
                case AESMode::CBC: {
                    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption
                            encryptor(keyBlock, keyBlock.size(), ivBlock);
                    CryptoPP::StringSource
                            ss(plaintext, true,
                               new CryptoPP::StreamTransformationFilter(
                                       encryptor,
                                       new CryptoPP::StringSink(ciphertext),
                                       cryptoPadding));
                    break;
                }
                case AESMode::OFB: {
                    CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption
                            encryptor(keyBlock, keyBlock.size(), ivBlock);
                    CryptoPP::StringSource
                            ss(plaintext, true,
                               new CryptoPP::StreamTransformationFilter(
                                       encryptor,
                                       new CryptoPP::StringSink(ciphertext),
                                       CryptoPP::BlockPaddingSchemeDef::
                                               NO_PADDING));
                    break;
                }
                case AESMode::CFB: {
                    CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption
                            encryptor(keyBlock, keyBlock.size(), ivBlock);
                    CryptoPP::StringSource
                            ss(plaintext, true,
                               new CryptoPP::StreamTransformationFilter(
                                       encryptor,
                                       new CryptoPP::StringSink(ciphertext),
                                       CryptoPP::BlockPaddingSchemeDef::
                                               NO_PADDING));
                    break;
                }
                case AESMode::XTS: {
                    CryptoPP::XTS_Mode<CryptoPP::AES>::Encryption encryptor;
                    encryptor.SetKeyWithIV(keyBlock, keyBlock.size(), ivBlock,
                                           ivBlock.size());
                    CryptoPP::StringSource
                            ss(plaintext, true,
                               new CryptoPP::StreamTransformationFilter(
                                       encryptor,
                                       new CryptoPP::StringSink(ciphertext),
                                       CryptoPP::BlockPaddingSchemeDef::
                                               NO_PADDING));
                    break;
                }
                case AESMode::CCM: {
                    CryptoPP::CCM<CryptoPP::AES>::Encryption encryptor;
                    encryptor.SetKeyWithIV(keyBlock, keyBlock.size(), ivBlock,
                                           ivBlock.size());

                    // Correct usage of SpecifyDataLengths (3 parameters)
                    encryptor.SpecifyDataLengths(0, plaintext.size(), 0);

                    // Set tag length in AuthenticatedEncryptionFilter
                    size_t tagSize =
                            12; // Authentication tag length (configurable)
                    CryptoPP::StringSource ss(
                            plaintext, true,
                            new CryptoPP::AuthenticatedEncryptionFilter(
                                    encryptor,
                                    new CryptoPP::StringSink(ciphertext),
                                    false,  // Do not append authentication tag,
                                            // (already included in CCM)
                                    tagSize // Specify authentication tag length
                                    ));
                    break;
                }
                case AESMode::EAX: {
                    CryptoPP::EAX<CryptoPP::AES>::Encryption encryptor;
                    encryptor.SetKeyWithIV(keyBlock, keyBlock.size(), ivBlock,
                                           ivBlock.size());
                    CryptoPP::StringSource
                            ss(plaintext, true,
                               new CryptoPP::AuthenticatedEncryptionFilter(
                                       encryptor,
                                       new CryptoPP::StringSink(ciphertext)));
                    break;
                }
                case AESMode::GCM: {
                    CryptoPP::GCM<CryptoPP::AES>::Encryption encryptor;
                    encryptor.SetKeyWithIV(keyBlock, keyBlock.size(), ivBlock,
                                           ivBlock.size());
                    CryptoPP::StringSource
                            ss(plaintext, true,
                               new CryptoPP::AuthenticatedEncryptionFilter(
                                       encryptor,
                                       new CryptoPP::StringSink(ciphertext)));
                    break;
                }
                default: {
                    result.error = "Unsupported encryption mode: " +
                            AESModeToString(mode);
                    result.success = false;
                    spdlog::error(result.error);
                    return result;
                }
            }

            if (outputEncoding != EncodingFormat::NONE) {
                ciphertext = processEncoding(ciphertext, outputEncoding, false);
            }

            result.data = ciphertext;
            result.success = true;
            return result;
        } catch (const CryptoPP::Exception &e) {
            result.error = "Crypto++ exception: " + std::string(e.what());
            result.success = false;
            spdlog::error(result.error);
            return result;
        } catch (const std::exception &e) {
            result.error = "Standard exception: " + std::string(e.what());
            result.success = false;
            spdlog::error(result.error);
            return result;
        }
    }

    AES::Result AES::Decrypt(const std::string &ciphertext,
                             AESMode mode,
                             PaddingScheme padding,
                             KeyBits keyBits,
                             const std::string &key,
                             const std::string &iv,
                             EncodingFormat inputEncoding) {
        Result result;

        // Validate streaming mode padding requirements
        if (!validateStreamingModePadding(mode, padding)) {
            result.error = "Streaming modes require NO_PADDING";
            result.success = false;
            spdlog::error(result.error);
            return result;
        }

        try {
            std::string processedCiphertext =
                    (inputEncoding == EncodingFormat::NONE)
                    ? ciphertext
                    : processEncoding(ciphertext, inputEncoding, true);

            CryptoPP::SecByteBlock keyBlock;
            if (!initKey(keyBlock, key, keyBits, mode)) {
                result.error = "Invalid key configuration";
                result.success = false;
                return result;
            }

            CryptoPP::SecByteBlock ivBlock;
            if (!initIV(ivBlock, iv, mode, false)) {
                result.error = "Invalid IV for decryption";
                result.success = false;
                return result;
            }

            auto cryptoPadding = getPaddingScheme(padding);
            std::string plaintext;

            switch (mode) {
                case AESMode::ECB: {
                    CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption
                            decryptor(keyBlock, keyBlock.size());
                    CryptoPP::StringSource
                            ss(processedCiphertext, true,
                               new CryptoPP::StreamTransformationFilter(
                                       decryptor,
                                       new CryptoPP::StringSink(plaintext),
                                       cryptoPadding));
                    break;
                }
                case AESMode::CBC: {
                    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption
                            decryptor(keyBlock, keyBlock.size(), ivBlock);
                    CryptoPP::StringSource
                            ss(processedCiphertext, true,
                               new CryptoPP::StreamTransformationFilter(
                                       decryptor,
                                       new CryptoPP::StringSink(plaintext),
                                       cryptoPadding));
                    break;
                }
                case AESMode::OFB: {
                    CryptoPP::OFB_Mode<CryptoPP::AES>::Decryption
                            decryptor(keyBlock, keyBlock.size(), ivBlock);
                    CryptoPP::StringSource
                            ss(processedCiphertext, true,
                               new CryptoPP::StreamTransformationFilter(
                                       decryptor,
                                       new CryptoPP::StringSink(plaintext),
                                       CryptoPP::BlockPaddingSchemeDef::
                                               NO_PADDING));
                    break;
                }
                case AESMode::CFB: {
                    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption
                            decryptor(keyBlock, keyBlock.size(), ivBlock);
                    CryptoPP::StringSource
                            ss(processedCiphertext, true,
                               new CryptoPP::StreamTransformationFilter(
                                       decryptor,
                                       new CryptoPP::StringSink(plaintext),
                                       CryptoPP::BlockPaddingSchemeDef::
                                               NO_PADDING));
                    break;
                }
                case AESMode::XTS: {
                    CryptoPP::XTS_Mode<CryptoPP::AES>::Decryption decryptor;
                    decryptor.SetKeyWithIV(keyBlock, keyBlock.size(), ivBlock,
                                           ivBlock.size());
                    CryptoPP::StringSource
                            ss(processedCiphertext, true,
                               new CryptoPP::StreamTransformationFilter(
                                       decryptor,
                                       new CryptoPP::StringSink(plaintext),
                                       CryptoPP::BlockPaddingSchemeDef::
                                               NO_PADDING));
                    break;
                }
                case AESMode::CCM: {
                    CryptoPP::CCM<CryptoPP::AES>::Decryption decryptor;
                    decryptor.SetKeyWithIV(keyBlock, keyBlock.size(), ivBlock,
                                           ivBlock.size());

                    // Correct usage of SpecifyDataLengths (3 parameters)
                    size_t tagSize = 12; // Must match encryption value
                    size_t messageLength = processedCiphertext.size() - tagSize;
                    decryptor.SpecifyDataLengths(0, messageLength, 0);

                    CryptoPP::StringSource ss(
                            processedCiphertext, true,
                            new CryptoPP::AuthenticatedDecryptionFilter(
                                    decryptor,
                                    new CryptoPP::StringSink(plaintext),
                                    CryptoPP::AuthenticatedDecryptionFilter::
                                            DEFAULT_FLAGS,
                                    tagSize // Specify authentication tag length
                                    ));
                    break;
                }
                case AESMode::EAX: {
                    CryptoPP::EAX<CryptoPP::AES>::Decryption decryptor;
                    decryptor.SetKeyWithIV(keyBlock, keyBlock.size(), ivBlock,
                                           ivBlock.size());
                    CryptoPP::StringSource
                            ss(processedCiphertext, true,
                               new CryptoPP::AuthenticatedDecryptionFilter(
                                       decryptor,
                                       new CryptoPP::StringSink(plaintext)));
                    break;
                }
                case AESMode::GCM: {
                    CryptoPP::GCM<CryptoPP::AES>::Decryption decryptor;
                    decryptor.SetKeyWithIV(keyBlock, keyBlock.size(), ivBlock,
                                           ivBlock.size());
                    CryptoPP::StringSource
                            ss(processedCiphertext, true,
                               new CryptoPP::AuthenticatedDecryptionFilter(
                                       decryptor,
                                       new CryptoPP::StringSink(plaintext)));
                    break;
                }
                default: {
                    result.error = "Unsupported decryption mode: " +
                            AESModeToString(mode);
                    result.success = false;
                    spdlog::error(result.error);
                    return result;
                }
            }

            result.data = plaintext;
            result.success = true;
            return result;
        } catch (const CryptoPP::Exception &e) {
            result.error = "Crypto++ exception: " + std::string(e.what());
            result.success = false;
            spdlog::error(result.error);
            return result;
        } catch (const std::exception &e) {
            result.error = "Standard exception: " + std::string(e.what());
            result.success = false;
            spdlog::error(result.error);
            return result;
        }
    }

} // namespace CryptoToysPP::Algorithm::Advance
