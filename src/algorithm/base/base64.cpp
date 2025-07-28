/* clang-format off */
/*
 * @file base64.cpp
 * @date 2025-04-09
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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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
#include "base64.h"
#include <algorithm>
namespace CryptoToysPP::Algorithm::Base {
    std::vector<uint8_t> Base64::Encode(const uint8_t *data, const size_t len) {
        if (len <= 0)
            return {};
        constexpr uint8_t BASE64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghi"
                                           "jklmnopqrstuvwxyz0123456789+/";
        std::vector<uint8_t> encoded;
        encoded.reserve(4 * ((len + 2) / 3));

        size_t i = 0;
        while (i < len) {
            uint32_t triple = 0;
            int32_t valid_bytes = 0;

            for (int32_t j = 0; j < 3; ++j, ++i) {
                triple <<= 8;
                if (i < len) {
                    triple |= data[i];
                    valid_bytes++;
                }
            }

            const int32_t sextets = (valid_bytes * 8 + 5) / 6;
            for (int32_t j = 0; j < 4; ++j) {
                const uint8_t index = (triple >> (18 - j * 6)) & 0x3F;
                if (j < sextets) {
                    encoded.push_back(BASE64_CHARS[index]);
                } else {
                    encoded.push_back('=');
                }
            }
        }
        return encoded;
    }

    std::vector<uint8_t> Base64::Decode(const uint8_t *data, const size_t len) {
        if (len <= 0 || len % 4 != 0)
            return {};

        constexpr uint8_t BASE64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghi"
                                           "jklmnopqrstuvwxyz0123456789+/";

        std::vector decode_table(256, -1);
        for (int32_t i = 0; i < 64; ++i) {
            decode_table[BASE64_CHARS[i]] = i;
        }
        decode_table['='] = -1;

        std::vector<uint8_t> decoded;
        decoded.reserve((len * 3) / 4);

        uint32_t buffer = 0;
        int32_t bits_collected = 0;
        int32_t padding = 0;

        for (size_t i = 0; i < len; ++i) {
            const uint8_t c = data[i];
            if (c == '=') {
                padding++;
                if (i < len - 2 || padding > 2)
                    return {};
                continue;
            }
            if (decode_table[c] == -1)
                return {};

            buffer = (buffer << 6) | decode_table[c];
            bits_collected += 6;

            if (bits_collected >= 8) {
                decoded.push_back((buffer >> (bits_collected - 8)) & 0xFF);
                bits_collected -= 8;
            }
        }
        if (padding > 0) {
            const size_t expected_size = (len - padding) * 3 / 4;
            if (decoded.size() < expected_size)
                return {};
            decoded.resize(expected_size);
        }
        return decoded;
    }

    std::vector<uint8_t> Base64::Encode(const std::vector<uint8_t> &data) {
        return Encode(data.data(), data.size());
    }

    std::vector<uint8_t> Base64::Decode(const std::vector<uint8_t> &data) {
        return Decode(data.data(), data.size());
    }

    std::string Base64::Encode(const std::string &data) {
        const std::vector<uint8_t> input(data.begin(), data.end());
        std::vector<uint8_t> encoded = Encode(input);
        std::string result(encoded.begin(), encoded.end());
        return result;
    }

    std::string Base64::Decode(const std::string &data) {
        const std::vector<uint8_t> input(data.begin(), data.end());
        std::vector<uint8_t> decoded = Decode(input);
        std::string result(decoded.begin(), decoded.end());
        return result;
    }

    std::vector<uint8_t> Base64::EncodeURL(const uint8_t *data,
                                           const size_t len) {
        if (len <= 0)
            return {};
        constexpr uint8_t BASE64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghi"
                                           "jklmnopqrstuvwxyz0123456789+/";
        std::vector<uint8_t> encoded;
        encoded.reserve(4 * ((len + 2) / 3));

        size_t i = 0;
        while (i < len) {
            uint32_t triple = 0;
            int32_t valid_bytes = 0;

            for (int32_t j = 0; j < 3; ++j, ++i) {
                triple <<= 8;
                if (i < len) {
                    triple |= data[i];
                    valid_bytes++;
                }
            }

            const int32_t sextets = (valid_bytes * 8 + 5) / 6;
            for (int32_t j = 0; j < 4; ++j) {
                const uint8_t index = (triple >> (18 - j * 6)) & 0x3F;
                if (j < sextets) {
                    encoded.push_back(BASE64_CHARS[index]);
                } else {
                    encoded.push_back('=');
                }
            }
        }

        for (auto &c : encoded) {
            if (c == '+')
                c = '-';
            if (c == '/')
                c = '_';
        }
        while (!encoded.empty() && encoded.back() == '=')
            encoded.pop_back();

        return encoded;
    }

    std::vector<uint8_t> Base64::DecodeURL(const uint8_t *data,
                                           const size_t len) {
        if (len <= 0)
            return {};

        std::vector modified_data(data, data + len);
        std::ranges::transform(modified_data, modified_data.begin(),
                               [](const uint8_t c) -> uint8_t {
                                   if (c == '-')
                                       return '+';
                                   if (c == '_')
                                       return '/';
                                   return c;
                               });

        const size_t padding = (4 - (modified_data.size() % 4)) % 4;
        modified_data.insert(modified_data.end(), padding, '=');

        constexpr uint8_t BASE64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghi"
                                           "jklmnopqrstuvwxyz0123456789+/";
        std::vector decode_table(256, -1);
        for (int32_t i = 0; i < 64; ++i) {
            decode_table[BASE64_CHARS[i]] = i;
        }

        std::vector<uint8_t> decoded;
        decoded.reserve((modified_data.size() * 3) / 4);

        uint32_t buffer = 0;
        int32_t bits_collected = 0;
        bool padding_started = false;

        for (size_t i = 0; i < modified_data.size(); ++i) {
            const uint8_t c = modified_data[i];
            if (c == '=') {
                if (i < modified_data.size() - padding)
                    return {};
                padding_started = true;
                continue;
            }

            if (decode_table[c] == -1)
                return {};
            if (padding_started)
                return {};

            buffer = (buffer << 6) | decode_table[c];
            bits_collected += 6;

            if (bits_collected >= 8) {
                decoded.push_back(static_cast<uint8_t>(
                        (buffer >> (bits_collected - 8)) & 0xFF));
                bits_collected -= 8;
            }
        }

        if (padding > 0) {
            const size_t expected_size =
                    (modified_data.size() - padding) * 3 / 4;
            if (decoded.size() < expected_size)
                return {};
            decoded.resize(expected_size);
        }

        return decoded;
    }

    std::vector<uint8_t> Base64::EncodeURL(const std::vector<uint8_t> &data) {
        return EncodeURL(data.data(), data.size());
    }

    std::vector<uint8_t> Base64::DecodeURL(const std::vector<uint8_t> &data) {
        return DecodeURL(data.data(), data.size());
    }

    std::string Base64::EncodeURL(const std::string &data) {
        const std::vector<uint8_t> input(data.begin(), data.end());
        std::vector<uint8_t> encoded = EncodeURL(input);
        std::string result(encoded.begin(), encoded.end());
        return result;
    }

    std::string Base64::DecodeURL(const std::string &data) {
        const std::vector<uint8_t> input(data.begin(), data.end());
        std::vector<uint8_t> decoded = DecodeURL(input);
        std::string result(decoded.begin(), decoded.end());
        return result;
    }
} // namespace CryptoToysPP::Algorithm::Base
