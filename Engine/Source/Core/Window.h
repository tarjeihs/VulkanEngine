#pragma once

#include <glm/vec2.hpp>

#include "Math/MathTypes.h"
#include "Engine.h"

class CRendererContext;
class CInput;
class CEvent;

struct SWindowSpecification
{
    const char* Title;

    uint32 Width;

    uint32 Height;
};

struct SWindowUserData
{
};

enum class ECursorMode
{
    None = 0, Visible, Hidden
};

class CWindow
{
protected:
    SWindowSpecification Specification;

    SWindowUserData UserData;
    
    CRendererContext* RendererContext;

    void* WindowHandle = nullptr;

    bool bMinimized = false;

public:
    CWindow(const SWindowSpecification& InSpecification)
        : Specification(InSpecification)
    {
    }

    virtual ~CWindow() = default;
    
    virtual void CreateNativeWindow() = 0;
    virtual void DestroyWindow() = 0;

    virtual void Poll() = 0;
    virtual void Swap() = 0;
    
    virtual bool ShouldClose() const = 0;

    inline CRendererContext* GetContext() const
    {
        return RendererContext;
    }

    inline void* GetNativeWindow() const
    {
        return WindowHandle;
    }

    const char* GetTitle() const
    {
        return Specification.Title;
    }
    
    uint32 GetWidth() const
    {
        return Specification.Width;
    }

    uint32 GetHeight() const
    {
        return Specification.Height;
    }

    inline bool IsMinimized() const
    {
        return bMinimized;
    }

    inline void SetIsMinimized(bool bValue)
    {
        bMinimized = bValue;
    }
};

static inline CWindow* GetWindow()
{
    return CEngine::Get()->GetWindow();
}
