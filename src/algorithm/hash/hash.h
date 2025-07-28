/* clang-format off */
/*
 * @file hash.h
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
#ifndef HASH_H
#define HASH_H
#include <string>
namespace CryptoToysPP::Algorithm::Hash {
    std::string MD2(const std::string &data);

    std::string MD4(const std::string &data);

    std::string MD5(const std::string &data);

    std::string SHA1(const std::string &data);

    std::string SHA224(const std::string &data);

    std::string SHA256(const std::string &data);

    std::string SHA384(const std::string &data);

    std::string SHA512(const std::string &data);

    std::string SHA3_224(const std::string &data);

    std::string SHA3_256(const std::string &data);

    std::string SHA3_384(const std::string &data);

    std::string SHA3_512(const std::string &data);
} // namespace CryptoToysPP::Algorithm::Hash

#endif // HASH_H
