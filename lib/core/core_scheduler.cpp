#include "proj/pch.h"
#include "core/core_scheduler.h"

namespace mylib
{
	bool Scheduler::Init()
	{
		_top_time = std::numeric_limits<systime_t>::max();
		Add(&_dummy, systime_day*356*10);

		return true;
	}

	void Scheduler::Cleanup()
	{
		Shutdown();
	}

	void Scheduler::Run()
	{
		{
			scoped_lt_lock slock(_lock);
			if (_is_request_run) {
				return;
			}
			_is_request_run = true;
		}

		std::thread worker([]() {Scheduler::get()->OnThread(); });

		{
			scoped_lt_lock slock(_lock);
			_cv.wait(slock, [this] {return _is_ready_workthread; });
		}

		worker.detach();
	}

	void Scheduler::Shutdown()
	{
		scoped_lt_lock slock(_lock);
		if (!_is_request_shutdown) {
			_is_request_shutdown = true;
			_cv.notify_all();

			_cv.wait(slock, [this] {return _is_shutdown_complete; });

			_queue = {};
		}
	}

	bool Scheduler::Add(Schedulable* obj, systime_t period)
	{
		scoped_lt_lock slock(_lock);

		if (_is_request_shutdown) {
			return false;
		}

		auto time = systime_now() + period;
		obj->_period = period;
		obj->_time = time;
		_queue.push(obj);

		if (time < _top_time) {
			_top_time = time;
			_cv.notify_one();
		}

		return true;
	}

	void Scheduler::OnThread()
	{
		_is_ready_workthread = true;
		_cv.notify_one();

		scoped_lt_lock slock(_lock);

		while (!_is_request_shutdown) {
			auto now = systime_now();
			while (now > _top_time) {
				auto top = _queue.top();
				auto period = top->_period;
				ThreadMgr::get()->AddItem([top, now, period]() {
					top->OnSchedule(now);
					Scheduler::get()->Add(top, period);
				});
				_queue.pop();
				
				top = _queue.top();
				_top_time = top->_time;
			}
			
			_cv.wait_for(slock, std::chrono::milliseconds(_top_time - now));
		}

		_is_shutdown_complete = true;
		_cv.notify_all();
	}
}