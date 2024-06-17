#pragma once
#include <cstdlib>
#include <type_traits>

#include "Math/MathTypes.h"

template<typename TElement>
struct TLinkedListNode
{
public:
    TElement Element;

    TLinkedListNode<TElement>* Previous = nullptr;

    TLinkedListNode<TElement>* Next = nullptr;

public:
    TLinkedListNode()
        : Next(nullptr), Previous(nullptr)
    {
    }
    
    ~TLinkedListNode()
    {
        if constexpr (std::is_pointer_v<TElement>)
        {
            delete Element;
        }
    }
};

// A double linked list
template<typename TElement>
class TLinkedList
{
private:
    TLinkedListNode<TElement>* Head;

    TLinkedListNode<TElement>* Tail;

    uint32 Size;
    
public:
    TLinkedList()
        : Head(nullptr), Tail(nullptr), Size(0)
    {
    }

    ~TLinkedList()
    {
        TLinkedListNode<TElement>* Current = Head;
        while (Current != nullptr)
        {
            TLinkedListNode<TElement>* Next = Current->Next;
            delete Current;
            Current = Next;
        }   
    }

    TElement& operator[](uint32 Index)
    {
        uint32 Count = 0;
        TLinkedListNode<TElement>* Current = Head;
        while (Current != nullptr && Count < Index)
        {
            TLinkedListNode<TElement>* Next = Current->Next;
            Current = Next;

            Count++;
        }
        return Current->Element;
    }
    
    const TElement& operator[](uint32 Index) const
    {
        uint32 Count = 0;
        TLinkedListNode<TElement>* Current = Head;
        while (Current != nullptr && Count < Index)
        {
            TLinkedListNode<TElement>* Next = Current->Next;
            Current = Next;

            Count++;
        }
        return Current->Element;
    }

public:
    void Insert(TElement Element)
    {
        if (Size == 0)
        {
            Head = new TLinkedListNode<TElement>();
            Head->Element = Element;
            Tail = Head;
        }
        else
        {
            TLinkedListNode<TElement>* OldTail = Tail;
            Tail = new TLinkedListNode<TElement>();
            Tail->Previous = OldTail;
            Tail->Element = Element;
            OldTail->Next = Tail;
        }
        ++Size;
    }

    void Remove(TElement Element);
};
