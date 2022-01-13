#pragma once

#include "content/game_defines.h"

namespace myproj
{
	struct mathcing_player_node
	{
		player_info base;
		uint32_t regist_time;
		GamePoint point_bound = 0;
		uint32_t point_order = 0;
		uint32_t regist_order = 0;

		uint8_t in_matching : 1 = 0;
		uint8_t del_reseved : 1 = 0;
		uint8_t game_matched : 1 = 0;
		uint8_t unknown : 5 = 0;

		int operator < (const mathcing_player_node& other) const {
			return base.point < other.base.point;
		}
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
		void OnSchedule(systime_t now) override;
	private:
		void PrepareProcqueue();
		void RebuildSortedList(systime_t now);
		void ProgressMatching(systime_t now);
		void OnMatchedGame(mathcing_player_node** nodes, int32_t count, GamePoint avgPoint);

	public:
		result_t Request_AddPlayer(const player_info& p_info);
		result_t Request_DelPlayer(PlayerNo p_no, bool& directly_deleted);

	public:
		int64_t GetGameAllocated() const { return _game_id_alloc; }

	private:
		mathcing_player_node* AllocateNode(const player_info& p, systime_t now) const;
		void FreeNode(mathcing_player_node* node) const;

	private:
		void Send_Canceled(mathcing_player_node* node);

	private:

		lt_lock _lock;
		int32_t _add_del_requested = 0;
		multiset_t<mathcing_player_node*, less_ptr<mathcing_player_node>> _add_queue;
		vector_t<mathcing_player_node*> _del_queue;

		multiset_t<mathcing_player_node*, less_ptr<mathcing_player_node>> _add_proc_queue;
		vector_t<mathcing_player_node*> _del_proc_queue;

		hash_map_t<PlayerNo, mathcing_player_node*> _players;

		vector_t<mathcing_player_node*> _replacer;
		vector_t<mathcing_player_node*> _sorted_point; // 점수 순 정렬 low -> high
		vector_t<mathcing_player_node*> _sorted_regist; // 매칭 순으로 정렬 old -> new

		int64_t _game_id_alloc = 0;

		int32_t _stat_cycles = {};
		int32_t _stat_matched = {};
		int32_t _stat_remain = {};
	};
}