#pragma once

#include <cstddef>
#include <mutex>

#include "Mem.h"
#include "Memory.h"
#include "Core/Engine.h"

struct SAllocatorMetrics
{
    uint32 CurrentAllocatedObject;
    uint32 CurrentAllocatedMemory;

    uint32 TotalAllocatedObjects;
    uint32 TotalAllocatedMemory;
};

template<typename TAllocatorType, std::size_t BlockSize>
class TStackAllocator
{
    static_assert(BlockSize > 0, "BlockSize must be greater than zero.");
    
private:
    TAllocatorType* Memory;

    std::size_t Offset;

public:
    using ValueType = TAllocatorType;

    TStackAllocator()
        : Offset(0), Metrics(MakeUniquePtr<SAllocatorMetrics>())
    {
        Memory = Cast<TAllocatorType>(::operator new(BlockSize * sizeof(TAllocatorType)));
    }

    ~TStackAllocator()
    {
        ::operator delete(Memory);
    }

public:
    TAllocatorType* Allocate(std::size_t Size = 1)
    {
        ASSERT(Offset + Size <= BlockSize, "Stack overflow");
        TAllocatorType* Pointer = Memory + Offset; // 0 + Offset
        Offset += Size;
        Metrics->CurrentAllocatedObject++;
        Metrics->TotalAllocatedObjects++;
        return Pointer;
    }

    void Deallocate(TAllocatorType* Pointer, std::size_t Size)
    {
        ASSERT(Offset >= Size, "Stack underflow"); // Stack underflow
        Metrics->CurrentAllocatedObject--;
    }

    template<typename TPointer, typename... TArgs>
    FORCEINLINE void CallCTOR(TPointer* Pointer, TArgs&&... Args)
    {
        new(Pointer) TPointer(std::forward<TArgs>(Args)...);
    }

    template<typename TPointer>
    FORCEINLINE void CallDTOR(TPointer* Pointer)
    {
        Pointer->~TPointer();
    }

    FORCEINLINE TAllocatorType* First()
    {
        return Memory;
    }
    
    FORCEINLINE TAllocatorType* Last()
    {
        return Memory + Offset;
    }

    FORCEINLINE TAllocatorType* operator[](int32 Index)
    {
        return Offset > Index ? Memory + Index : nullptr;
    }

    FORCEINLINE const TAllocatorType* operator[](int32 Index) const
    {
        return Offset > Index ? Memory + Index : nullptr;
    }

    FORCEINLINE const SAllocatorMetrics& GetMetrics() const
    {
        return *Metrics.Get();
    }

private:
    TUniquePtr<SAllocatorMetrics> Metrics;
};

struct SBlock
{
    size_t Size;
    SBlock* Next;
};

template<typename TAllocatorType, SizeType TAllocatorSize>
class TAllocator
{
private:
    void* Position;

    SizeType Offset;

    TUniquePtr<SAllocatorMetrics> Metrics;
    
public:
    TAllocator()
    : Offset(0), Metrics(MakeUniquePtr<SAllocatorMetrics>())
    {
        Position = Cast<TAllocatorType>(::operator new(TAllocatorSize * sizeof(TAllocatorType)));
    }
    
    virtual ~TAllocator()
    {
        ::operator delete[](Position);
    }
};

template<typename TElement>
class TAllocatorView
{
    
};

template<typename TElement, std::size_t AllocatorSize>
class TObjectAllicator : public TStackAllocator<TElement, AllocatorSize> {};
