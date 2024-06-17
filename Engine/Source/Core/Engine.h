#pragma once

#include <chrono>
#include <mutex>

#include "Math/MathTypes.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#ifndef FORCEINLINE
	#define FORCEINLINE __forceinline
#endif

class CImGui;
class CWindow;
class CRenderer;
class CScene;

typedef std::size_t SizeType;

//#ifdef MOD_DEBUG
	//#define ASSERT(Condition, Message) if (!(Condition)) { __debugbreak(); }
	#define ASSERT(Condition, Message) { __debugbreak(); }
//#endif

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

static constexpr int32 PARAMETER_VIEWPORT_WIDTH = 1920;
static constexpr int32 PARAMETER_VIEWPORT_HEIGHT = 1080;

/* A duration of time in seconds. */
struct STimespan
{
	using SClock = std::chrono::high_resolution_clock;
	using STimePoint = std::chrono::time_point<SClock>;
	using SDuration = std::chrono::duration<float>;

	STimePoint Start;
	STimePoint Current;
	SDuration DeltaTime;

	STimespan()
	{
		Start = SClock::now();
		Current = Start;
		DeltaTime = SDuration(0);
	}

	void Validate()
	{
		auto NewTime = SClock::now();
		DeltaTime = NewTime - Current;
		Current = NewTime;
	}

	FORCEINLINE float GetDeltaTime() const
	{
		return DeltaTime.count();
	}

	FORCEINLINE float GetCurrentTime() const
	{
		return std::chrono::duration<float>(Current - Start).count();
	}

	FORCEINLINE float GetStartTime() const
	{
		return std::chrono::duration<float>(Start.time_since_epoch()).count();
	}
};

struct SMetrics
{
	uint32 DrawCallCounter;
	
	uint32 CurrentObjectAllocated;
	uint32 TotalObjectAllocated;

	size_t CurrentSizeAllocated;
	size_t TotalSizeAllocated;

	void Reset()
	{
		DrawCallCounter = 0;
	}
};

class CEngine
{
public:
	virtual ~CEngine() = default;
	
	static inline CEngine* Get()
	{
		return GEngine; 
	}

	void Start();
	void Run();
	void Stop();
	
	CWindow* GetWindow() const;
	CRenderer* GetRenderer() const;
	CScene* GetScene() const;

	STimespan Time;
	SMetrics Metrics;
	
protected:
	virtual void OnStart() {}
	virtual void OnUpdate(float DeltaTime) {}
	virtual void OnStop() {}
	
	CWindow* Window;
	CRenderer* Renderer;
	CScene* Scene;
	
	static CEngine* GEngine;
};

static inline CEngine* GetEngine()
{
	return CEngine::Get();
}