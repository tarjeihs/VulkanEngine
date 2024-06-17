#include "EnginePCH.h"
#include "Log.h"

// Used for spdlog set_level function
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> CLog::GEngineLogger;

void CLog::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");

	GEngineLogger = spdlog::stderr_color_mt("Engine");
	GEngineLogger->set_level(spdlog::level::trace);

	RK_ENGINE_TRACE("Initializing Rocket Engine...");
}
