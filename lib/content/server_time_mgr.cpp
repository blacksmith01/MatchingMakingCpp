#include "proj/pch.h"
#include "content/server_time_mgr.h"

namespace myproj
{
	bool ServerTimeManager::Init()
	{
		return true;
	}

	void ServerTimeManager::Cleanup()
	{

	}

	systime_t ServerTimeManager::GetGameTime()
	{
		return systime_now() + _game_time_mod;
	}
	systime_t ServerTimeManager::GetGameTime(systime_t systime)
	{
		return systime + _game_time_mod;
	}
	void ServerTimeManager::UpdateGameTimeMod(systime_t mod_time)
	{
		_game_time_mod = mod_time;
	}
}