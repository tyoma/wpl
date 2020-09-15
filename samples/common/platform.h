#pragma once

#include <wpl/visual.h>

namespace wpl
{
	std::shared_ptr<gcontext::text_engine_type> create_text_engine();

	void run_message_loop();
	void exit_message_loop();
}
