#include "proj/pch.h"
#include "content/matching_mgr.h"
#include "content/server_time_mgr.h"

namespace myproj
{
	bool MatchingMgr::Init()
	{
		if (!Scheduler::get()->Add(this, systime_second)) {
			logerr("!add to Scheduler");
			return false;
		}

		return true;
	}

	void MatchingMgr::Cleanup()
	{

	}

	result_t MatchingMgr::Request_AddPlayer(const player_info& p_info)
	{
		if (!IsValidPlayerNo(p_info.no) || !IsValidGamePoint(p_info.point)) {
			return RESULT_MATCHING_ADD_INVALID_REQUEST;
		}

		auto now = ServerTimeManager::get()->GetGameTime();

		auto node = AllocateNode(p_info, now);

		scoped_lt_lock lock(_lock);

		auto result = _players.emplace(node->base.no, node);
		if (!result.second) {
			FreeNode(node);
			return RESULT_MATCHING_ADD_DUPLICATED;
		}

		_add_queue.insert(result.first->second);
		_add_del_requested++;

		logmsg("[MATCH] add player({:05}), [{:06}]", node->base.no, node->base.point);

		return RESULT_OK;
	}

	result_t MatchingMgr::Request_DelPlayer(PlayerNo p_no, bool& directly_deleted)
	{
		directly_deleted = false;

		scoped_lt_lock lock(_lock);

		auto result = _players.find(p_no);
		if (result == _players.end()) {
			return RESULT_MATCHING_DEL_NOT_REQUESTED;
		}

		auto node = result->second;
		if (node->del_reseved) {
			return RESULT_MATCHING_DEL_DUPLICATED;
		}
		if (node->game_matched) {
			return RESULT_MATCHING_DEL_ALREADY_MATCHED;
		}

		if (!node->in_matching) {
			if (!_add_queue.contains(node)) {
				return RESULT_UNKNOWN;
			}
			// add_queue 에 들어 있는 경우 즉시 삭제한다.
			_players.erase(p_no);
			_add_queue.erase(node);

			logmsg("[MATCH] del player({:05})", node->base.no);

			FreeNode(node);
			directly_deleted = true;
			return RESULT_OK;
		}

		// del이 예약되지만 매칭이 될 수도 있다.
		// 때문에 del이 확정될 때 cancel 패킷을 보낸다.
		node->del_reseved = 1;

		_del_queue.push_back(node);
		_add_del_requested++;

		return RESULT_OK;
	}

	void MatchingMgr::OnSchedule(systime_t now)
	{
		auto gametime = ServerTimeManager::get()->GetGameTime(now);

		_stat_cycles++;

		PrepareProcqueue();

		RebuildSortedList(gametime);

		if (_sorted_regist.size() >= MAX_MATCHING_PLAYER_COUNT) {
			ProgressMatching(gametime);
		}

		/*if (_stat_cycles % 10 == 0)*/ {
			GamePoint oldest_pt = 0;
			uint32_t oldest_wait_time = 0;
			if (!_sorted_regist.empty()) {
				auto& oldest = _sorted_regist.front();
				oldest_pt = oldest->base.point;
				oldest_wait_time = (gametime / 1000) - oldest->regist_time;
			}
			logmsg("[MATCH] cycle={}, matched={}, remain={}, oldest={}pt {}sec \r\n",
				_stat_cycles, _stat_matched, _stat_remain, oldest_pt, oldest_wait_time);
		}
	}

	void MatchingMgr::PrepareProcqueue()
	{
		if (_add_del_requested) {
			scoped_lt_lock slock(_lock);

			if (!_add_queue.empty() || !_del_queue.empty()) {
				// 추가는 그대로 스왑
				_add_proc_queue.swap(_add_queue);

				// 삭제는 매칭완료된 유저 데이터가 남아 있을 수 있다.
				if (!_del_proc_queue.empty()) {
					for (auto elm : _del_queue) {
						// 매칭완료된 경우 중복 포함되지 않도록 한다.
						if (!elm->game_matched) {
							_del_proc_queue.push_back(elm);
						}
					}
					_del_queue.clear();
				}
				else {
					_del_proc_queue.swap(_del_queue);
				}

				for (auto elm : _add_proc_queue) {
					elm->in_matching = 1;
				}
				for (auto elm : _del_proc_queue) {
					_players.erase(elm->base.no);
				}
			}
			_add_del_requested = 0;
		}
		else if (!_del_proc_queue.empty()) {
			scoped_lt_lock slock(_lock);
			for (auto p : _del_proc_queue) {
				_players.erase(p->base.no);
			}
		}
	}

	inline void __AddTo_TempList_Pt(vector_t<mathcing_player_node*>& list, mathcing_player_node* node)
	{
		node->point_order = (int32_t)list.size();
		list.push_back(node);
	}
	inline void __AddTo_TempList_Rg(vector_t<mathcing_player_node*>& list, mathcing_player_node* node, uint32_t now_sec)
	{
		node->regist_order = (int32_t)list.size();
		node->point_bound = GetPointBound_ByRegistTime(now_sec - node->regist_time);

		list.push_back(node);
	}

	void MatchingMgr::RebuildSortedList(systime_t now)
	{
		auto now_sec = (uint32_t)(now / 1000);

		if (_add_proc_queue.empty() && _del_proc_queue.empty()) {
			for (auto& node : _sorted_regist) {
				node->point_bound = GetPointBound_ByRegistTime(now_sec - node->regist_time);
			}
			return;
		}

		_replacer.reserve(_sorted_point.size() - _del_proc_queue.size() + _add_proc_queue.size());

		// 포인트 리스트는 명확하게 비교해서 순서대로 정렬시칸다.
		{
			auto it = _add_proc_queue.begin();
			auto it_end = _add_proc_queue.end();
			auto cmp_pt = (it == it_end) ? n_max<GamePoint>() : (*it)->base.point;

			for (auto& node : _sorted_point) {
				if (node->del_reseved || node->game_matched) {
					continue;
				}
				while (cmp_pt < node->base.point) {
					__AddTo_TempList_Pt(_replacer, *it);
					it++;
					cmp_pt = (it == it_end) ? n_max<GamePoint>() : (*it)->base.point;
				}
				__AddTo_TempList_Pt(_replacer, node);
			}
			for (; it != it_end; it++) {
				__AddTo_TempList_Pt(_replacer, *it);
			}

			_sorted_point.swap(_replacer);
			_replacer.clear();
		}

		// 시간 기준 정렬은 한 사이클에 추가되는 유저들이 차이가 크지않기 때문에 그대로 뒤에 추가한다.
		{
			for (auto& node : _sorted_regist) {
				if (node->del_reseved || node->game_matched) {
					continue;
				}
				__AddTo_TempList_Rg(_replacer, node, now_sec);
			}
			for (auto& node : _add_proc_queue) {
				__AddTo_TempList_Rg(_replacer, node, now_sec);
			}
			_sorted_regist.swap(_replacer);
			_replacer.clear();
		}

		_add_proc_queue.clear();
		for (auto node : _del_proc_queue) {
			if (!node->game_matched) {
				// 매칭 취소 확정.
				logmsg("[MATCH] canceld player({:05})", node->base.no);
				Send_Canceled(node);
			}
			FreeNode(node);
		}
		_del_proc_queue.clear();
	}

	struct MatchingContext
	{
		mathcing_player_node* matched_nodes[MAX_MATCHING_PLAYER_COUNT] = {};
		int32_t matched_count = 0;

		void Add(mathcing_player_node* node)
		{
			matched_nodes[matched_count] = node;
			matched_count++;
		}
		bool CheckMatchable(GamePoint op_pt, GamePoint op_pt_bnd)
		{
			for (int32_t i = 0; i < matched_count; i++)
			{
				auto node = matched_nodes[i];
				auto my_pt = node->base.point;
				if (my_pt < op_pt - op_pt_bnd || my_pt > op_pt + op_pt_bnd) {
					return false;
				}

				auto my_pt_bnd = node->point_bound;
				if (op_pt < my_pt - my_pt_bnd || op_pt > my_pt + my_pt_bnd) {
					return false;
				}
			}
			return true;
		}
		GamePoint GetAveragePoint() const
		{
			if (matched_count > 0)
			{
				GamePoint total = 0;
				for (int32_t i = 0; i < matched_count; i++) {
					total += matched_nodes[i]->base.point;
				}
				return total / matched_count;
			}
			else
			{
				return 0;
			}
		}
	};
	void MatchingMgr::ProgressMatching(systime_t now)
	{
		_stat_remain = (int32_t)_sorted_regist.size();

		MatchingContext ctx;

		// 오래된 플레이어부터 우선권을 주어 매칭 비교를 한다.
		for (auto my_node_reg : _sorted_regist) {
			if (my_node_reg->game_matched) {
				continue;
			}

			ctx.matched_count = 0;

			auto my_pt = my_node_reg->base.point;
			auto my_pt_bnd = my_node_reg->point_bound;

			// 포인트 정렬 리스트에서 현재 플레이어 기준으로 좌(less), 우(high)를
			// 순차적으로 점수를 비교해서 매칭 여부를 확인한다.
			auto center_idx = my_node_reg->point_order;
			auto pt_list_size = (int)_sorted_point.size();
			bool no_more_lesser = false;
			bool no_more_higher = false;
			for (int i = 0; i < pt_list_size; i++) {
				int dir = (i & 1); // 0, 1, 0, 1, ...
				int dist = ((i / 2) + 1) * (1 - dir * 2); // 1, -1, 2, -2, 3, -3, ...
				int op_idx = center_idx + dist;
				if (op_idx < 0) {
					no_more_lesser = true;
				}
				else if (op_idx >= pt_list_size) {
					no_more_higher = true;
				}
				else {
					bool is_matched = false;
					auto& op_node_pt = _sorted_point[op_idx];
					auto op_pt = op_node_pt->base.point;
					auto op_pt_bnd = op_node_pt->point_bound;
					if (dist < 0) {
						// ... op <- my -> ....
						if (op_pt < my_pt - my_pt_bnd) {
							no_more_lesser = true;
						}
						else if (op_node_pt->game_matched || op_pt + op_pt_bnd < my_pt) {
						}
						else if (ctx.CheckMatchable(op_pt, op_pt_bnd)) {
							is_matched = true;
						}
					}
					else {
						// ... <- my -> op ....
						if (my_pt + my_pt_bnd < op_pt) {
							no_more_higher = true;
						}
						else if (op_node_pt->game_matched || my_pt < op_pt - op_pt_bnd) {
						}
						else if (ctx.CheckMatchable(op_pt, op_pt_bnd)) {
							is_matched = true;
						}
					}
					if (is_matched) {
						ctx.Add(op_node_pt);

						if (ctx.matched_count + 1 >= MAX_MATCHING_PLAYER_COUNT) {
							ctx.Add(_sorted_point[center_idx]);
							OnMatchedGame(ctx.matched_nodes, ctx.matched_count, ctx.GetAveragePoint());
							break;
						}
					}
				}

				if (no_more_lesser && no_more_higher) {
					break;
				}
			}
		}
	}

	void MatchingMgr::OnMatchedGame(mathcing_player_node** nodes, int32_t count, GamePoint avgPoint)
	{
		auto new_game_id = ++_game_id_alloc;
		logmsg("[MATCH] matched game({:05}) [{:06}]", new_game_id, avgPoint);

		for (int j = 0; j < count; j++) {
			auto node = nodes[j];
			node->game_matched = 1;
			_del_proc_queue.push_back(node);
			logmsg("[MATCH] matched game({:05}) [{:06} +-{:04}] player({:05})", new_game_id, node->base.point, node->point_bound, node->base.no);
		}
		_stat_matched += count;
		_stat_remain -= count;
	}

	mathcing_player_node* MatchingMgr::AllocateNode(const player_info& p, systime_t now) const
	{
		return new mathcing_player_node {
			.base = p,
			.regist_time = uint32_t(now / 1000),
		};
	}
	void MatchingMgr::FreeNode(mathcing_player_node* node) const
	{
		delete node;
	}

	void MatchingMgr::Send_Canceled(mathcing_player_node* node)
	{

	}
}