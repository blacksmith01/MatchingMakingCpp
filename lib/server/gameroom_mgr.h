#pragma once

#include "game\game_all.h"

namespace myproj
{
	class GameRoomMgr : public single_ptr<GameRoomMgr>
	{
	public:
		GameRoomMgr() = default;
		~GameRoomMgr() = default;

	public:
		bool Init();
		void Cleanup();

		void AddRoom(const player_info* players, int32_t len);
	};
}