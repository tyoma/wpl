//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <wpl/controls/listview_basic.h>

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/figures.h>
#include <agge/filling_rules.h>
#include <agge/stroke_features.h>
#include <agge.text/text_engine.h>
#include <wpl/stylesheet.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace controls
	{
		namespace
		{
			typedef blender_solid_color<simd::blender_solid_color, order_bgra> blender;

			void inflate(rect_r &r, real_t dx, real_t dy)
			{	r.x1 -= dx, r.x2 += dx, r.y1 -= dy, r.y2 += dy;	}
		}

		void listview_basic::apply_styles(const stylesheet &ss)
		{
			_font = ss.get_font("text.listview");

			_bg_even = ss.get_color("background.listview.even");
			_bg_odd = ss.get_color("background.listview.odd");
			_bg_selected = ss.get_color("background.selected.listview");
			_fg_normal = ss.get_color("text.listview");
			_fg_focus = ss.get_color("text.listview");
			_fg_selected = ss.get_color("text.selected.listview");
			_fg_focus_selected = ss.get_color("text.selected.listview");

			agge::font::metrics m = _font->get_metrics();

			_padding = ss.get_value("padding");
			_baseline_offset = _padding + m.ascent;
			_item_height = _baseline_offset + m.descent + _padding;

			_stroke.set_cap(agge::caps::butt());
			_stroke.set_join(agge::joins::bevel());
			_stroke.width(1.0f);
			_dash.add_dash(1.0f, 1.0f);
			_dash.dash_start(0.5f);
			invalidate(nullptr);
		}

		real_t listview_basic::get_minimal_item_height() const
		{	return _item_height;	}

		void listview_basic::draw_item_background(gcontext &ctx, gcontext::rasterizer_ptr &ras, const rect_r &b,
			index_type item, unsigned state) const
		{
			const auto bg = state & selected ? _bg_selected : (item & 1) ? _bg_odd : _bg_even;

			if (bg.a)
			{
				add_path(*ras, rectangle(b.x1, b.y1, b.x2, b.y2));
				ctx(ras, blender(bg), winding<>());
			}
		}

		void listview_basic::draw_item(gcontext &ctx, gcontext::rasterizer_ptr &ras, const rect_r &b_, index_type /*item*/,
			unsigned state) const
		{
			if (state & focused)
			{
				rect_r b(b_);
				const real_t d = -0.5f * _padding;

				inflate(b, d, d);
				add_path(*ras, assist(assist(rectangle(b.x1, b.y1, b.x2, b.y2), _dash), _stroke));
				ctx(ras, blender(state & selected ? _fg_focus_selected : _fg_focus), winding<>());
			}
		}

		void listview_basic::draw_subitem(gcontext &ctx, gcontext::rasterizer_ptr &ras, const rect_r &b, index_type /*item*/,
			unsigned state, columns_model::index_type /*subitem*/, const wstring &text) const
		{
			const auto max_width = b.x2 - b.x1 - 2.0f * _padding;

			ctx.text_engine.render_string(*ras, *_font, text.c_str(), layout::near, b.x1 + _padding, b.y1 + _baseline_offset, max_width);
			ras->sort(true);
			ctx(ras, blender(state & selected ? _fg_selected : _fg_normal), winding<>());
		}
	}
}