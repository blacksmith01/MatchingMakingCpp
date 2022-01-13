#pragma once

#include "content/game_defines.h"

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

		static void SignalHandler(int sig);

	private:
		inline static volatile int _signal_value = {};
	};
}