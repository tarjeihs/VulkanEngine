#pragma once

#include "Math.h"

template<typename T>
struct TVector3
{
    T X;

    T Y;

    T Z;

    TVector3();

    TVector3(T Scalar);

    TVector3(T InX, T InY, T InZ);

    TVector3(const TVector3<T>& Other);

    inline TVector3<T> operator+(T Other);

    inline TVector3<T> operator-(T Other);

    inline TVector3<T> operator*(T Other);

    inline TVector3<T> operator/(T Other);

    inline TVector3<T> operator+(const TVector3<T>& Other) const;

    inline TVector3<T> operator-(const TVector3<T>& Other) const;

    inline TVector3<T> operator*(const TVector3<T>& Other) const;

    inline TVector3<T> operator/(const TVector3<T>& Other) const;

    inline TVector3<T>& operator+=(T Other);

    inline TVector3<T>& operator-=(T Other);

    inline TVector3<T>& operator*=(T Other);

    inline TVector3<T>& operator/=(T Other);
    
    inline TVector3<T>& operator+=(const TVector3<T>& Other);

    inline TVector3<T>& operator-=(const TVector3<T>& Other);

    inline TVector3<T>& operator*=(const TVector3<T>& Other);

    inline TVector3<T>& operator/=(const TVector3<T>& Other);

    inline TVector3<T> Zero() const;

    inline TVector3<T> One() const;
    
    void Normalize();

    T Magnitude();
    
    static T Distance(const TVector3<T>& Left, const TVector3<T>& Right);
};

template <typename T>
inline TVector3<T>::TVector3() : X(0), Y(0), Z(0)
{
}

template <typename T>
inline TVector3<T>::TVector3(T Scalar) : X(Scalar), Y(Scalar), Z(Scalar)
{
}

template <typename T>
inline TVector3<T>::TVector3(T InX, T InY, T InZ) : X(InX), Y(InY), Z(InZ)
{
}

template <typename T>
inline TVector3<T>::TVector3(const TVector3<T>& Other) : X(Other.X), Y(Other.Y), Z(Other.Z)
{
}

template <typename T>
inline TVector3<T> TVector3<T>::operator+(T Other)
{
    return TVector3<T>(X + Other, Y + Other, Z + Other);
}

template <typename T>
inline TVector3<T> TVector3<T>::operator-(T Other)
{
    return TVector3<T>(X - Other, Y - Other, Z - Other);
}

template <typename T>
inline TVector3<T> TVector3<T>::operator*(T Other)
{
    return TVector3<T>(X * Other, Y * Other, Z * Other);
}

template <typename T>
inline TVector3<T> TVector3<T>::operator/(T Other)
{
    return TVector3<T>(X / Other, Y / Other, Z / Other);
}

template <typename T>
inline TVector3<T> TVector3<T>::operator+(const TVector3<T>& Other) const
{
    return TVector3<T>(X + Other.X, Y + Other.Y, Z + Other.Z);
}

template <typename T>
inline TVector3<T> TVector3<T>::operator-(const TVector3<T>& Other) const
{
    return TVector3<T>(X - Other.X, Y - Other.Y, Z - Other.Z);
}

template <typename T>
inline TVector3<T> TVector3<T>::operator*(const TVector3<T>& Other) const
{
    return TVector3<T>(X * Other.X, Y * Other.Y, Z * Other.Z);
}

template <typename T>
inline TVector3<T> TVector3<T>::operator/(const TVector3<T>& Other) const
{
    return TVector3<T>(X / Other.X, Y / Other.Y, Z / Other.Z);
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator+=(T Other)
{
    X += Other;
    Y += Other;
    Z += Other;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator-=(T Other)
{
    X -= Other;
    Y -= Other;
    Z -= Other;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator*=(T Other)
{
    X *= Other;
    Y *= Other;
    Z *= Other;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator/=(T Other)
{
    X /= Other;
    Y /= Other;
    Z /= Other;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator+=(const TVector3<T>& Other)
{
    X += Other.X;
    Y += Other.Y;
    Z += Other.Z;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator-=(const TVector3<T>& Other)
{
    X -= Other.X;
    Y -= Other.Y;
    Z -= Other.Z;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator*=(const TVector3<T>& Other)
{
    X *= Other.X;
    Y *= Other.Y;
    Z *= Other.Z;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator/=(const TVector3<T>& Other)
{
    X /= Other.X;
    Y /= Other.Y;
    Z /= Other.Z;
    return *this;
}

template <typename T>
inline TVector3<T> TVector3<T>::Zero() const
{
    return TVector<T>(0, 0, 0);
}

template <typename T>
inline TVector3<T> TVector3<T>::One() const
{
    return TVector<T>(1, 1, 1);
}

template <typename T>
void TVector3<T>::Normalize()
{
    T Mag = Magnitude();

    if (Mag > 0)
    {
        X /= Mag;
        Y /= Mag;
        Z /= Mag;
    }
}

template <typename T>
T TVector3<T>::Magnitude()
{
    return Math::Sqrt(X * X + Y * Y + Z * Z);
}

template <typename T>
T TVector3<T>::Distance(const TVector3<T>& Left, const TVector3<T>& Right)
{
    return Math::Sqrt(Math::Pow(Right.X - Left.X, 2) +
            Math::Pow(Right.Y - Left.Y, 2) +
            Math::Pow(Right.Z - Left.Z, 2));
}

typedef TVector3<int> SVector3i;
typedef TVector3<float> SVector3f;
typedef TVector3<double> SVector3d;