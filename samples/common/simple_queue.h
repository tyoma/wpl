#pragma once

#include <memory>
#include <wpl/concepts.h>
#include <wpl/queue.h>

namespace wpl
{
	class simple_queue : noncopyable
	{
	public:
		simple_queue();
		~simple_queue();

		bool schedule(const queue_task &task, timespan defer_by);

	private:
		struct impl;

	private:
		std::unique_ptr<impl> _impl;
	};
}
