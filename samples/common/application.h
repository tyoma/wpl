#pragma once

#include <memory>

namespace wpl
{
	class application
	{
	public:
		application();
		~application();
		
		void run();
		void exit();
		
	private:
		class impl;
		
	private:
		std::unique_ptr<impl> _impl;
	};
}
