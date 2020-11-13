#pragma once

#include <wpl/queue.h>
#include <queue>

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			struct queue_task
			{
				wpl::queue_task task;
				timespan defer_by;
			};

			typedef std::queue<queue_task> queue_container;

			inline wpl::queue create_queue(queue_container &container)
			{
				return [&container] (const wpl::queue_task &task, timespan defer_by) -> bool {
					queue_task t = { task, defer_by };

					container.push(t);
					return true;
				};
			}
		}
	}
}
