#pragma once

#include "base/base_all.h"
#include "core/core_class_utils.h"

namespace mylib
{
	class Scheduler;
	class Schedulable
	{
	public:
		virtual void OnSchedule(systime_t now) = 0;

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
		systime_t _period = {};
		systime_t _time = {};

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
		bool Add(Schedulable* obj, systime_t period);

	private:
		void OnThread();


	private:
		lt_lock _lock;
		cond_var _cv;

		volatile bool _is_request_run = {};
		volatile bool _is_ready_workthread = {};
		volatile bool _is_request_shutdown = {};
		volatile bool _is_shutdown_complete = {};

		pr_queue<Schedulable*,vector_t<Schedulable*>,greater_ptr<Schedulable>> _queue;
		systime_t _top_time = {};

		class DummyItem : public Schedulable  // queue 가 empty되지 않도록 보장.
		{
		public:
			void OnSchedule(systime_t now) {}
		};
		DummyItem _dummy;
	};
}