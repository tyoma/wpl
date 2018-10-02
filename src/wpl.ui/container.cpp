#include <wpl/ui/container.h>

#include <wpl/ui/layout.h>

using namespace std;
using namespace placeholders;

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
			for (vector<positioned_view>::const_iterator i = _children.begin(); i != _children.end(); ++i)
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

		void container::do_layout()
		{
			if (!_children.empty())
				_layout->layout(_cx, _cy, &_children[0], _children.size());
			for (vector<positioned_view>::const_iterator i = _children.begin(); i != _children.end(); ++i)
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
	}
}
