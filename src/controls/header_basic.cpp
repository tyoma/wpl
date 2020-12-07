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

#include <wpl/controls/header_basic.h>

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/filling_rules.h>
#include <agge/figures.h>
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
		}

		header_basic::header_basic(shared_ptr<cursor_manager> cursor_manager_)
			: header_core(cursor_manager_)
		{	}

		void header_basic::draw(gcontext &ctx, gcontext::rasterizer_ptr &ras) const
		{
			if (_bg.a)
			{
				const auto ua = ctx.update_area();
				add_path(*ras, rectangle(static_cast<real_t>(ua.x1), static_cast<real_t>(ua.y1),
					static_cast<real_t>(ua.x2), static_cast<real_t>(ua.y2)));
				ctx(ras, blender(_bg), winding<>());
			}
			header_core::draw(ctx, ras);
		}

		void header_basic::draw_item_background(gcontext &ctx, gcontext::rasterizer_ptr &ras, const agge::rect_r &b,
			index_type /*item*/, unsigned /*item_state_flags*/ state) const
		{
			if ((state & sorted) && _bg_sorted.a)
			{
				add_path(*ras, rectangle(b.x1, b.y1, b.x2, b.y2));
				ctx(ras, blender(_bg_sorted), winding<>());
			}
		}

		void header_basic::draw_item(gcontext &ctx, gcontext::rasterizer_ptr &ras, const agge::rect_r &b, index_type /*item*/,
			unsigned /*item_state_flags*/ state, const wstring &text) const
		{
			const auto w = b.x2 - b.x1;

			ctx.text_engine.render_string(*ras, *_font, text.c_str(), layout::near, b.x1 + _padding, b.y1 + _baseline_offset, w);
			ctx(ras, blender(state & sorted ? _fg_sorted : _fg_normal), winding<>());

			add_path(*ras, rectangle(b.x2 - _separator_width, b.y1, b.x2, b.y2));
			ctx(ras, blender(_fg_separator), winding<>());
			add_path(*ras, rectangle(b.x1, b.y2 - _separator_width, b.x2, b.y2));
			ctx(ras, blender(_fg_separator), winding<>());

			if (header_basic::sorted & state)
			{
				const auto order_string = (header_basic::ascending & state) ? L"\x02C4" : L"\x02C5";
				ctx.text_engine.render_string(*ras, *_font, order_string, layout::center, b.x1 + 0.5f * w, b.y2 - _font->get_metrics().descent);
				ctx(ras, blender(_fg_normal), winding<>());
			}
		}

		void header_basic::apply_styles(const stylesheet &ss)
		{
			_font = ss.get_font("text.header");

			_bg = ss.get_color("background.header");
			_bg_sorted = ss.get_color("background.header.sorted");
			_fg_normal = ss.get_color("text.header");
			_fg_sorted = ss.get_color("text.header.sorted");
			_fg_separator = ss.get_color("border.header.separator");

			agge::font::metrics m = _font->get_metrics();

			_padding = ss.get_value("padding.header");
			_baseline_offset = _padding + m.ascent;
			_separator_width = ss.get_value("border.header.separator");
			invalidate(nullptr);
		}
	}
}
