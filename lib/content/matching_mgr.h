#pragma once

#include "content/game_defines.h"

/*

8�� free for all ��Ī ����

- Ŭ���̾�Ʈ ��û�� ��� �ݿ����� �ʰ� ť�� �׾� �ξ��ٰ� Ÿ�̸� ȣ��� ���� �� ��Ī ó��.
- Lock ������ ���̱� ���� ����Ʈ�� ����. PlayerNo �� �ؽ��� index�� ����.
- Ŭ���̾�Ʈ�� ��� ��û�� �����ߴ��� ��Ī�� �� �� �ֱ� ������ ��� �Ϸ� �Ǵ� ��Ī ���� ��Ŷ�� ��ٷ��� ��.

��Ī��
- ����� ������ �÷��̾��� ��Ī��Ų��. �ڽŰ� ��Ī�� �� �ִ� �ִ�,�ּ� ���� ���� �÷��̵鸸 ��Ī�ȴ�.
- ��� �ð��� ��������� ��Ī������ �ִ�,�ּ� ������ �о�����.
- ��� �ð��� ������ �÷��̾� �켱���� ��Ī��Ų��.

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
		vector_t<mathcing_player_node*> _sorted_point; // ���� �� ���� low -> high
		vector_t<mathcing_player_node*> _sorted_regist; // ��Ī ������ ���� old -> new

		int64_t _game_id_alloc = 0;

		int32_t _stat_cycles = {};
		int32_t _stat_matched = {};
		int32_t _stat_matched_latest = {};
		int32_t _stat_remain = {};
	};
}