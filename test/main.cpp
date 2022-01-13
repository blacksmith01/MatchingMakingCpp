#include "proj/pch.h"
#include "content/matching_mgr.h"
#include "content/server_time_mgr.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using namespace myproj;

TEST_CASE("matching_add") {
	scoped_singleton<ServerTimeManager> _;
	scoped_singleton<MatchingMgr> __;
	int64_t matchedGame = 0;
	bool direct_delted;
	MatchingMgr::get()->OnSchedule(systime_now());

	// �⺻ �߰�
	REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1,.point = 0 }) == RESULT_OK);

	// �ߺ� �߰�
	REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1,.point = 0 }) == RESULT_MATCHING_ADD_DUPLICATED);

	// ���� �� ���߰�
	REQUIRE((MatchingMgr::get()->Request_DelPlayer(1, direct_delted) == RESULT_OK && direct_delted == true));
	REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1,.point = 0 }) == RESULT_OK);

	// ��Īť�� ������Ʈ �� ����,���߰�
	MatchingMgr::get()->OnSchedule(systime_now());
	REQUIRE((MatchingMgr::get()->Request_DelPlayer(1, direct_delted) == RESULT_OK && direct_delted == false));
	REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1,.point = 0 }) == RESULT_MATCHING_ADD_DUPLICATED);
	MatchingMgr::get()->OnSchedule(systime_now());
	REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1,.point = 0 }) == RESULT_OK);

	// ��Ī �Ϸ� �� ���߰�
	for (int i = 1; i < 10; i++)
	{
		REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1 + i,.point = 0 }) == RESULT_OK);
	}
	MatchingMgr::get()->OnSchedule(systime_now());
	REQUIRE(MatchingMgr::get()->GetGameAllocated() == matchedGame + 1);
	matchedGame++;
	REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1,.point = 0 }) == RESULT_MATCHING_ADD_DUPLICATED);
	MatchingMgr::get()->OnSchedule(systime_now());
	REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1,.point = 0 }) == RESULT_OK);
}

TEST_CASE("matching_del") {
	scoped_singleton<ServerTimeManager> _;
	scoped_singleton<MatchingMgr> __;
	int64_t matchedGame = 0;
	bool direct_delted;
	MatchingMgr::get()->OnSchedule(systime_now());

	// ���� ����
	REQUIRE(MatchingMgr::get()->Request_DelPlayer(1, direct_delted) == RESULT_MATCHING_DEL_NOT_REQUESTED);

	// ��� ����
	REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1,.point = 0 }) == RESULT_OK);
	REQUIRE((MatchingMgr::get()->Request_DelPlayer(1, direct_delted) == RESULT_OK && direct_delted));

	// ���� ��õ�
	REQUIRE(MatchingMgr::get()->Request_DelPlayer(1, direct_delted) == RESULT_MATCHING_DEL_NOT_REQUESTED);

	// ��Īť�� ������Ʈ �� ����
	REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1,.point = 0 }) == RESULT_OK);
	MatchingMgr::get()->OnSchedule(systime_now());
	REQUIRE((MatchingMgr::get()->Request_DelPlayer(1, direct_delted) == RESULT_OK && direct_delted == false));
	REQUIRE(MatchingMgr::get()->Request_DelPlayer(1, direct_delted) == RESULT_MATCHING_DEL_DUPLICATED);
	MatchingMgr::get()->OnSchedule(systime_now());
	REQUIRE(MatchingMgr::get()->Request_DelPlayer(1, direct_delted) == RESULT_MATCHING_DEL_NOT_REQUESTED);

	// ��Ī �Ϸ� �� ����
	for (int i = 0; i < 10; i++)
	{
		REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1 + i,.point = 0 }) == RESULT_OK);
	}
	MatchingMgr::get()->OnSchedule(systime_now());
	REQUIRE(MatchingMgr::get()->GetGameAllocated() == matchedGame + 1);
	matchedGame++;
	REQUIRE(MatchingMgr::get()->Request_DelPlayer(1, direct_delted) == RESULT_MATCHING_DEL_ALREADY_MATCHED);
	MatchingMgr::get()->OnSchedule(systime_now());
	REQUIRE(MatchingMgr::get()->Request_DelPlayer(1, direct_delted) == RESULT_MATCHING_DEL_NOT_REQUESTED);
}

TEST_CASE("matching_point") {
	scoped_singleton<ServerTimeManager> _;
	scoped_singleton<MatchingMgr> __;
	int64_t matchedGame = 0;
	bool direct_delted;
	MatchingMgr::get()->OnSchedule(systime_now());

	// ���� ���� ��Ī
	for (int i = 0; i < MAX_MATCHING_PLAYER_COUNT; i++)
	{
		REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1 + i,.point = 0 }) == RESULT_OK);
	}
	MatchingMgr::get()->OnSchedule(systime_now());
	REQUIRE(MatchingMgr::get()->GetGameAllocated() == matchedGame + 1);
	matchedGame++;
	MatchingMgr::get()->OnSchedule(systime_now());

	// ���� �� ���� ��Ī
	for (int iPhase = 0; iPhase < MAX_MATCHING_PHASE_COUNT; iPhase++)
	{
		ServerTimeManager::get()->UpdateGameTimeMod(0);
		GamePoint ptMin = 1;
		GamePoint ptMax = 1 + MATCHING_PHASE_POINT_BOUND_ARR[iPhase];
		std::default_random_engine rand;
		REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1,.point = ptMin - 1 }) == RESULT_OK);
		REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 2,.point = ptMax + 1 }) == RESULT_OK);
		for (int iPlayer = 2; iPlayer < MAX_MATCHING_PLAYER_COUNT; iPlayer++)
		{
			auto randPoint = (GamePoint)(ptMin + (rand() % (ptMax - ptMin + 1)));
			REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = iPlayer + 1,.point = randPoint}) == RESULT_OK);
		}
		if (iPhase > 0)
		{
			ServerTimeManager::get()->UpdateGameTimeMod(MATCHING_PHASE_TIME_SEC_ARR[iPhase - 1] * systime_second);
		}
		MatchingMgr::get()->OnSchedule(systime_now());
		REQUIRE(MatchingMgr::get()->GetGameAllocated() == matchedGame);

		// ���� �� ������ ���� �� ��Ī
		for (int iPlayer = 0; iPlayer < MAX_MATCHING_PLAYER_COUNT; iPlayer++)
		{
			REQUIRE((MatchingMgr::get()->Request_DelPlayer(iPlayer + 1, direct_delted) == RESULT_OK && direct_delted == false));
		}
		ServerTimeManager::get()->UpdateGameTimeMod(0);
		MatchingMgr::get()->OnSchedule(systime_now());
		REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 1,.point = ptMin }) == RESULT_OK);
		REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = 2,.point = ptMax }) == RESULT_OK);
		for (int iPlayer = 2; iPlayer < MAX_MATCHING_PLAYER_COUNT; iPlayer++)
		{
			auto randPoint = (GamePoint)(ptMin + (rand() % (ptMax - ptMin + 1)));
			REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = iPlayer + 1,.point = randPoint }) == RESULT_OK);
		}
		if (iPhase > 0)
		{
			ServerTimeManager::get()->UpdateGameTimeMod(MATCHING_PHASE_TIME_SEC_ARR[iPhase - 1] * systime_second);
		}
		MatchingMgr::get()->OnSchedule(systime_now());
		REQUIRE(MatchingMgr::get()->GetGameAllocated() == matchedGame + 1);
		matchedGame++;
		MatchingMgr::get()->OnSchedule(systime_now());
	}

	// �ð��� ������ ���� ��Ī ��ȭ
	for (int iPhase = 1; iPhase < MAX_MATCHING_PHASE_COUNT; iPhase++)
	{
		ServerTimeManager::get()->UpdateGameTimeMod(0);
		GamePoint ptMin = 1;
		GamePoint ptMax = 1 + MATCHING_PHASE_POINT_BOUND_ARR[0];
		std::default_random_engine rand;
		for (int iPlayer = 0; iPlayer < MAX_MATCHING_PLAYER_COUNT; iPlayer++)
		{
			// �׽�Ʈ�� ������ ������ �ϳ� �־��ش�.
			auto point = iPlayer == 0 ? MATCHING_PHASE_POINT_BOUND_ARR[iPhase] : (GamePoint)(ptMin + (rand() % (ptMax - ptMin + 1)));
			REQUIRE(MatchingMgr::get()->Request_AddPlayer(player_info{ .no = iPlayer + 1,.point = point }) == RESULT_OK);
		}
		MatchingMgr::get()->OnSchedule(systime_now());
		REQUIRE(MatchingMgr::get()->GetGameAllocated() == matchedGame);

		// �ð� ���� �� ��Ī
		ServerTimeManager::get()->UpdateGameTimeMod(MATCHING_PHASE_TIME_SEC_ARR[iPhase - 1] * systime_second);
		MatchingMgr::get()->OnSchedule(systime_now());
		REQUIRE(MatchingMgr::get()->GetGameAllocated() == matchedGame + 1);
		matchedGame++;
		MatchingMgr::get()->OnSchedule(systime_now());
	}
}