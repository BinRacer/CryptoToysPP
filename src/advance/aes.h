/* clang-format off */
/*
 * @file aes.h
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
#ifndef AES_H
#define AES_H
#include <string>
#include <cryptopp/filters.h>
namespace CryptoToysPP::Advance {
    class AES {
    public:
        enum class AESMode { ECB, CBC, OFB, CFB, XTS, CCM, EAX, GCM, UNKNOWN };
        enum class PaddingScheme {
            NONE,
            ZEROS,
            PKCS7,
            ONE_AND_ZEROS,
            W3C,
            DEFAULT,
            UNKNOWN
        };
        enum class KeyBits {
            BITS_128 = 128,
            BITS_192 = 192,
            BITS_256 = 256,
            BITS_UNKNOWN = -1
        };
        enum class EncodingFormat { NONE, BASE64, HEX, UNKNOWN };

        struct Result {
            std::string data;
            bool success;
            std::string error;
        };

        static std::string AESModeToString(AESMode mode);

        static AESMode StringToAESMode(const std::string &mode);

        static std::string PaddingSchemeToString(PaddingScheme padding);

        static PaddingScheme StringToPaddingScheme(const std::string &padding);

        static KeyBits IntToKeyBits(int bits);

        static EncodingFormat StringToEncodingFormat(const std::string &format);

        static std::string HexToString(const std::string &hex);

        static Result Encode(const std::string &plaintext,
                             AESMode mode,
                             PaddingScheme padding,
                             KeyBits keyBits,
                             const std::string &key,
                             const std::string &iv,
                             EncodingFormat outputEncoding);

        static Result Decode(const std::string &ciphertext,
                             AESMode mode,
                             PaddingScheme padding,
                             KeyBits keyBits,
                             const std::string &key,
                             const std::string &iv,
                             EncodingFormat inputEncoding);

    private:
        static bool initKey(CryptoPP::SecByteBlock &keyBlock,
                            const std::string &keyStr,
                            KeyBits keyBits,
                            AESMode mode);

        static bool initIV(CryptoPP::SecByteBlock &ivBlock,
                           const std::string &ivStr,
                           AESMode mode,
                           bool isEncryption);

        static CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme
        getPaddingScheme(PaddingScheme padding);

        static std::string processEncoding(const std::string &input,
                                           EncodingFormat encoding,
                                           bool decode);

        static bool validateStreamingModePadding(AESMode mode,
                                                 PaddingScheme padding);
    };
} // namespace CryptoToysPP::Advance

#endif // AES_H
