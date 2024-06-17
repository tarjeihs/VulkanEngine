#pragma once

#include <functional>

#include "Memory/Memory.h"

class CRenderCommand
{
public:
    CRenderCommand() = default;
    CRenderCommand(const std::function<void()>& Func)
        : Command(Func)
    {
    }

    std::function<void()> Command;
};

class CRenderQueue
{
public:
    void AddCommand(const CRenderCommand& Command)
    {
        Commands.Push(Command);
    }

    void Sort()
    {
        // Implement sorting logic if needed (e.g., by shader, material, etc.)
        // std::sort(commands.begin(), commands.end(), [](const RenderCommand& a, const RenderCommand& b) { /* sorting logic */ });
    }

    void ExecuteCommands()
    {
        for (const CRenderCommand& RenderCommand : Commands)
        {
            RenderCommand.Command();
        }
        Commands.Empty();
    }

    TArray<CRenderCommand> Commands;
};


class CRenderer
{
public:
    virtual ~CRenderer() = default;
    
    virtual void BeginFrame() = 0;
    //virtual void Submit(CMesh* Mesh, CMaterialInstance* MaterialInstance, const STransform& Transform) = 0;
    virtual void EndFrame() = 0;

protected:
    CRenderQueue RenderQueue;
};