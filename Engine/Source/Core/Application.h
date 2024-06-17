#pragma once
#include "Engine.h"
#include "Math/MathTypes.h"

class CApplication
{
public:
    CApplication(CEngine* InEngine)
        : Engine(InEngine)
    {
    }

    virtual ~CApplication()
    {
        delete Engine;    
    }

    void Start()
    {
        Engine->Start();
    }

    void Run()
    {
        Engine->Run();
    }

    void Stop()
    {
        Engine->Stop();
    }
    
protected:
    CEngine* Engine;
};
