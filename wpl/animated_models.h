#pragma once

#include "models.h"
#include "queue.h"

namespace wpl
{
	class animated_scroll_model : public scroll_model
	{
	public:
		animated_scroll_model(std::shared_ptr<scroll_model> underlying, const clock &clock_, const queue &queue_);

		virtual std::pair<double, double> get_range() const;
		virtual std::pair<double, double> get_window() const;
		virtual double get_increment() const;
		virtual void scrolling(bool begins);
		virtual void scroll_window(double window_min, double window_width);

	private:
		std::shared_ptr<scroll_model> _underlying;
		slot_connection _invalidate_connection;
	};
}
