#pragma once

#include <functional>
#include <wpl/ui/models.h>

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace mocks
			{
				class scroll_model : public wpl::ui::scroll_model
				{
				public:
					virtual std::pair<double /*[min*/, double /*max)*/> get_range() const
					{	return range;	}

					virtual std::pair<double /*[window_min*/, double /*window_width)*/> get_window() const
					{	return window;	}

					virtual void moving(bool begins)
					{
						if (on_moving)
							on_moving(begins);
					}

					virtual void move_window(double window_min, double window_width)
					{
						if (on_move)
							on_move(window_min, window_width);
					}

					std::function<void (bool begins)> on_moving;
					std::function<void (double window_min, double window_width)> on_move;

				public:
					std::pair<double, double> range, window;
				};
			}
		}
	}
}
