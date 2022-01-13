#include "proj/pch.h"
#include "gameserver.h"

#include "content/server_time_mgr.h"
#include "content/matching_mgr.h"
#include "content/random_pkt_generator.h"

namespace myproj
{
	bool GameServer::Init()
	{
		signal(SIGINT, SignalHandler);

		ServerTimeManager::make_single();
		Logger::make_single();
		ThreadMgr::make_single();
		Scheduler::make_single();

		MatchingMgr::make_single();
		RandomPktGenerator::make_single();

		if (!ServerTimeManager::get()->Init()) {
			return false;
		}
		if (!Logger::get()->Init()) {
			return false;
		}
		if (!Scheduler::get()->Init()) {
			return false;
		}
		if (!ThreadMgr::get()->Init()) {
			return false;
		}

		if (!MatchingMgr::get()->Init()) {
			return false;
		}
		if (!RandomPktGenerator::get()->Init()) {
			return false;
		}

		logmsg("gameserver initialized.");

		return true;
	}

	void GameServer::Cleanup()
	{
		logmsg("gameserver cleanup.");

		RandomPktGenerator::get()->Cleanup();
		MatchingMgr::get()->Cleanup();

		Scheduler::get()->Cleanup();
		ThreadMgr::get()->Cleanup();
		Logger::get()->Cleanup();
		ServerTimeManager::get()->Cleanup();

		RandomPktGenerator::delete_single();
		MatchingMgr::delete_single();

		Scheduler::delete_single();
		ThreadMgr::delete_single();
		Logger::delete_single();
		ServerTimeManager::delete_single();
	}

	void GameServer::Run()
	{
		logmsg("gameserver running.");

		ThreadMgr::get()->Run();
		Scheduler::get()->Run();
		RandomPktGenerator::get()->Run();

		while (_signal_value == 0) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		logmsg("gameserver shutdown started.");

		RandomPktGenerator::get()->Shutdown();
		Scheduler::get()->Shutdown();
		ThreadMgr::get()->Shutdown();

		logmsg("gameserver shutdown finished.");
	}

	void GameServer::SignalHandler(int sig)
	{
		_signal_value = sig;
	}
}