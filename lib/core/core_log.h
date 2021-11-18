#pragma once

#include "base\base_all.h"

namespace mylib
{
	enum enum_log_level {
		LogLevel_Err = 0,
		LogLevel_Wrn,
		LogLevel_Msg,
	};
	inline constexpr strview_t GetEnumString_LogLevel(enum_log_level level) {
		switch (level) {
		case LogLevel_Err: return "ERR";
		case LogLevel_Wrn: return "WRN";
		case LogLevel_Msg: return "MSG";
		default: return "NOP";
		}
	}

	class Logger : public single_ptr<Logger>
	{
	public:
		Logger() = default;
		~Logger() = default;

		Logger(const Logger&) = delete;
		Logger(Logger&&) = delete;

		bool Init();
		void Cleanup();

		void Log(enum_log_level level, strview_t file, int32_t line, uint32_t thread_id, strview_t fmt, std::format_args&& args);
	};

#ifdef DEF_NOT_USING_CORE_LOGGER
	#define logmsg(format, ...) 
	#define logwrn(format, ...) 
	#define logerr(format, ...) 
#else
	#define logmsg(format, ... ) Logger::get()->Log(LogLevel_Msg, __FILE__, __LINE__, _Thrd_id(), format, std::make_format_args(__VA_ARGS__))
	#define logwrn(format, ... ) Logger::get()->Log(LogLevel_Wrn, __FILE__, __LINE__, _Thrd_id(), format, std::make_format_args(__VA_ARGS__))
	#define logerr(format, ... ) Logger::get()->Log(LogLevel_Err, __FILE__, __LINE__, _Thrd_id(), format, std::make_format_args(__VA_ARGS__))
#endif
}