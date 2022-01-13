#pragma once

#include "content/game_defines.h"

/*

8인 free for all 매칭 서비스

- 클라이언트 요청시 즉시 반영하지 않고 큐에 쌓아 두었다가 타이머 호출시 병합 후 매칭 처리.
- Lock 경합을 줄이기 위해 리스트를 샤딩. PlayerNo 를 해시한 index로 접근.
- 클라이언트의 취소 요청이 성공했더라도 매칭이 될 수 있기 때문에 취소 완료 또는 매칭 성공 패킷을 기다려야 함.

매칭룰
- 비슷한 수준의 플레이어들과 매칭시킨다. 자신과 매칭될 수 있는 최대,최소 점수 내의 플레이들만 매칭된다.
- 대기 시간이 길어질수록 매칭점수의 최대,최소 간격이 넓어진다.
- 대기 시간이 오래된 플레이어 우선으로 매칭시킨다.

*/

namespace myproj
{
	struct mathcing_player_node
	{
		player_info base;
		uint32_t regist_time;
		GamePoint point_bound;
		uint32_t point_order;
		uint32_t regist_order;

		uint8_t in_matching : 1;
		uint8_t del_reseved : 1;
		uint8_t game_matched : 1;
		uint8_t unknown : 5;

		int operator < (const mathcing_player_node& other) const {
			if (base.point == other.base.point) {
				return base.no < other.base.no;
			}
			else {
				return base.point < other.base.point;
			}
		}
	};

	struct sharded_node_group
	{
		sharded_node_group();

		lt_lock lock;

		int32_t add_del_requested = 0;
		vector_t<mathcing_player_node*> add_queue;
		vector_t<mathcing_player_node*> del_queue;
		vector_t<mathcing_player_node*> matched_queue;
		hash_map_t<PlayerNo, mathcing_player_node*> players;
		vector_t<mathcing_player_node*> pool;

		vector_t<mathcing_player_node*> free_queue; // OnSchedule only.
	};

	class MatchingMgr : public single_ptr<MatchingMgr>, public Schedulable
	{
	public:
		MatchingMgr();
		~MatchingMgr();

	public:
		bool Init();
		void Cleanup();

	public:
		void OnSchedule(systime_t now) override;
	private:
		void PrepareProcQueue(uint32_t gidx);
		void RebuildSortedList(systime_t now);
		void ProgressMatching(systime_t now);
		void OnMatchedGame(mathcing_player_node** nodes, int32_t count, GamePoint avgPoint);

	public:
		result_t Request_AddPlayer(const player_info& p_info);
		result_t Request_DelPlayer(PlayerNo p_no, bool& directly_deleted);

	public:
		int64_t GetGameAllocated() const { return _game_id_alloc; }

	private:
		mathcing_player_node* AllocateNode(sharded_node_group& group, const player_info& p, systime_t now) const;
		void FreeNode(sharded_node_group& group, mathcing_player_node* node) const;

	private:
		void Send_MatchingSuccess(mathcing_player_node* node);
		void Send_MatchingDelCompleted(mathcing_player_node* node);

	private:
		sharded_node_group* _node_groups = {};

		set_t<mathcing_player_node*, less_ptr<mathcing_player_node>> _add_proc_queue;
		vector_t<mathcing_player_node*> _del_proc_queue;
		vector_t<mathcing_player_node*> _replacer;
		vector_t<mathcing_player_node*> _sorted_point; // 점수 순 정렬 low -> high
		vector_t<mathcing_player_node*> _sorted_regist; // 매칭 순으로 정렬 old -> new

		int64_t _game_id_alloc = 0;

		int32_t _stat_cycles = {};
		int32_t _stat_matched = {};
		int32_t _stat_matched_latest = {};
		int32_t _stat_remain = {};
	};
}