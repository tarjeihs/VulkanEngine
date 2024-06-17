#include "EnginePCH.h"
#include "Mem.h"

#include <mutex>

SMemoryMetrics GMemoryMetrics;

SMemoryMetrics& GetMemoryMetrics()
{
    return GMemoryMetrics;
}

void* operator new(size_t size)
{
    GMemoryMetrics.TotalHeapAllocations++;

    // Allocate extra memory to store the size
    size_t totalSize = size + sizeof(size_t);
    void* p = std::malloc(totalSize);
    if (!p)
    {
        throw std::bad_alloc();
    }

    // Store the size at the beginning of the block
    *reinterpret_cast<size_t*>(p) = size;
    GMemoryMetrics.CurrentHeapAllocation += size;

    // Return a pointer to the memory block after the size
    return static_cast<char*>(p) + sizeof(size_t);
}

void* operator new[](size_t size)
{
    return ::operator new(size);
}

void operator delete(void* p)
{
    if (!p)
    {
        return;
    }

    // Adjust the pointer to get the original allocated block
    char* block = static_cast<char*>(p) - sizeof(size_t);

    // Retrieve the size from the beginning of the block
    size_t size = *reinterpret_cast<size_t*>(block);

    GMemoryMetrics.CurrentHeapAllocation -= size;
    GMemoryMetrics.TotalHeapDeallocations++;

    // Free the original allocated block
    std::free(block);
}

void operator delete[](void* p) noexcept
{
    ::operator delete(p);
}