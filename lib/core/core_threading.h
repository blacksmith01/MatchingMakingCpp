#pragma once

#include "base/base_all.h"
#include "core/core_class_utils.h"

namespace mylib
{
	using WorkItem = std::function<void()>;

	struct WorkItemQueueNode
	{
		lt_lock lock;
		cond_var cv;
		vector_t<WorkItem> queue;
	};

	class ThreadMgr : public single_ptr<ThreadMgr>, public Runable
	{
	public:
		ThreadMgr() = default;
		~ThreadMgr() = default;

		bool Init();
		void Cleanup();

	public:
		void Run() override;
		void Shutdown() override;

	public:
		void AddItem(WorkItem item);

	private:
		void OnThread(int32_t thread_idx);

	private:

		lt_lock _lock;
		cond_var _cv;

		int32_t _max_thread_count = {};
		int32_t _thread_run;
		int32_t _thread_shutdown;

		bool _is_request_run = {};
		bool _is_request_shutdown = {};

		std::atomic_uint32_t _queue_toggle = {};
		WorkItemQueueNode* _nodes = nullptr;
	};
}