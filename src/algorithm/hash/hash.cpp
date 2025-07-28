/* clang-format off */
/*
 * @file hash.cpp
 * @date 2025-07-25
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
#include "hash.h"
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md2.h>
#include <cryptopp/md4.h>
#include <cryptopp/md5.h>
#include <cryptopp/sha.h>
#include <cryptopp/sha3.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
namespace CryptoToysPP::Algorithm::Hash {
    std::string MD2(const std::string &data) {
        CryptoPP::Weak1::MD2 hash;
        std::string digest;
        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }

    std::string MD4(const std::string &data) {
        CryptoPP::Weak1::MD4 hash;
        std::string digest;
        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }

    std::string MD5(const std::string &data) {
        CryptoPP::Weak1::MD5 hash;
        std::string digest;
        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }

    std::string SHA1(const std::string &data) {
        CryptoPP::SHA1 hash;
        std::string digest;

        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }

    std::string SHA224(const std::string &data) {
        CryptoPP::SHA224 hash;
        std::string digest;

        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }

    std::string SHA256(const std::string &data) {
        CryptoPP::SHA256 hash;
        std::string digest;

        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }

    std::string SHA384(const std::string &data) {
        CryptoPP::SHA384 hash;
        std::string digest;

        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }

    std::string SHA512(const std::string &data) {
        CryptoPP::SHA512 hash;
        std::string digest;

        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }

    std::string SHA3_224(const std::string &data) {
        CryptoPP::SHA3_224 hash;
        std::string digest;

        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }

    std::string SHA3_256(const std::string &data) {
        CryptoPP::SHA3_256 hash;
        std::string digest;

        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }

    std::string SHA3_384(const std::string &data) {
        CryptoPP::SHA3_384 hash;
        std::string digest;

        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }

    std::string SHA3_512(const std::string &data) {
        CryptoPP::SHA3_512 hash;
        std::string digest;

        CryptoPP::StringSource source(
                data, true,
                new CryptoPP::HashFilter(
                        hash,
                        new CryptoPP::HexEncoder(new CryptoPP::StringSink(
                                                         digest),
                                                 false // lowercase hex output
                                                 )));
        return digest;
    }
} // namespace CryptoToysPP::Algorithm::Hash
