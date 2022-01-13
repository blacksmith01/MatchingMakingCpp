#pragma once

#include "base/base_libraries.h"

namespace mylib
{
	template <typename T>
	constexpr T n_max() {
		return std::numeric_limits<T>::max();
	}
}