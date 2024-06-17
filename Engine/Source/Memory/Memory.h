#pragma once

#include <algorithm>
#include <assert.h>
#include <stdexcept>
#include <utility>
#include <type_traits>

template<typename TElement>
class TArrayView;

template<typename T>
class TUniquePtr
{
private:
    T* Pointer;

public:
    explicit TUniquePtr(T* InPointer = nullptr)
        : Pointer(InPointer)
    {
    }

    TUniquePtr(TUniquePtr&& Other) noexcept
        : Pointer(Other.Pointer)
    {
        Other.Pointer = nullptr;
    }

    ~TUniquePtr()
    {
        delete Pointer;
    }

    TUniquePtr(const TUniquePtr&) = delete;
    TUniquePtr& operator=(const TUniquePtr&) = delete;

    TUniquePtr& operator=(TUniquePtr&& Other) noexcept
    {
        if (this != &Other)
        {
            delete Pointer;
            Pointer = Other.Pointer;
            Other.Pointer = nullptr;
        }
        return *this;
    }
    
    explicit operator bool() const
    {
        return IsValid();
    }

    T& operator*() const
    {
        return *Get();
    }

    T* operator->() const
    {
        return Get();
    }

    T* Get() const
    {
        return Pointer;
    }

    T* Release()
    {
        T* TempPointer = Pointer;
        Pointer = nullptr;
        return TempPointer;
    }

    void Reset(T* NewPointer = nullptr)
    {
        if (Pointer != NewPointer)
        {
            delete Pointer;
            Pointer = NewPointer;
        }
    }

    bool IsValid() const
    {
        return Pointer != nullptr;
    }
};

template<typename T>
class TSharedPtr
{
private:
    T* Pointer;

    size_t* ReferenceCount;

public:
    TSharedPtr()
        : Pointer(nullptr), ReferenceCount(0)
    {
    }
    
    explicit TSharedPtr(T* InPointer)
        : Pointer(InPointer)
    {
        ReferenceCount = (InPointer ? new size_t(1) : nullptr);
    }

    ~TSharedPtr()
    {
        if (ReferenceCount && --(*ReferenceCount) == 0)
        {
            delete Pointer;
            delete ReferenceCount;
        }
    }

    TSharedPtr(const TSharedPtr& Other)
        : Pointer(Other.Pointer)
    {
        ReferenceCount = Other.ReferenceCount;
        
        if (ReferenceCount)
        {
            ++(*ReferenceCount);
        }
    }

    TSharedPtr(TSharedPtr&& Other) noexcept
        : Pointer(Other.Pointer)
    {
        ReferenceCount = Other.ReferenceCount;

        Other.Pointer = nullptr;
        Other.ReferenceCount = nullptr;
    }

    TSharedPtr& operator=(const TSharedPtr& Other)
    {
        if (this != &Other)
        {
            if (ReferenceCount && --(*ReferenceCount) == 0) {
                delete Pointer;
                delete ReferenceCount;
            }
            
            Pointer = Other.Pointer;
            ReferenceCount = Other.ReferenceCount;

            if (ReferenceCount)
            {
                ++(*ReferenceCount);     
            }
        }
        return *this;
    }

    TSharedPtr& operator=(TSharedPtr&& Other) noexcept
    {
        if (this != &Other)
        {
            Pointer = Other.Pointer;
            ReferenceCount = Other.ReferenceCount;

            Other.Pointer = nullptr;
            Other.ReferenceCount = nullptr;
        }
        return *this;
    }

    TSharedPtr& operator=(decltype(__nullptr))
    {
        Reset(nullptr);
        return *this;
    }

    explicit operator bool() const
    {
        return IsValid();
    }

    T& operator*() const
    {
        return *Get();
    }

    T* operator->() const
    {
        return Get();
    }

    T* Get() const
    {
        return Pointer;
    }

    T* Release()
    {
        T* TempPointer = Pointer;
        if (ReferenceCount && --(*ReferenceCount) == 0)
        {
            delete ReferenceCount;
        }
        Pointer = nullptr;
        ReferenceCount = nullptr;
        return TempPointer;
    }

    void Reset(T* NewPointer = nullptr)
    {
        if (Pointer != NewPointer)
        {
            if (ReferenceCount && --(*ReferenceCount) == 0)
            {
                delete Pointer;
                delete ReferenceCount;
            }

            Pointer = NewPointer;
            ReferenceCount = (NewPointer ? new size_t(1) : nullptr);
        }
    }

    bool IsValid() const
    {
        return Pointer != nullptr;
    }

    bool IsUnique() const
    {
        return GetReferenceCount() == 1;
    }
    
    size_t GetReferenceCount() const
    {
        return *ReferenceCount;
    }
};

/*
 * TArray is contiguous and fixed-size, while also a dynamic memory. 
 */
template<typename TElement>
class TArray
{
private:
    size_t Size;

    size_t Capacity;

    TElement* Data;

public:
    TArray()
        : Size(0), Capacity(1), Data(new TElement[Capacity])
    {
    }

    TArray(std::initializer_list<TElement> Init)
        : Size(Init.size()), Capacity(Init.size()), Data(new TElement[Capacity])
    {
        std::copy(Init.begin(), Init.end(), Data);
    }

    TArray(const TArray& Other)
        : Size(Other.Size), Capacity(Other.Capacity), Data(new TElement[Capacity])
    {
        std::copy(Other.Data, Other.Data + Other.Size, Data);
    }

    TArray(TArray&& Other) noexcept
        : Size(Other.Size), Capacity(Other.Capacity), Data(Other.Data)
    {
        Other.Size = 0;
        Other.Capacity = 0;
        Other.Data = nullptr;
    }

    ~TArray()
    {
        delete[] Data;
    }

    TArray& operator=(const TArray& Other)
    {
        if (this != &Other)
        {
            TElement* NewData = new TElement[Other.Capacity];
            std::copy(Other.Data, Other.Data + Other.Size, NewData);
            delete[] Data;
            Size = Other.Size;
            Capacity = Other.Capacity;
            Data = NewData;
        }
        return *this;
    }

    TArray& operator=(TArray&& Other) noexcept // Move
    {
        if (this != &Other)
        {
            delete[] Data;
            Size = Other.Size;
            Capacity = Other.Capacity;
            Data = Other.Data;
            Other.Size = 0;
            Other.Capacity = 0;
            Other.Data = nullptr;
        }
        return *this;
    }

    TElement& operator[](size_t Index)
    {
        if (Index >= Size) throw std::out_of_range("Index is out of range.");
        return Data[Index];
    }

    const TElement& operator[](size_t Index) const
    {
        if (Index >= Size) throw std::out_of_range("Index is out of range.");
        return Data[Index];
    }

    TElement& At(size_t Index)
    {
        if (Index >= Size) throw std::out_of_range("Index is out of range.");
        return Data[Index];
    }

    const TElement& At(size_t Index) const
    {
        if (Index >= Size) throw std::out_of_range("Index is out of range.");
        return Data[Index]; 
    }

    TElement* begin() { return Data; }
    TElement* end() { return Data + Size; }

    const TElement* begin() const { return Data; }
    const TElement* end() const { return Data + Size; }
    
public:
    void Init(size_t NewCapacity, TElement InitToValue = nullptr)
    {
        Reserve(NewCapacity);

        for (size_t Index = 0; Index < NewCapacity; ++Index)
        {
            Push(InitToValue);
        }
    }
    
    void Reserve(size_t NewCapacity)
    {
        if (NewCapacity > Capacity)
        {
            TElement* NewData = new TElement[NewCapacity];
            std::move(Data, Data + Size, NewData);
            delete[] Data;
            Capacity = NewCapacity;
            Data = NewData;
        }
    }
    
    void Push(const TElement& Value)
    {
        if (Size >= Capacity)
        {
            Reserve(Capacity * 2);
        }

        Data[Size++] = Value;
    }
    
    void Push(TElement&& Value)
    {
        if (Size >= Capacity)
        {
            Reserve(Capacity * 2);
        }

        Data[Size++] = std::move(Value);
    }

    TElement Pop()
    {
        if (Size > 0)
        {
            Data[--Size].~TElement();
        }
        return TElement();
    }

    void Empty()
    {
        while (Size > 0)
        {
            Data[--Size].~TElement();
        }
    }

    size_t GetSize() const
    {
        return Size;
    }

private:
    friend class TArrayView<TElement>;
};

/*
 * Non-owning, fixed-size view of a contiguous block of memory (the TArray).
 */
template<typename TElement>
class TArrayView
{
private:
    TElement* Data;

    size_t Size;

public:
    TArrayView(TElement* InData, size_t InSize)
        : Data(InData), Size(InSize)
    {
    }

    TArrayView(const TArray<TElement>& Array)
        : Data(Array.Data), Size(Array.Size)
    {
    }

    inline TElement& operator[](size_t Index)
    {
        if (Index >= Size) throw std::out_of_range("Index is out of range.");
        return Data[Index];
    }

    inline const TElement& operator[](size_t Index) const
    {
        if (Index >= Size) throw std::out_of_range("Index is out of range.");
        return Data[Index];
    }

    inline size_t GetSize() const
    {
        return Size;
    }
};

template<typename T, typename... TArgs>
static TUniquePtr<T> MakeUniquePtr(TArgs&&... Args)
{
    return TUniquePtr<T>(new T(std::forward<TArgs>(Args)...));
}

template<typename T>
static TUniquePtr<T> MakeUniquePtr(T* Other)
{
    return TUniquePtr<T>(Other);
}

template<typename T, typename... TArgs>
static TSharedPtr<T> MakeSharedPtr(TArgs&&... Args)
{
    return TSharedPtr<T>(new T(std::forward<TArgs>(Args)...));
}

template<typename T>
static TSharedPtr<T> MakeSharedPtr(T* Other)
{
    return TSharedPtr<T>(Other);
}

template<typename TFrom>
typename std::remove_reference<TFrom>::type&& Move(TFrom&& From)
{
    return static_cast<typename std::remove_reference<TFrom>::type&&>(From);
}

template<typename T>
static void MoveOwnership(T& From, T& To)
{
    To = Move(From);
}