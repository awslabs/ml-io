/*
 * Based on the cpp-HyperLogLog library:
 * https://github.com/hideo55/cpp-HyperLogLog
 *
 * Copyright (c) 2013 Hideaki Ohno <hide.o.j55{at}gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the 'Software'),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wextra-semi-stmt"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
#pragma GCC diagnostic ignored "-Wreserved-id-macro"
#define XXH_INLINE_ALL
#include "xxhash.h"

#if defined(__has_builtin) && (defined(__GNUC__) || defined(__clang__))

#define _GET_CLZ(x, b) (uint8_t) std::min(b, ::__builtin_clz(x)) + 1

#else

inline uint8_t _get_leading_zero_count(uint32_t x, uint8_t b)
{

#if defined(_MSC_VER)
    uint32_t leading_zero_len = 32;
    ::_BitScanReverse(&leading_zero_len, x);
    --leading_zero_len;
    return std::min(b, (uint8_t) leading_zero_len);
#else
    uint8_t v = 1;
    while (v <= b && !(x & 0x80000000)) {
        v++;
        x <<= 1;
    }
    return v;
#endif
}
#define _GET_CLZ(x, b) _get_leading_zero_count(x, b)
#endif /* defined(__GNUC__) */
#pragma GCC diagnostic pop

#define HLL_HASH_SEED 0

namespace hll {

static const double pow_2_32 = 4294967296.0;       ///< 2^32
static const double neg_pow_2_32 = -4294967296.0;  ///< -(2^32)

class HyperLogLog {
public:
    HyperLogLog(uint8_t b = 4);

    /// Adds a string-value to the estimator.
    void add(const std::string_view &str);

    /// Estimates the cardinality.
    double estimate() const;

    /// Merges two HyperLogLog estimators together.
    void merge(const HyperLogLog &other);

    /// Resets the estimator.
    void clear();

    // Returns the size of the estimator.
    uint32_t register_size() const;

protected:
    /// register bit width
    uint8_t b_;
    /// register size
    uint32_t m_;
    /// alpha * m^2
    double alpha_mm_;
    /// registers
    std::vector<uint8_t> M_;
};

}  // namespace hll
