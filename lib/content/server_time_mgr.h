#pragma once

#include "content/game_defines.h"

namespace myproj
{
	class ServerTimeManager : public single_ptr<ServerTimeManager>
	{
	public:
		ServerTimeManager() = default;
		~ServerTimeManager() = default;

	public:
		bool Init();
		void Cleanup();

	public:
		systime_t GetGameTime();
		systime_t GetGameTime(systime_t systime);
		void UpdateGameTimeMod(systime_t mod_time);

	private:
		systime_t _game_time_mod = {};
	};
}