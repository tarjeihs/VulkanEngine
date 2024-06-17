#pragma once

#include "MathTypes.h"
#include "Memory/Memory.h"

template<typename TNumeric>
class TMatrix
{
private:
    uint32 Rows;

    uint32 Columns;

    TArray<TNumeric> Data;

public:
    TMatrix()
        : Rows(1), Columns(1), Data()
    {
    }

    TMatrix(uint32 NumOfRows, uint32 NumOfColumns)
        : Rows(NumOfRows), Columns(NumOfColumns)
    {
        Data.Init(Rows * Columns, TNumeric());
    }

    ~TMatrix()
    {
    }

    inline TNumeric GetValue(const uint32 Row, const uint32 Column) const
    {
        return Data[Row * Columns + Column];
    }

    inline void SetValue(const uint32 Row, const uint32 Column, TNumeric Value)
    {
        Data[Row * Columns + Column] = Value;
    }
    
private:
    
};

//using SMatrix1x1f = TMatrix<float, 1, 1>;
//using SMatrix2x2f = TMatrix<float, 2, 2>;
//using SMatrix3x3f = TMatrix<float, 3, 3>;
//using SMatrix4x4f = TMatrix<float, 4, 4>;
