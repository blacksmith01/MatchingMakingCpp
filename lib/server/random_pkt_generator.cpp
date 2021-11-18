#include "proj\pch.h"
#include "server\random_pkt_generator.h"

#include "server\matching_mgr.h"

namespace myproj
{
	bool RandomPktGenerator::Init()
	{
		_max_thread_count = 4;

		return true;
	}

	void RandomPktGenerator::Cleanup()
	{
		Shutdown();
	}

	void RandomPktGenerator::Run()
	{
		{
			scoped_lt_lock slock(_lock);
			if (_is_request_run) {
				return;
			}
			_is_request_run = true;
		}

		for (int i = 0; i < _max_thread_count; i++) {
			std::thread th([this, i]() { OnThread(i); });
			th.detach();
		}

		while (true) {
			scoped_lt_lock slock(_lock);
			_cv.wait(slock, [this]() { return _thread_run == _max_thread_count; });
			if (_thread_run == _max_thread_count) {
				break;
			}
		}
	}

	void RandomPktGenerator::Shutdown()
	{
		scoped_lt_lock slock(_lock);
		if (_is_request_shutdown) {
			return;
		}
		_is_request_shutdown = true;

		while (true) {
			_cv.wait(slock, [this]() { return _thread_shutdown == 4; });
			if (_thread_shutdown == 4) {
				break;
			}
		}
	}

	void RandomPktGenerator::OnThread(int32_t thread_idx)
	{
		{
			scoped_lt_lock slock(_lock);
			_thread_run++;
			_cv.notify_all();
		}

		PlayerNo last_player_no = 0;
		PlayerNo max_player_per_thread = 10000;
		uint32_t count = 0;

		std::default_random_engine rand(thread_idx);

		while (!_is_request_shutdown) {
			auto rval = rand();
			uint32_t sleep_time_ms = 200 + ((uint32_t)rval % 800);
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));

			if (last_player_no > 0 && ((uint32_t)rval % 10) == 0) {
				Send_Del(last_player_no);
				last_player_no = 0;
			}
			else {
				player_info p_info;
				p_info.no = thread_idx * max_player_per_thread + ((++count) % max_player_per_thread);
				p_info.point = rval % (GAME_POINT_GRADE_START_GOLD + GAME_POINT_GRADE_START_GOLD / 10);
				p_info.grade = GetPlayerGrade_ByGamePoint(p_info.point);
				Send_Add(p_info);
				last_player_no = p_info.no;
			}
		}

		{
			scoped_lt_lock slock(_lock);
			_thread_shutdown++;
			_cv.notify_all();
		}
	}

	void RandomPktGenerator::Send_Add(const player_info& p_info)
	{
		ThreadMgr::get()->AddItem([p_info]()
			{
				MatchingMgr::get()->Request_AddPlayer(p_info);
			}
		);
	}

	void RandomPktGenerator::Send_Del(PlayerNo p_no)
	{
		ThreadMgr::get()->AddItem([p_no]()
			{
				bool directly_deleted;
				MatchingMgr::get()->Request_DelPlayer(p_no, directly_deleted);
			}
		);
	}
}