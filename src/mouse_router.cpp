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
	namespace
	{
		const placed_view *find(const vector<placed_view> &views, const agge::point<int> &point)
		{
			for (auto i = views.rbegin(); i != views.rend(); ++i)
			{
				const auto &w = i->location;

				if (w.x1 <= point.x && point.x < w.x2 && w.y1 <= point.y && point.y < w.y2)
					return &*i;
			}
			return nullptr;
		}
	}

	mouse_router::view_and_target::operator bool() const
	{	return !!events_target;	}

	mouse_router::mouse_router(const vector<placed_view> &views, mouse_router_host &host)
		: _views(views), _host(host)
	{	}

	void mouse_router::reload_views()
	{
		size_t index = 0;

		_connections.clear();
		for (auto i = _views.begin(); i != _views.end(); ++i, ++index)
		{
			if (const auto &v = i->regular)
			{
				const auto &originator = *i;

				_connections.push_back(v->capture += [this, &originator] (shared_ptr<void> &handle, mouse_input &target) {
					typedef mouse_router::capture_target capture_target_;

					capture_target_ t = {	_host.capture_mouse(), originator, &target	};
					auto *self = this;
					const shared_ptr<capture_target_> p(new capture_target_(t), [self] (capture_target_ *p) {
						if (self->_mouse_over != p->originator.regular)
						{
							if (p->originator.regular)
								p->originator.regular->mouse_leave();
							if (self->_mouse_over)
								self->_mouse_over->mouse_enter();
						}
						delete p;
					});

					handle = p;
					_capture_target = p;
				});
			}
		}
	}

	mouse_router::view_and_target mouse_router::from(agge::point<int> &point) const
	{
		const auto match = find(_views, point);
		view_and_target result = {	match ? match->regular : nullptr, nullptr	};
		const rect_i *reference = nullptr;

		if (const auto capture = _capture_target.lock())
			result.events_target = capture->target, reference = &capture->originator.location;
		else if (match)
			result.events_target = match->regular.get(), reference = &match->location;

		if (reference)
			point.x -= reference->x1, point.y -= reference->y1;
		return result;
	}

	void mouse_router::mouse_leave()
	{
		if (/*_capture_target.expired() &&*/ _mouse_over)
			_mouse_over->mouse_leave();
		_mouse_over = shared_ptr<view>();
	}

	void mouse_router::mouse_move(int depressed, agge::point<int> point)
	{
		if (const auto m = switch_mouse_over(point))
			m.events_target->mouse_move(depressed, point.x, point.y);
	}

	void mouse_router::mouse_click(void (mouse_input::*fn)(mouse_input::mouse_buttons button, int depressed,
		int x, int y), mouse_input::mouse_buttons button_, int depressed, agge::point<int> point)
	{
		if (const auto m = switch_mouse_over(point))
		{
			_host.request_focus(m.over); // TODO: use pair - mouse_input + keyboard_input.
			((*m.events_target).*fn)(button_, depressed, point.x, point.y);
		}
	}

	void mouse_router::mouse_scroll(int depressed, agge::point<int> point, int delta_x, int delta_y)
	{
		if (const auto m = switch_mouse_over(point))
			m.events_target->mouse_scroll(depressed, point.x, point.y, delta_x, delta_y);
	}

	mouse_router::view_and_target mouse_router::switch_mouse_over(agge::point<int> &point)
	{
		const auto mouse_over = from(point);
		const auto notify = _capture_target.expired();

		if (mouse_over.over != _mouse_over)
		{
			if (notify && _mouse_over)
				_mouse_over->mouse_leave();
			_mouse_over = mouse_over.over;
			if (notify && _mouse_over)
				_mouse_over->mouse_enter();
		}
		return mouse_over;
	}
}
