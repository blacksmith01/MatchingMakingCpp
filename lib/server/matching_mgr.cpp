#include "proj\pch.h"
#include "server\matching_mgr.h"

#include "server\gameroom_mgr.h"

namespace myproj
{
	bool MatchingMgr::Init()
	{
		if (!Scheduler::get()->Add(this, tick_second)) {
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
		if (!IsValidPlayerNo(p_info.no) || !IsValidGamePoint(p_info.point) || !IsValidPlayerGrade(p_info.grade)
			|| p_info.grade != GetPlayerGrade_ByGamePoint(p_info.point)) {
			return RESULT_MATCHING_ADD_INVALID_REQUEST;
		}

		auto now = tickcnt_now();

		auto node = AllocateNode(p_info, now);

		scoped_lt_lock lock(_lock);

		auto result = _players.emplace(node->base.no, node);
		if (!result.second) {
			FreeNode(node);
			return RESULT_MATCHING_ADD_DUPLICATED;
		}

		_add_queue.insert(result.first->second);
		_add_del_requested++;

		logmsg("[MATCH] add player, {:05}[{:06}]", node->base.no, node->base.point);

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
		if (node->matched) {
			return RESULT_MATCHING_DEL_ALREADY_MATCHED;
		}

		if (!node->in_matching) {
			if (!_add_queue.contains(node)) {
				return RESULT_UNKNOWN;
			}
			// add_queue �� ��� �ִ� ��� ��� �����Ѵ�.
			_players.erase(p_no);
			_add_queue.erase(node);

			logmsg("[MATCH] del player, {:05}", node->base.no);

			FreeNode(node);
			directly_deleted = true;
			return RESULT_OK;
		}

		// del�� ��������� ��Ī�� �� ���� �ִ�.
		// ������ del�� Ȯ���� �� cancel ��Ŷ�� ������.
		node->del_reseved = 1;

		_del_queue.push_back(node);
		_add_del_requested++;

		return RESULT_OK;
	}

	void MatchingMgr::OnSchedule(tickcnt_t now)
	{
		_stat_cycles++;

		PrepareInqueue();

		if (!_add_in_queue.empty() || !_del_in_queue.empty()) {
			RebuildSortedList();
		}

		if (_sorted_regist.size() >= MAX_MATCHING_PLAYER_COUNT) {
			ProgressMatching(now);
		}

		if (_stat_cycles % 10 == 0 && !_sorted_regist.empty()) {
			logmsg("[MATCH] cycle={}, matched={}, remain={}, oldest={}sec \r\n",
				_stat_cycles, _stat_matched, _stat_remain, (now - _sorted_regist.front()->regist_time) / 1000);
		}

		Scheduler::get()->Add(this, tick_second);
	}

	void MatchingMgr::PrepareInqueue()
	{
		if (_add_del_requested) {
			scoped_lt_lock slock(_lock);

			if (!_add_queue.empty() || !_del_queue.empty()) {
				// �߰��� �״�� ����
				_add_in_queue.swap(_add_queue);

				// ������ ��Ī�Ϸ�� ���� �����Ͱ� ���� ���� �� �ִ�.
				if (_del_in_queue.empty()) {
					_del_in_queue.swap(_del_queue);
				}
				else {
					for (auto p : _del_queue) {
						// del_reseved �Ǿ� ������ ��Ī�Ϸ�Ǿ� �̹� ���ԵǾ� ���� �� �ִ�.
						if (!p->matched) {
							_del_in_queue.push_back(p);
						}
					}
					_del_queue.clear();
				}

				for (auto p : _add_in_queue) {
					p->in_matching = 1;
				}
				for (auto p : _del_in_queue) {
					_players.erase(p->base.no);
				}
			}
			_add_del_requested = 0;
		}
		else if (_del_in_queue.empty()) {
			scoped_lt_lock slock(_lock);
			for (auto p : _del_in_queue) {
				_players.erase(p->base.no);
			}
		}
	}

	inline void __AddTo_TempList_Point(vector_t<mathcing_player_node*>& list, mathcing_player_node* node)
	{
		node->point_order = (int32_t)list.size();
		list.push_back(node);
	}
	inline void __AddTo_TempList_Regist(vector_t<mathcing_player_node*>& list, mathcing_player_node* node)
	{
		node->regist_order = (int32_t)list.size();
		list.push_back(node);
	}

	void MatchingMgr::RebuildSortedList()
	{
		_replacer.reserve(_sorted_point.size() - _del_in_queue.size() + _add_in_queue.size());

		// ����Ʈ ����Ʈ�� ��Ȯ�ϰ� ���ؼ� ������� ���Ľ�ĭ��.
		{
			auto it = _add_in_queue.begin();
			auto it_end = _add_in_queue.end();
			auto pt_it = (it == it_end) ? n_max<GamePoint>() : (*it)->base.point;

			for (auto& p : _sorted_point) {
				if (p->del_reseved || p->matched) {
					continue;
				}
				while (pt_it < p->base.point) {
					__AddTo_TempList_Point(_replacer, *it);
					it++;
					pt_it = (it == it_end) ? n_max<GamePoint>() : (*it)->base.point;
				}
				__AddTo_TempList_Point(_replacer, p);
			}
			for (; it != it_end; it++) {
				__AddTo_TempList_Point(_replacer, *it);
			}

			_sorted_point.swap(_replacer);
			_replacer.clear();
		}

		// �ð� ���� ������ �� ����Ŭ�� �߰��Ǵ� �������� ���̰� ũ���ʰ� ������ �ʿ䰡 �����Ƿ� �״�� �ڿ� �߰��Ѵ�.
		{
			for (auto& p : _sorted_regist) {
				if (p->del_reseved || p->matched) {
					continue;
				}
				__AddTo_TempList_Regist(_replacer, p);
			}
			for (auto& p : _add_in_queue) {
				__AddTo_TempList_Regist(_replacer, p);
			}
			_sorted_regist.swap(_replacer);
			_replacer.clear();
		}

		_add_in_queue.clear();
		for (auto p : _del_in_queue) {
			if (!p->matched) {
				// ��Ī ��� Ȯ��.
				logmsg("[MATCH] canceld player, {:05}", p->base.no);
				Send_Canceled(p);
			}
			FreeNode(p);
		}
		_del_in_queue.clear();
	}

	void MatchingMgr::ProgressMatching(tickcnt_t now)
	{
		_stat_remain = (int32_t)_sorted_regist.size();

		// ������ �÷��̾���� �켱���� �־� ��Ī �񱳸� �Ѵ�.
		for (auto& p : _sorted_regist) {
			if (p->matched) {
				continue;
			}

			auto my_pt = p->base.point;
			auto my_pt_bound = GetPointBound_ByRegistTime(p, now);
			mathcing_player_node* matched_nodes[MAX_MATCHING_PLAYER_COUNT] = {};
			GamePoint pt_accumulated = 0;
			int matched_count = 0;

			// ����Ʈ ���� ����Ʈ���� ���� �÷��̾� �������� ��(less), ��(high)��
			// ���������� ������ ���ؼ� ��Ī ���θ� Ȯ���Ѵ�.
			auto center_idx = p->point_order;
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
					auto op = _sorted_point[op_idx];
					auto op_pt = op->base.point;
					auto op_pt_bound = GetPointBound_ByRegistTime(op, now);
					if (dist < 0) {
						// ... op <- my -> ....
						if (op_pt < my_pt - my_pt_bound) {
							no_more_lesser = true;
						}
						else if (op->matched || my_pt > op_pt + op_pt_bound) {
						}
						else {
							is_matched = true;
						}
					}
					else {
						// ... <- my -> op ....
						if (op_pt > my_pt + my_pt_bound) {
							no_more_higher = true;
						}
						else if (op->matched || my_pt < op_pt - op_pt_bound) {
						}
						else {
							is_matched = true;
						}
					}
					if (is_matched) {
						matched_nodes[matched_count] = op;
						matched_count++;
						pt_accumulated += op->base.point;

						if (matched_count + 1 >= MAX_MATCHING_PLAYER_COUNT) {
							matched_nodes[matched_count] = p;
							matched_count++;
							pt_accumulated += p->base.point;

							logmsg("[MATCH] matched game [{:06}]", pt_accumulated / MAX_MATCHING_PLAYER_COUNT);

							array_t<player_info*, MAX_MATCHING_PLAYER_COUNT> matched_info;
							for (int j = 0; j < MAX_MATCHING_PLAYER_COUNT; j++) {
								auto matched_node = matched_nodes[j];
								matched_info[j] = &matched_node->base;
								matched_node->matched = 1; // ���� �����층 ������Ʈ���� �����Ѵ�.
								_del_in_queue.push_back(matched_node);

								logmsg("[MATCH] suc player, {:05}", matched_node->base.no);
							}
							GameRoomMgr::get()->AddRoom(*matched_info.data(), MAX_MATCHING_PLAYER_COUNT);
							_stat_matched += MAX_MATCHING_PLAYER_COUNT;
							_stat_remain -= MAX_MATCHING_PLAYER_COUNT;
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

	mathcing_player_node* MatchingMgr::AllocateNode(const player_info& p, tickcnt_t now)
	{
		return new mathcing_player_node{
			.base = p,
			.regist_time = now,
		};
	}
	void MatchingMgr::FreeNode(mathcing_player_node* node)
	{
		delete node;
	}

	GamePoint MatchingMgr::GetPointBound_ByRegistTime(mathcing_player_node* node, tickcnt_t now)
	{
		auto elapsed_sec = (now - node->regist_time) / 1000;
		if (elapsed_sec < MATCHING_PHASE_1_TIME) {
			return MATCHING_PHASE_1_BOUNDS;
		}
		else if (elapsed_sec < MATCHING_PHASE_2_TIME) {
			return MATCHING_PHASE_2_BOUNDS;
		}
		else if (elapsed_sec < MATCHING_PHASE_3_TIME) {
			return MATCHING_PHASE_3_BOUNDS;
		}
		else { // if (elapsed_sec < MATCHING_PHASE_4_TIME) {
			return MATCHING_PHASE_4_BOUNDS;
		}
	}

	void MatchingMgr::Send_Canceled(mathcing_player_node* node)
	{

	}
}