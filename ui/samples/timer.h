#pragma once

#include <functional>
#include <memory>

namespace wpl
{
	std::shared_ptr<void> create_timer(unsigned timeout, const std::function<void(unsigned elapsed)> &callback);
}
