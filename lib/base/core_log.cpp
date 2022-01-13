#include "proj/pch.h"
#include "core/core_log.h"

namespace mylib
{
	bool Logger::Init()
	{
		return true;
	}

	void Logger::Cleanup()
	{

	}

	void Logger::Log(enum_log_level level, strview_t file, int32_t line, uint32_t thread_id, strview_t fmt, std::format_args&& args)
	{
		

		auto dir_sep_idx = file.rfind('\\');
		if (dir_sep_idx != strview_t::npos) {
			file = file.substr(dir_sep_idx + 1);
		}

		auto time = std::chrono::system_clock::now();
		str_t str;
		auto it = std::back_inserter(str);
		std::format_to(it, "[{}][{:05}][{:%m-%d %H:%M:%S}] ", GetEnumString_LogLevel(level), thread_id, time);
		format_args_to(it, fmt, args);
		std::format_to(it, "\r\n");

		std::cout << str;
	}
}