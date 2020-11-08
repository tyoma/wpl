#pragma once

#include "models.h"

#include <tq/queue.h>

namespace wpl
{
	class animated_scroll_model : public scroll_model
	{
	public:
		animated_scroll_model(std::shared_ptr<scroll_model> underlying, tq::queue::ptr queue);

		virtual std::pair<double, double> get_range() const;
		virtual std::pair<double, double> get_window() const;
		virtual double get_increment() const;
		virtual void scrolling(bool begins);
		virtual void scroll_window(double window_min, double window_width);

	private:
		std::shared_ptr<scroll_model> _underlying;
		tq::queue::ptr _queue;
		slot_connection _invalidate_connection;
	};
}
