#pragma once

#include "game\game_all.h"

namespace myproj
{
	struct mathcing_player_node
	{
		player_info base;
		tickcnt_t regist_time;
		int32_t point_order = -1;
		int32_t regist_order = -1;

		uint8_t in_matching : 1 = 0;
		uint8_t del_reseved : 1 = 0;
		uint8_t matched : 1 = 0;
		uint8_t unknown : 5 = 0;

		int operator < (const mathcing_player_node& other) const {
			return base.point < other.base.point;
		}
	};

	struct sorting_node
	{
		mathcing_player_node* player;
		GamePoint point;
		uint8_t del_reseved : 1 = 0;
		uint8_t matched : 1 = 0;
		uint8_t unknown : 6 = 0;
	};

	class MatchingMgr : public single_ptr<MatchingMgr>, public Schedulable
	{
	public:
		MatchingMgr() = default;
		~MatchingMgr() = default;

	public:
		bool Init();
		void Cleanup();

	public:
		void OnSchedule(tickcnt_t now) override;
	private:
		void PrepareInqueue();
		void RebuildSortedList();
		void ProgressMatching(tickcnt_t now);

	public:
		result_t Request_AddPlayer(const player_info& p_info);
		result_t Request_DelPlayer(PlayerNo p_no, bool& directly_deleted);

	private:
		mathcing_player_node* AllocateNode(const player_info& p, tickcnt_t now);
		void FreeNode(mathcing_player_node* node);

		GamePoint GetPointBound_ByRegistTime(mathcing_player_node* node, tickcnt_t now);

	private:
		void Send_Canceled(mathcing_player_node* node);

	private:

		lt_lock _lock;
		int32_t _add_del_requested = 0;
		multiset_t<mathcing_player_node*, less_ptr<mathcing_player_node>> _add_queue;
		vector_t<mathcing_player_node*> _del_queue;

		multiset_t<mathcing_player_node*, less_ptr<mathcing_player_node>> _add_in_queue;
		vector_t<mathcing_player_node*> _del_in_queue;

		hash_map_t<PlayerNo, mathcing_player_node*> _players;

		vector_t<mathcing_player_node*> _replacer;
		vector_t<mathcing_player_node*> _sorted_point; // 점수 순 정렬 low -> high
		vector_t<mathcing_player_node*> _sorted_regist; // 매칭 순으로 정렬 old -> new

		int32_t _stat_cycles = {};
		int32_t _stat_matched = {};
		int32_t _stat_remain = {};
	};
}