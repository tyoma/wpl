//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
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
#include <wpl/helpers.h>
#include <wpl/stylesheet.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace controls
	{
		namespace
		{
			typedef blender_solid_color<simd::blender_solid_color, platform_pixel_order> blender;
		}

		void listview_basic::apply_styles(const stylesheet &ss)
		{
			_font = ss.get_font("text.listview");

			_bg = ss.get_color("background.listview");
			_bg_even = ss.get_color("background.listview.even");
			_bg_odd = ss.get_color("background.listview.odd");
			_bg_selected = ss.get_color("background.selected.listview");
			_fg_normal = ss.get_color("text.listview");
			_fg_focus = ss.get_color("text.listview");
			_fg_selected = ss.get_color("text.selected.listview");
			_fg_focus_selected = ss.get_color("text.selected.listview");

			auto m = _font->get_metrics();

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

		void listview_basic::draw(gcontext &ctx, gcontext::rasterizer_ptr &ras) const
		{
			if (_bg.a)
			{
				const auto ua = ctx.update_area();

				add_path(*ras, rectangle(static_cast<real_t>(ua.x1), static_cast<real_t>(ua.y1),
					static_cast<real_t>(ua.x2), static_cast<real_t>(ua.y2)));
				ctx(ras, blender(_bg), winding<>());
			}
			listview_core::draw(ctx, ras);
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

				inflate(b, -0.5f * _padding, -0.5f * _padding);
				add_path(*ras, assist(assist(rectangle(b.x1, b.y1, b.x2, b.y2), _dash), _stroke));
				ctx(ras, blender(state & selected ? _fg_focus_selected : _fg_focus), winding<>());
			}
		}

		void listview_basic::draw_subitem(gcontext &ctx, gcontext::rasterizer_ptr &ras, const rect_r &b_, index_type /*item*/,
			unsigned state, columns_model::index_type /*subitem*/, const wstring &text) const
		{
			rect_r b(b_);

			inflate(b, -_padding, -_padding);
			render_string(*ras, text, ctx.text_engine, *_font, b, align_near, align_center);
			ras->sort(true);
			ctx(ras, blender(state & selected ? _fg_selected : _fg_normal), winding<>());
		}
	}
}
