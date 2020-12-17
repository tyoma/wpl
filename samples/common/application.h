#pragma once

#include <memory>
#include <wpl/queue.h>

namespace wpl
{
	class factory;

	class application
	{
	public:
		application();
		~application();

		std::shared_ptr<factory> create_default_factory() const;
		queue get_application_queue() const;

		void run();
		void exit();
		
	private:
		class impl;
		
	private:
		std::unique_ptr<impl> _impl;
		queue _queue;
	};
}
