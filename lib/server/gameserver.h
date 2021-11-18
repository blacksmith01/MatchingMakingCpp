#pragma once

#include "game\game_all.h"

namespace myproj
{
	class GameServer
	{
	public:
		GameServer() = default;
		~GameServer() = default;

		GameServer(const GameServer&) = delete;
		GameServer(GameServer&&) = delete;

	public:
		bool Init();
		void Cleanup();

		void Run();

	private:

	};
}