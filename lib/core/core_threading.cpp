#include "proj\pch.h"
#include "core\core_threading.h"

namespace mylib
{
	bool ThreadMgr::Init()
	{
		_max_thread_count = 4;

		_nodes = new WorkItemQueueNode[_max_thread_count];

		return true;
	}

	void ThreadMgr::Cleanup()
	{
		Shutdown();

		if (_nodes) {
			delete[] _nodes;
			_nodes = nullptr;
		}
	}

	void ThreadMgr::Run()
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

	void ThreadMgr::Shutdown()
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

	void ThreadMgr::AddItem(WorkItem item)
	{
		int idx = (++_queue_toggle) % _max_thread_count;
		auto& node = _nodes[idx];

		scoped_lt_lock slock(node.lock);
		node.queue.push_back(item);
		node.cv.notify_one();
	}

	void ThreadMgr::OnThread(int32_t thread_idx)
	{
		{
			scoped_lt_lock slock(_lock);
			_thread_run++;
			_cv.notify_all();
		}

		auto& node = _nodes[thread_idx];

		vector_t<WorkItem> queue_swap;
		queue_swap.reserve(128);
		while (!_is_request_shutdown) {
			{
				scoped_lt_lock slock(node.lock);
				_cv.wait_for(slock, std::chrono::milliseconds(100), [&node]() { return !node.queue.empty(); });
				queue_swap.swap(node.queue);
			}

			for (auto item : queue_swap) {
				item();
			}
			queue_swap.clear();
		}
	}
}