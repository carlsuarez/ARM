#ifndef MATH_H
#define MATH_H

#include <stdint.h>

static inline int32_t abs(int32_t a)
{
    if (a < 0)
        return -a;
    return a;
}

static inline double math_ceil(double x)
{
    // Handle special cases
    if (x != x)
        return x; // NaN
    if (x == 0.0 || x == -0.0)
        return x; // Zero
    if (x > 0x1p52 || x < -0x1p52)
        return x; // Already integer (too large)

    // Cast to integer (truncates towards zero)
    long long int_part = (long long)x;

    // If x is negative or already an integer, return the integer part
    if (x <= 0.0 || x == (double)int_part)
    {
        return (double)int_part;
    }

    // For positive non-integers, add 1
    return (double)(int_part + 1);
}

#endif