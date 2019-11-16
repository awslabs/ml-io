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

#include "hyperloglog.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace hll {

HyperLogLog::HyperLogLog(uint8_t b) : b_(b), m_(1 << b), M_(m_, 0)
{
    if (b < 4 || 30 < b) {
        throw std::invalid_argument("bit width must be in the range [4,30]");
    }

    double alpha;
    switch (m_) {
    case 16:
        alpha = 0.673;
        break;
    case 32:
        alpha = 0.697;
        break;
    case 64:
        alpha = 0.709;
        break;
    default:
        alpha = 0.7213 / (1.0 + 1.079 / m_);
        break;
    }
    alpha_mm_ = alpha * m_ * m_;
}

void
HyperLogLog::add(std::string_view const &str)
{
    // Hash the value to add.
    uint32_t hash;
    hash = XXH32(str.begin(), str.size(), HLL_HASH_SEED);
    // Determine the register that this belongs to.
    // (e.g. use the first b_ bits as an index).
    uint32_t index = hash >> (32 - b_);
    // Get the number of leading zeros.
    uint8_t rank = _GET_CLZ((hash << b_), 32 - b_);
    // Update the register if there are more leading zeros.
    if (rank > M_[index]) {
        M_[index] = rank;
    }
}

double
HyperLogLog::estimate() const
{
    double estimate;
    double sum = 0.0;
    for (uint32_t i = 0; i < m_; i++) {
        sum += 1.0 / (1 << M_[i]);
    }
    estimate = alpha_mm_ / sum;  // E in the original paper
    if (estimate <= 2.5 * m_) {
        uint32_t zeros = 0;
        for (uint32_t i = 0; i < m_; i++) {
            if (M_[i] == 0) {
                zeros++;
            }
        }
        if (zeros != 0) {
            estimate = m_ * std::log(static_cast<double>(m_) / zeros);
        }
    }
    else if (estimate > (1.0 / 30.0) * pow_2_32) {
        estimate = neg_pow_2_32 * log(1.0 - (estimate / pow_2_32));
    }
    return estimate;
}

void
HyperLogLog::merge(const HyperLogLog &other)
{
    if (m_ != other.m_) {
        std::stringstream ss;
        ss << "number of registers doesn't match: " << m_
           << " != " << other.m_;
        throw std::invalid_argument(ss.str().c_str());
    }
    for (uint32_t r = 0; r < m_; ++r) {
        if (M_[r] < other.M_[r]) {
            M_[r] |= other.M_[r];
        }
    }
}

void
HyperLogLog::clear()
{
    std::fill(M_.begin(), M_.end(), 0);
}

uint32_t
HyperLogLog::register_size() const
{
    return m_;
}

}  // namespace hll
