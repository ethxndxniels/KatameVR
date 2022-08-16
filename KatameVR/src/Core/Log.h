#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Katame
{
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define KM_CORE_TRACE(...)		::Katame::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KM_CORE_INFO(...)		::Katame::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KM_CORE_WARN(...)	    ::Katame::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KM_CORE_ERROR(...)	    ::Katame::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KM_CORE_CRITICAL(...)   ::Katame::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define KM_TRACE(...)			::Katame::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KM_INFO(...)			::Katame::Log::GetClientLogger()->info(__VA_ARGS__)
#define KM_WARN(...)			::Katame::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KM_ERROR(...)			::Katame::Log::GetClientLogger()->error(__VA_ARGS__)
#define KM_CRITICAL(...)		::Katame::Log::GetClientLogger()->critical(__VA_ARGS__)