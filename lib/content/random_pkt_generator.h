#pragma once

#include "content/game_defines.h"

namespace myproj
{
	class RandomPktGenerator : public single_ptr<RandomPktGenerator>, public Runable
	{
	public:
		RandomPktGenerator() = default;
		~RandomPktGenerator() = default;

	public:
		bool Init();
		void Cleanup();

	public:
		void Run() override;
		void Shutdown() override;

	private:
		void OnThread(int32_t thread_idx);

	private:
		void Send_Add(const player_info& p_info);
		void Send_Del(PlayerNo p_no);

	private:
		lt_lock _lock;
		cond_var _cv;

		int32_t _max_thread_count = {};
		int32_t _thread_run = {};
		int32_t _thread_shutdown = {};
		
		bool _is_request_run = {};
		bool _is_request_shutdown = {};
	};
}