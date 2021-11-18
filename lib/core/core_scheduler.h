#pragma once

#include "base\base_all.h"
#include "core\core_class_utils.h"

namespace mylib
{
	class Scheduler;
	class Schedulable
	{
	public:
		virtual void OnSchedule(tickcnt_t now) = 0;

	public:
		bool operator > (const Schedulable& other) const {
			if (_time == other._time) {
				return this > & other;
			}
			else {
				return _time > other._time;
			}
		}
	private:
		tickcnt_t _cycle;
		tickcnt_t _time;

		friend class Scheduler;
	};

	class Scheduler : public single_ptr<Scheduler>, public Runable
	{
	public:
		Scheduler() = default;
		~Scheduler() = default;

		bool Init();
		void Cleanup();

	public:
		void Run() override;
		void Shutdown() override;

	public:
		bool Add(Schedulable* obj, tickcnt_t cycle);

	private:
		void OnThread();


	private:
		lt_lock _lock;
		cond_var _cv;

		bool _is_request_run = {};
		bool _is_ready_workthread = {};
		bool _is_request_shutdown = {};
		bool _is_shutdown_complete = {};

		pr_queue<Schedulable*,vector_t<Schedulable*>,greater_ptr<Schedulable>> _queue;
		tickcnt_t _top_time = {};

		class DummyItem : public Schedulable  // queue 가 empty되지 않도록 보장.
		{
		public:
			void OnSchedule(tickcnt_t now) {}
		};
		DummyItem _dummy;
	};
}