#pragma once

#include "base/base_libraries.h"

namespace mylib
{
	using systime_t = int64_t;

	inline systime_t systime_now() {
		using namespace std::chrono;
		return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	}

	const systime_t systime_second = 1000;
	const systime_t systime_minute = systime_second * 60;
	const systime_t systime_hour = systime_minute * 60;
	const systime_t systime_day = systime_hour * 24;
}