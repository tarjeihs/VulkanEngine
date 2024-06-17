#pragma once

#include "Core/Log.h"

#include <filesystem>

#define RK_EXPAND_MACRO(x) x
#define RK_STRINGIFY_MACRO(x) #x

#ifdef _MSC_VER
	#define RK_DEBUGBREAK() __debugbreak()
#else
	#define RK_DEBUGBREAK() __builtin_trap()
#endif

#define RK_ENABLE_ASSERTS

#ifdef RK_ENABLE_ASSERTS
	#define RK_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { RK##type##ERROR(msg, __VA_ARGS__); RK_DEBUGBREAK(); } }
	#define RK_INTERNAL_ASSERT_WITH_MSG(type, check, ...) RK_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define RK_INTERNAL_ASSERT_NO_MSG(type, check) RK_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed {0} at {1}:{2}", RK_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)
	
	#define RK_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define RK_INTERNAL_ASSERT_GET_MACRO(...) RK_EXPAND_MACRO(RK_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, RK_INTERNAL_ASSERT_WITH_MSG, RK_INTERNAL_ASSERT_NO_MSG))
	
	#define RK_ENGINE_ASSERT(...) RK_EXPAND_MACRO(RK_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_ENGINE_, __VA_ARGS__))
#else
	#define RK_ENGINE_ASSERT(...)
#endif