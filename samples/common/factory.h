#pragma once

#include <memory>

namespace wpl
{
	class factory;

	std::shared_ptr<factory> create_sample_factory();
}
