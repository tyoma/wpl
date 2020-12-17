#pragma once

#include <wpl/visual.h>

namespace wpl
{
	struct stylesheet;

	std::shared_ptr<stylesheet> create_sample_stylesheet(std::shared_ptr<gcontext::text_engine_type> text_engine);
}
