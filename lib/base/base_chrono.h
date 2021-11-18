#pragma once

#include "base\base_libraries.h"

namespace mylib
{
	using tickcnt_t = uint64_t;
	using sys_clock = std::chrono::system_clock;

	inline tickcnt_t tickcnt_now() {
		using namespace std::chrono;
		return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	}
	inline auto tickcnt_duration(tickcnt_t tick) {
		return std::chrono::milliseconds(tick);
	}

	const tickcnt_t tick_second = 1000;
	const tickcnt_t tick_minute = tick_second * 60;
	const tickcnt_t tick_hour = tick_minute * 60;
	const tickcnt_t tick_day = tick_hour * 24;
}