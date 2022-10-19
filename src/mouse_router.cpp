//	Copyright (c) 2011-2022 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#include <wpl/mouse_router.h>

#include <wpl/view.h>

using namespace std;

namespace wpl
{
	mouse_router::mouse_router(const vector<placed_view> &views, mouse_router_host &host)
		: _views(views), _host(host)
	{	}

	void mouse_router::reload_views()
	{
		size_t index = 0;

		_connections.clear();
		for (auto i = _views.begin(); i != _views.end(); ++i, ++index)
		{
			if (const auto m = i->regular)
			{
				_connections.push_back(m->capture += [this, index] (shared_ptr<void> &handle) {
					const auto p = make_shared<mouse_router::capture_target>(_host.capture_mouse(), index);

					handle = p;
					_capture_target = p;
				});
			}
		}
	}

	shared_ptr<view> mouse_router::from(agge::point<int> &point) const
	{
		if (const auto capture = _capture_target.lock())
		{
			const auto &pv = _views[capture->second];

			return point.x -= pv.location.x1, point.y -= pv.location.y1, pv.regular;
		}
		for (auto i = _views.rbegin(); i != _views.rend(); ++i)
		{
			const auto &w = i->location;

			if (w.x1 <= point.x && point.x < w.x2 && w.y1 <= point.y && point.y < w.y2)
				return point.x -= w.x1, point.y -= w.y1, i->regular;
		}
		return shared_ptr<view>();
	}

	void mouse_router::mouse_leave()
	{
		if (_mouse_over)
		{
			_mouse_over->mouse_leave();
			_mouse_over = shared_ptr<view>();
		}
	}

	void mouse_router::mouse_move(int depressed, agge::point<int> point)
	{
		if (const auto m = switch_mouse_over(point))
			m->mouse_move(depressed, point.x, point.y);
	}

	void mouse_router::mouse_click(void (mouse_input::*fn)(mouse_input::mouse_buttons button, int depressed,
		int x, int y), mouse_input::mouse_buttons button_, int depressed, agge::point<int> point)
	{
		if (const auto m = switch_mouse_over(point))
		{
			_host.request_focus(m);
			((*m).*fn)(button_, depressed, point.x, point.y);
		}
	}

	void mouse_router::mouse_scroll(int depressed, agge::point<int> point, int delta_x, int delta_y)
	{
		if (const auto m = switch_mouse_over(point))
			m->mouse_scroll(depressed, point.x, point.y, delta_x, delta_y);
	}

	shared_ptr<view> mouse_router::switch_mouse_over(agge::point<int> &point)
	{
		const auto mouse_over = from(point);

		if (mouse_over != _mouse_over)
		{
			if (_mouse_over)
				_mouse_over->mouse_leave();
			if (mouse_over)
				mouse_over->mouse_enter();
			_mouse_over = mouse_over;
		}
		return mouse_over;
	}
}
