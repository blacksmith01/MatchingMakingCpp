#pragma once

#include "base\base_libraries.h"
#include "base\base_container.h"

namespace mylib
{
	template <class T>
	struct less_ptr {
		bool operator()(const T* l, const T* r) const {
			return *l < *r;
		}
	};
	template <class T>
	struct greater_ptr {
		bool operator()(const T* l, const T* r) const {
			return *l > *r;
		}
	};
}