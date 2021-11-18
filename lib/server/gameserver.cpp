#include "proj\pch.h"
#include "gameserver.h"

#include "server\gameroom_mgr.h"
#include "server\matching_mgr.h"
#include "server\random_pkt_generator.h"

bool GameServer::Init()
{
	Logger::make_single();
	ThreadMgr::make_single();
	Scheduler::make_single();

	MatchingMgr::make_single();
	GameRoomMgr::make_single();
	RandomPktGenerator::make_single();

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
	if (!GameRoomMgr::get()->Init()) {
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
	GameRoomMgr::get()->Cleanup();
	MatchingMgr::get()->Cleanup();

	Scheduler::get()->Cleanup();
	ThreadMgr::get()->Cleanup();
	Logger::get()->Cleanup();

	RandomPktGenerator::delete_single();
	GameRoomMgr::delete_single();
	MatchingMgr::delete_single();

	Scheduler::delete_single();
	ThreadMgr::delete_single();
	Logger::delete_single();
}

void GameServer::Run()
{
	logmsg("gameserver running.");

	ThreadMgr::get()->Run();
	Scheduler::get()->Run();
	RandomPktGenerator::get()->Run();

	str_t str;
	while (str != "exit") {
		std::cin >> str;
	}

	logmsg("gameserver shutdown started.");

	RandomPktGenerator::get()->Shutdown();
	Scheduler::get()->Shutdown();
	ThreadMgr::get()->Shutdown();

	logmsg("gameserver shutdown finished.");
}