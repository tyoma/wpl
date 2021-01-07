#include <wpl/animated_models.h>

#include <algorithm>

#pragma warning(disable: 4355)

using namespace std;
using namespace std::placeholders;

namespace wpl
{
	namespace
	{
		const double c_max_squeeze = 0.01;
	}

	animated_scroll_model::animated_scroll_model(shared_ptr<scroll_model> underlying, const clock &clock_,
			const queue &queue_, const animation_function &release_animation)
		: _clock(clock_), _queue(queue_), _release_animation(release_animation), _underlying(underlying),
			_invalidate_connection(underlying->invalidate += bind(&animated_scroll_model::on_invalidate, this, _1))
	{	}

	pair<double, double> animated_scroll_model::get_range() const
	{	return _underlying->get_range();	}

	pair<double, double> animated_scroll_model::get_window() const
	{
		const auto r = _underlying->get_range();
		auto uw = _underlying->get_window();

		if (uw.first < r.first)
		{
			uw.second = (max)(c_max_squeeze * uw.second, uw.first + uw.second - r.first);
			uw.first = r.first;
		}
		else if (uw.first + uw.second > r.first + r.second)
		{
			uw.second = (max)(c_max_squeeze * uw.second, r.first + r.second - uw.first);
			uw.first = r.first + r.second - uw.second;
		}
		return uw;
	}

	double animated_scroll_model::get_increment() const
	{	return _underlying->get_increment();	}

	void animated_scroll_model::scrolling(bool begins)
	{
		if (begins)
		{
			_underlying->scrolling(true);
		}
		else if (!_excess)
		{
			_underlying->scrolling(false);
		}
		else
		{
			_animation_start = _clock();
			_queue([this] {	animate();	}, 10);
		}
	}

	void animated_scroll_model::scroll_window(double window_min, double /*window_width*/)
	{
		const auto r = _underlying->get_range();
		const auto w = _underlying->get_window().second;
		const auto lbound = r.first;
		const auto ubound = r.first + r.second - w;

		if (window_min < lbound)
		{
			const auto d = 1 + (lbound - window_min) / w;

			_excess = -w + w / d;
			window_min = lbound + _excess;
		}
		else if (window_min > ubound)
		{
			const auto d = 1 + (window_min - ubound) / w;

			_excess = -w / d + w;
			window_min = ubound + _excess;
		}
		else
		{
			_excess = 0;
		}
		_underlying->scroll_window(window_min, w);
	}

	void animated_scroll_model::animate()
	{
		double progress;
		const auto r = _underlying->get_range();
		const auto uw = _underlying->get_window();
		const double target = _excess < 0 ? r.first : r.first + r.second - uw.second;
		const auto proceed = _release_animation(progress, 0.001 * (_clock() - _animation_start));

		_underlying->scroll_window(target + (1.0 - progress) * _excess, uw.second);
		if (proceed)
			_queue([this] {	animate();	}, 10);
		else
			_underlying->scrolling(false);
		invalidate(false);
	}

	void animated_scroll_model::on_invalidate(bool invalidate_range)
	{
		if (invalidate_range)
		{
			auto require_scroll = false;
			const auto r = _underlying->get_range();
			auto uw = _underlying->get_window();

			if (uw.first + uw.second > r.first + r.second)
				uw.first = r.first + r.second - uw.second, require_scroll = true;
			if (uw.first < r.first)
				uw.first = r.first, require_scroll = true;
			if (require_scroll)
				_underlying->scroll_window(uw.first, uw.second);
		}
		invalidate(invalidate_range);
	}
}
