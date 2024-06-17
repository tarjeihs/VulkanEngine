#pragma once

#include "spdlog/spdlog.h"

// Ignores all warnings raised inside external header files
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

class CLog
{
public:
	static void Init();

	static inline std::shared_ptr<spdlog::logger>& GetEngineLogger()
	{
		return GEngineLogger;
	}

private:
	static std::shared_ptr<spdlog::logger> GEngineLogger;
};

#define RK_ENGINE_TRACE(...) ::CLog::GetEngineLogger()->trace(__VA_ARGS__)
#define RK_ENGINE_VERBOSE(...) ::CLog::GetEngineLogger()->debug(__VA_ARGS__)
#define RK_ENGINE_INFO(...)  ::CLog::GetEngineLogger()->info(__VA_ARGS__)
#define RK_ENGINE_WARNING(...)  ::CLog::GetEngineLogger()->warn(__VA_ARGS__)
#define RK_ENGINE_ERROR(...) ::CLog::GetEngineLogger()->error(__VA_ARGS__)