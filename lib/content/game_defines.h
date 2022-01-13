#pragma once

#include "core/core_all.h"

namespace myproj
{
	/*
	*
	���� ��Ī��

	1. 8�� free for all ����

	2. �⺻������ ��Ī ���� ���� ������ �ְ� ��� �ð��� ��������� ������ �о�����.

	3. ��Ī �Ϸ�� ��� �Ұ����ϰ� �ٷ� ���� ����.

	*/


	using result_t = uint32_t;

	enum enum_result_codes
	{
		RESULT_OK = 0,

		RESULT_UNKNOWN,

		RESULT_MATCHING_ADD_INVALID_REQUEST,
		RESULT_MATCHING_ADD_DUPLICATED,
		RESULT_MATCHING_DEL_NOT_REQUESTED,
		RESULT_MATCHING_DEL_DUPLICATED,
		RESULT_MATCHING_DEL_ALREADY_MATCHED,
	};

	using PlayerNo = int64_t;
	constexpr inline bool IsValidPlayerNo(PlayerNo no) {
		return no > 0;
	}

	using PlayerGrade = uint8_t;
	enum enum_player_grades
	{
		PLAYER_GRADE_NONE = 0,
		PLAYER_GRADE_BRONZE,
		PLAYER_GRADE_SILVER,
		PLAYER_GRADE_GOLD,

		PLAYER_GRADE__MIN = PLAYER_GRADE_BRONZE,
		PLAYER_GRADE__MAX = PLAYER_GRADE_GOLD,
	};
	constexpr inline bool IsValidPlayerGrade(PlayerGrade grade) {
		return (grade >= PLAYER_GRADE__MIN) && (grade <= PLAYER_GRADE__MAX);
	}

	using GamePoint = int32_t;
	enum enum_game_point_for_grade_starts
	{
		GAME_POINT_GRADE_START_BRONZE = 0,
		GAME_POINT_GRADE_START_SILVER = 5000,
		GAME_POINT_GRADE_START_GOLD = 10000,
	};
	constexpr inline bool IsValidGamePoint(GamePoint grade) {
		return grade >= 0;
	}
	constexpr enum_player_grades GetPlayerGrade_ByGamePoint(GamePoint pt) {
		if (pt < GAME_POINT_GRADE_START_SILVER) {
			return PLAYER_GRADE_BRONZE;
		}
		else if (pt < GAME_POINT_GRADE_START_GOLD) {
			return PLAYER_GRADE_SILVER;
		}
		else {
			return PLAYER_GRADE_GOLD;
		}
	}

	enum enum_game_constants
	{
		MAX_MATCHING_PLAYER_COUNT = 8,
		MAX_MATCHING_PHASE_COUNT = 4,
	};

	const int MATCHING_PHASE_TIME_SEC_ARR[MAX_MATCHING_PHASE_COUNT] = 
	{
		5,
		10,
		30,
		60,
	};
	const GamePoint MATCHING_PHASE_POINT_BOUND_ARR[MAX_MATCHING_PHASE_COUNT] = 
	{
		250,
		1000,
		1500,
		2000,
	};

	inline constexpr GamePoint GetPointBound_ByRegistTime(int32_t elapsed_sec)
	{
		for (int i = 0; i < MAX_MATCHING_PHASE_COUNT; i++)
		{
			if (elapsed_sec < MATCHING_PHASE_TIME_SEC_ARR[i]) {
				return MATCHING_PHASE_POINT_BOUND_ARR[i];
			}
		}
		return MATCHING_PHASE_POINT_BOUND_ARR[MAX_MATCHING_PHASE_COUNT-1];
	}

	struct player_info
	{
		PlayerNo no;
		GamePoint point;
	};
}