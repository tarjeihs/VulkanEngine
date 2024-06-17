#pragma once

#include "MathTypes.h"

#define GLM_ENABLE_EXPERIMENTAL

namespace Math
{
    template<typename TBase>
    static TBase Clamp(TBase Value, TBase Min, TBase Max)
    {
        if (Value < Min) return Min;
        if (Value > Max) return Max;
        return Value;
    }

    template<typename TBase>
    static TBase Min(TBase A, TBase B)
    {
        if (A < B)
        {
            return A;
        }

        return B;
    }

    template<typename TBase>
    static TBase Max(TBase A, TBase B)
    {
        if (A > B)
        {
            return A;
        }
        return B;
    }

    template<typename TBase>
    static TBase Abs(TBase Value)
    {
        if (Value < 0)
        {
            return -1 * Value;
        }

        return Value;
    }

    template<typename TBase>
    static TBase Inverse(TBase Value)
    {
        return -1 * Value;
    }

    /* https://en.wikipedia.org/wiki/Linear_interpolation */
    template<typename TBase>
    static TBase Lerp(TBase A, TBase B, float Alpha)
    {
        return A + Alpha * (B - A);
    }
    
    /* https://en.wikipedia.org/wiki/Exponentiation_by_squaring */
    template<typename TNumeric>
    static TNumeric Pow(TNumeric Base, int32 Exponent)
    {
        TNumeric Result = 1;
        const bool bIsNegative = Exponent < 0;
        if (bIsNegative) Exponent = -Exponent;

        while (Exponent)
        {
            if (Exponent & 1)
            {
                Result *= Base;
            }
            Base *= Base;
            Exponent >>= 1;
        }

        return bIsNegative ? 1 / Result : Result;
    }

    /* https://en.wikipedia.org/wiki/Fast_inverse_square_root */
    template<typename TNumeric>
    static TNumeric Sqrt(TNumeric Value)
    {
        int32 I;
        float X2, Y;
        constexpr float ThreeHalfs = 1.5f;
        X2 = Value * 0.5f;
        Y = Value;
        I = *reinterpret_cast<int32_t*>(&Y);          // evil floating point bit level hacking
        I = 0x5f3759df - (I >> 1);                    // what the fuck? 
        Y = *reinterpret_cast<float*>(&I);
        Y = Y * (ThreeHalfs - (X2 * Y * Y));          // 1st iteration
        // Y = Y * (ThreeHalfs - (X2 * Y * Y));       // 2nd iteration, this can be removed for better performance
    
        return 1.0f / Y;
    }
}
