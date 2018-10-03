#include <wpl/ui/container.h>

#include <wpl/ui/layout.h>

using namespace std;
using namespace placeholders;

#define FOR_EACH(iterator_type, iterator_name, container)\
	for (iterator_type iterator_name = container.begin(); iterator_name != container.end(); ++iterator_name)\

namespace wpl
{
	namespace ui
	{
		void container::add_view(const shared_ptr<view> &child)
		{
			positioned_view c = {
				0, 0, 0, 0,
				child,
				child->invalidate += bind(&container::on_invalidate, this, static_cast<unsigned>(_children.size()), _1)
			};

			_children.push_back(c);
			do_layout();
		}

		void container::set_layout(const shared_ptr<layout_manager> &layout)
		{
			_layout = layout;
		}

		void container::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const
		{
			FOR_EACH(views_t::const_iterator, i, _children)
			{
				gcontext child_ctx = ctx.transform(i->left, i->top);
				i->child->draw(child_ctx, rasterizer);
			}
		}

		void container::resize(unsigned cx, unsigned cy)
		{
			_cx = cx, _cy = cy;
			do_layout();
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
				v->mouse_down(button, depressed, x, y);
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

		void container::do_layout()
		{
			if (!_children.empty())
				_layout->layout(_cx, _cy, &_children[0], _children.size());
			FOR_EACH(views_t::const_iterator, i, _children)
				i->child->resize(i->width, i->height);
		}

		void container::on_invalidate(unsigned index, const agge::rect_i *area)
		{
			const positioned_view &v = _children[index];
			agge::rect_i area2 = { 0, 0, v.width, v.height };

			if (area)
				area2 = *area;
			area2.x1 += v.left, area2.y1 += v.top, area2.x2 += v.left, area2.y2 += v.top;
			invalidate(&area2);
		}

		shared_ptr<view> container::child_from_point(int &x, int &y) const
		{
			FOR_EACH(views_t::const_iterator, i, _children)
			{
				int x2 = x - i->left, y2 = y - i->top;

				if ((x2 < 0) | (y2 < 0) | (x2 >= i->width) | (y2 >= i->height))
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
}
