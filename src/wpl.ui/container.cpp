#include <wpl/container.h>

#include <wpl/layout.h>

using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace
	{
		unsigned npos = static_cast<unsigned>(-1);
	}

	container::container()
		: _capture_target(npos)
	{	transcending = true;	}

	void container::add_view(const shared_ptr<view> &child, int tab_order)
	{
		positioned_view c = {
			{ 0, 0, 0, 0 },
			child,
			tab_order,
			child->invalidate += bind(&container::on_invalidate, this, static_cast<unsigned>(_children.size()), _1),
			child->force_layout += [this] { force_layout(); },
			child->capture += bind(&container::on_capture, this, static_cast<unsigned>(_children.size()), _1),
			child->request_focus += [this] (const shared_ptr<keyboard_input> &v) { request_focus(v); },
		};

		_children.push_back(c);
		force_layout();
	}

	void container::set_layout(const shared_ptr<layout_manager> &layout)
	{
		_layout = layout;
		force_layout();
	}

	void container::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const
	{
		for (auto i = _children.begin(); i != _children.end(); ++i)
		{
			gcontext child_ctx = ctx.translate(i->location.left, i->location.top);

			if (i->child->transcending)
			{
				i->child->draw(child_ctx, rasterizer);
			}
			else
			{
				gcontext child_ctx_windowed = child_ctx.window(0, 0, i->location.width, i->location.height);

				i->child->draw(child_ctx_windowed, rasterizer);
			}
		}
	}

	void container::resize(unsigned cx, unsigned cy, positioned_native_views &nviews)
	{
		if (!_children.empty())
			_layout->layout(cx, cy, &_children[0], _children.size());
		for (auto i = _children.begin(); i != _children.end(); ++i)
		{
			size_t j = nviews.size();

			i->child->resize(i->location.width, i->location.height, nviews);
			for (const size_t k = nviews.size(); j < k; ++j)
				nviews[j].location.left += i->location.left, nviews[j].location.top += i->location.top;
		}
	}

	void container::get_tabbed_controls(std::vector<tabbed_control> &tabbed_controls, bool do_clear)
	{
		if (do_clear)
			tabbed_controls.clear();
		for (auto i = _children.begin(); i != _children.end(); ++i)
		{
			if (i->tab_order)
				tabbed_controls.push_back(make_pair(i->tab_order, i->child));
			i->child->get_tabbed_controls(tabbed_controls, false);
		}
	}

	void container::mouse_leave()
	{
		if (!_mouse_over)
			return;
		_mouse_over->mouse_leave();
		_mouse_over.reset();
	}

	void container::mouse_move(int depressed, int x, int y)
	{
		if (shared_ptr<view> v = switch_mouse_over(x, y))
			v->mouse_move(depressed, x, y);
	}

	void container::mouse_down(mouse_buttons button, int depressed, int x, int y)
	{
		if (shared_ptr<view> v = switch_mouse_over(x, y))
		{
			request_focus(v);
			v->mouse_down(button, depressed, x, y);
		}
	}

	void container::mouse_up(mouse_buttons button, int depressed, int x, int y)
	{
		if (shared_ptr<view> v = switch_mouse_over(x, y))
			v->mouse_up(button, depressed, x, y);
	}

	void container::mouse_double_click(mouse_buttons button, int depressed, int x, int y)
	{
		if (shared_ptr<view> v = switch_mouse_over(x, y))
			v->mouse_double_click(button, depressed, x, y);
	}

	void container::on_invalidate(unsigned index, const agge::rect_i *area)
	{
		const positioned_view &v = _children[index];
		agge::rect_i area2 = { 0, 0, v.location.width, v.location.height };

		if (area)
			area2 = *area;
		area2.x1 += v.location.left, area2.y1 += v.location.top, area2.x2 += v.location.left, area2.y2 += v.location.top;
		invalidate(&area2);
	}

	void container::on_capture(unsigned index, shared_ptr<void> &capture_handle)
	{
		shared_ptr< shared_ptr<void> > upstream_handle(new shared_ptr<void>);

		capture(*upstream_handle);
		capture_handle.reset(new bool, [this, index, upstream_handle] (bool *p) {
			_capture_target = npos;
			delete p;
		});
		_capture_target = index;
	}

	shared_ptr<view> container::child_from_point(int &x, int &y) const
	{
		if (npos != _capture_target)
		{
			const positioned_view &v = _children[_capture_target];

			x -= v.location.left, y -= v.location.top;
			return v.child;
		}

		for (auto i = _children.rbegin(); i != _children.rend(); ++i)
		{
			int x2 = x - i->location.left, y2 = y - i->location.top;

			if ((x2 < 0) | (y2 < 0) | (x2 >= i->location.width) | (y2 >= i->location.height))
				continue;
			x = x2, y = y2;
			return i->child;
		}
		return shared_ptr<view>();
	}

	shared_ptr<view> container::switch_mouse_over(int &x, int &y)
	{
		shared_ptr<view> mouse_over = child_from_point(x, y);

		if (mouse_over != _mouse_over)
		{
			if (_mouse_over)
				_mouse_over->mouse_leave();
			_mouse_over = mouse_over;
			if (_mouse_over)
				_mouse_over->mouse_enter();
		}
		return mouse_over;
	}
}
