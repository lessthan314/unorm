// Copyright (c) 2024 Your Name
// 
// Licensed under the MIT License. See LICENSE file in the project root for details.

#ifndef H_UNORM_H
#define H_UNORM_H

#include <cstdint>
#include <stddef.h>
#include <limits>
#include <type_traits>
#include <iostream>
#include <float.h>
#include <cmath>
#include <cstring>
#include <bitset>

inline constexpr uint8_t __bit_reverse(uint8_t n)
{
#if defined(__has_builtin) && __has_builtin(__builtin_bitreverse8)
    return __builtin_bitreverse8(n);
#else
    n = ((n & 0xAA) >> 1) | ((n & 0x55) << 1);
    n = ((n & 0xCC) >> 2) | ((n & 0x33) << 2);
    n = (n >> 4) | (n << 4);
    return n;
#endif
}

inline constexpr uint16_t __bit_reverse(uint16_t n)
{
#if defined(__has_builtin) && __has_builtin(__builtin_bitreverse16)
    return __builtin_bitreverse16(n);
#else
    n = ((n & 0xAAAA) >> 1) | ((n & 0x5555) << 1);
    n = ((n & 0xCCCC) >> 2) | ((n & 0x3333) << 2);
    n = ((n & 0xF0F0) >> 4) | ((n & 0x0F0F) << 4);
    n = (n >> 8) | (n << 8);
    return n;
#endif
}

inline constexpr uint32_t __bit_reverse(uint32_t n)
{
#if defined(__has_builtin) && __has_builtin(__builtin_bitreverse32)
    return __builtin_bitreverse32(n);
#else
    n = ((n & 0xAAAAAAAA) >> 1) | ((n & 0x55555555) << 1);
    n = ((n & 0xCCCCCCCC) >> 2) | ((n & 0x33333333) << 2);
    n = ((n & 0xF0F0F0F0) >> 4) | ((n & 0x0F0F0F0F) << 4);
    n = ((n & 0xFF00FF00) >> 8) | ((n & 0x00FF00FF) << 8);
    n = (n >> 16) | (n << 16);
    return n;
#endif
}

inline constexpr uint64_t __bit_reverse(uint64_t n)
{
#if defined(__has_builtin) && __has_builtin(__builtin_bitreverse64)
    return __builtin_bitreverse64(n);
#else
    n = ((n & 0xAAAAAAAAAAAAAAAA) >> 1) | ((n & 0x5555555555555555) << 1);
    n = ((n & 0xCCCCCCCCCCCCCCCC) >> 2) | ((n & 0x3333333333333333) << 2);
    n = ((n & 0xF0F0F0F0F0F0F0F0) >> 4) | ((n & 0x0F0F0F0F0F0F0F0F) << 4);
    n = ((n & 0xFF00FF00FF00FF00) >> 8) | ((n & 0x00FF00FF00FF00FF) << 8);
    n = ((n & 0xFFFF0000FFFF0000) >> 16) | ((n & 0x0000FFFF0000FFFF) << 16);
    n = (n >> 32) | (n << 32);
    return n;
#endif
}

template <typename FLOAT>
constexpr size_t __mantissa()
{
    if constexpr (std::is_same_v<FLOAT, float>)
        return FLT_MANT_DIG;
    if constexpr (std::is_same_v<FLOAT, double>)
        return DBL_MANT_DIG;
    if constexpr (std::is_same_v<FLOAT, long double>)
        return LDBL_MANT_DIG;
}

template <size_t Bits>
struct uintN
{
    static_assert(Bits <= 64, "Only 64 bit values are supported so far.");
    using type = std::make_unsigned_t<
        std::conditional_t<(Bits <= 8), uint8_t,
            std::conditional_t<(Bits <= 16), uint16_t,
                std::conditional_t<(Bits <= 32), uint32_t,
                    uint64_t>>>>;
};
template <size_t Bits>
using uintN_t = typename uintN<Bits>::type;

template <size_t PRECISION = 32, bool REVERSED = false>
class unorm
{
private:
public:
    using value_type = uintN_t<PRECISION>;
    static constexpr unorm MAX()
    {
        unorm out;
        out.val = ~(value_type)0;
        return out;
    }
    static constexpr unorm MIN()
    {
        unorm out;
        out.val = 0;
        return out;
    };

    unorm() = default;
    ~unorm() = default;

    template <typename FLOAT>
    constexpr unorm(FLOAT x) : val(0)
    {
        // Get exponent and mantissa.
        int exponent;
        FLOAT mantissa = std::frexp(x, &exponent);

        // Store mantissa in a bitset for bit manipulation.
        std::bitset<std::max(sizeof(FLOAT) * 8, PRECISION)> bits;
        std::memcpy(&bits, &mantissa, sizeof(FLOAT));

        // Move mantissa, such that it's significant part is stored in the first PRECISION bytes.
        if constexpr (PRECISION > __mantissa<FLOAT>())
            bits <<= PRECISION - __mantissa<FLOAT>();
        else
            bits >>= __mantissa<FLOAT>() - PRECISION;
        // Add implicit 1.
        bits[PRECISION - 1] = 1;

        // Copy mantissa into the value.
        std::memcpy(&val, &bits, std::min(sizeof(bits), sizeof(val)));

        // Move mantissa based on the exponent.
        if (--exponent < 0)     // The '--' is needed due to the implicit 1.
            val >>= -exponent;
        else
            val <<= exponent;

        // If data is stored in reverse, reverse it.
        if constexpr (REVERSED)
            val = __bit_reverse(val);
    }

    inline unorm& operator*=(const unorm& other)
    {
        if constexpr (!REVERSED)
            val = __bit_reverse(__bit_reverse(val) * __bit_reverse(other.val));
        else
            val *= other.val;
        return *this;
    }
    inline unorm constexpr operator*(const unorm& other) const
    {
        unorm out = *this;
        out *= other;
        return out;
    }

    inline unorm& operator/=(const unorm& other)
    {
        if constexpr (!REVERSED)
            val = __bit_reverse(__bit_reverse(val) / __bit_reverse(other.val));
        else
            val /= other.val;
        return *this;
    }
    inline unorm constexpr operator/(const unorm& other) const
    {
        unorm out = *this;
        out /= other;
        return out;
    }

    inline unorm& operator+=(const unorm& other)
    {
        if constexpr (REVERSED)
            val = __bit_reverse(__bit_reverse(__bit_reverse(val) + __bit_reverse(other.val)));
        else
            val += other.val;
        return *this;
    }
    inline unorm constexpr operator+(const unorm& other) const
    {
        unorm out = *this;
        out += other;
        return out;
    }

    inline unorm& operator-=(const unorm& other)
    {
        if constexpr (REVERSED)
            val = __bit_reverse(__bit_reverse(__bit_reverse(val) - __bit_reverse(other.val)));
        else
            val += other.val;
        return *this;
    }
    inline unorm constexpr operator-(const unorm& other) const
    {
        unorm out = *this;
        out -= other;
        return out;
    }

    template <size_t N, bool B>
    friend std::ostream &operator<<(std::ostream &os, unorm<N, B> u);

    value_type bits() { return val; }

private:
    value_type val;
    static_assert(sizeof(value_type) * 8 >= PRECISION, "An unexpected error occured. Please contact the manufacturer.");
};

template <size_t PRECISION = 32>
using mnorm = unorm<PRECISION, true>;
template <size_t PRECISION = 32>
using anorm = unorm<PRECISION, false>;

template <size_t N, bool B>
std::ostream &operator<<(std::ostream &os, unorm<N, B> u)
{
    if constexpr (!B)
        u.val = __bit_reverse(u.val);
    os << (u.val & uintN_t<N>(1)) ? '1' : '0';
    os << '.';
    u.val >>= 1;

    if (u.val != 0)
    {
        char cs[N];
        std::fill_n(cs, N, '0');
        size_t digits = 5;
        size_t index = 0;
        while (true)
        {
            if (u.val & 1)
            {
                size_t digits_copy = digits;
                size_t index_copy = index;
                while (digits_copy)
                {
                    cs[index_copy] += digits_copy % 10;
                    size_t index_copy_copy = index_copy;
                    while (cs[index_copy_copy] > '9')
                    {
                        cs[index_copy_copy] -= 10;
                        cs[--index_copy_copy]++;
                    }
                    digits_copy /= 10;
                    index_copy--;
                }
            }

            u.val >>= 1;
            if (u.val == 0)
                break;
            digits *= 5;
            index++;
        }
        cs[index + 1] = '\0';
        os << cs;
    }
    os << "un";
    return os;
}

#endif