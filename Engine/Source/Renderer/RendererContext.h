#pragma once

class CRendererContext
{
protected:
    void* ContextHandle;
    
public:
    virtual ~CRendererContext() = default;
    
    virtual void Init() = 0;
    virtual void Destroy() = 0;
};
