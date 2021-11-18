#pragma once

#include "base\base_all.h"

namespace mylib
{
	class Runable
	{
	public:
		virtual void Run() = 0;
		virtual void Shutdown() = 0;
	};
}