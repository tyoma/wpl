#pragma once

#include <functional>
#include <wpl/models.h>

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			class scroll_model : public wpl::scroll_model
			{
			public:
				scroll_model()
					: increment(1)
				{	}

				virtual std::pair<double /*[min*/, double /*max)*/> get_range() const
				{	return range;	}

				virtual std::pair<double /*[window_min*/, double /*window_width)*/> get_window() const
				{	return window;	}

				virtual double get_increment() const
				{	return increment;	}

				virtual void scrolling(bool begins)
				{
					if (on_scrolling)
						on_scrolling(begins);
				}

				virtual void scroll_window(double window_min, double window_width)
				{
					if (on_scroll)
						on_scroll(window_min, window_width);
				}

				std::function<void (bool begins)> on_scrolling;
				std::function<void (double window_min, double window_width)> on_scroll;

			public:
				std::pair<double, double> range, window;
				double increment;
			};
		}
	}
}
