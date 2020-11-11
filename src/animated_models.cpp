#include <wpl/animated_models.h>

using namespace std;

namespace wpl
{
	namespace
	{
		const double c_max_squeeze = 0.01;
	}

	animated_scroll_model::animated_scroll_model(shared_ptr<scroll_model> underlying, const clock &/*clock_*/, const queue &/*queue_*/)
		: _underlying(underlying)
	{
		//_invalidate_connection = _underlying->invalidated += [this] {
		//	this->invalidated();
		//};
	}

	pair<double, double> animated_scroll_model::get_range() const
	{	return _underlying->get_range();	}

	pair<double, double> animated_scroll_model::get_window() const
	{
		const auto r = _underlying->get_range();
		auto uw = _underlying->get_window();

		if (uw.first < r.first)
		{
			uw.second = max(c_max_squeeze * uw.second, uw.first + uw.second - r.first);
			uw.first = r.first;
		}
		else if (uw.first + uw.second > r.first + r.second)
		{
			uw.second = max(c_max_squeeze * uw.second, r.first + r.second - uw.first);
			uw.first = r.first + r.second - uw.second;
		}
		return uw;
	}

	double animated_scroll_model::get_increment() const
	{	return _underlying->get_increment();	}

	void animated_scroll_model::scrolling(bool begins)
	{
		// UNTESTED
		_underlying->scrolling(begins);
	}

	void animated_scroll_model::scroll_window(double window_min, double /*window_width*/)
	{
		// UNTESTED
		const auto r = _underlying->get_range();
		const auto uw = _underlying->get_window();

		if (window_min < r.first)
		{
			double d = (r.first - window_min) / uw.second;

			window_min = r.first - uw.second + uw.second / (1 + d);
		}
		else if (window_min + uw.second >= r.first + r.second)
		{
			double d = (window_min + uw.second - r.first - r.second) / uw.second;

			window_min = r.first + r.second - uw.second / (1 + d);
		}
		_underlying->scroll_window(window_min, uw.second);
	}
}
