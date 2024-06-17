#pragma once

#include "Engine.h"
#include "Math/MathTypes.h"
#include "Memory/Memory.h"

class CScene
{
public:
    void Tick(float DeltaTime)
    {
    }

    struct SMetrics
    {
        uint32 CurrentActorCount;
        uint32 TotalActorCount;
    };
    SMetrics Metrics;
};

static inline CScene* GetScene()
{
    return CEngine::Get()->GetScene();
}