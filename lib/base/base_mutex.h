#pragma once

#include "base/base_libraries.h"

namespace mylib
{
	using lt_lock = std::recursive_mutex;
	using scoped_lt_lock = std::unique_lock<lt_lock>;
	using cond_var = std::condition_variable_any;
}