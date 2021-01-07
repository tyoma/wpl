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

#include <wpl/controls/header_basic.h>

#include "../glyphs.h"

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/filling_rules.h>
#include <agge/figures.h>
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

		header_basic::header_basic(shared_ptr<gcontext::text_engine_type> text_services,
				shared_ptr<cursor_manager> cursor_manager_)
			: header_core(cursor_manager_), _text_services(text_services)
		{	}

		header_basic::~header_basic()
		{	}

		void header_basic::apply_styles(const stylesheet &ss)
		{
			const font_style_annotation base_style = {	ss.get_font("text.header")->get_key(),	};

			_bg = ss.get_color("background.header");
			_bg_sorted = ss.get_color("background.header.sorted");
			_fg_normal = ss.get_color("text.header");
			_fg_sorted = ss.get_color("text.header.sorted");
			_fg_separator = ss.get_color("separator.header");
			_fg_indicator = ss.get_color("text.header.indicator");

			_padding = ss.get_value("padding.header");
			_separator_width = ss.get_value("separator.header");

			_up.reset(new glyph(glyphs::up(base_style.basic.height)));
			_down.reset(new glyph(glyphs::down(base_style.basic.height)));

			_caption_buffer.set_base_annotation(base_style);
			adjust_column_widths();
			invalidate(nullptr);
		}

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
			const auto &size = get_last_size();

			add_path(*ras, rectangle(0.0f, size.h - _separator_width, size.w, size.h));
			ctx(ras, blender(_fg_separator), winding<>());
		}

		box<int> header_basic::measure_item(const columns_model &model, index_type item) const
		{
			_caption_buffer.clear();
			model.get_caption(item, _caption_buffer);

			box_r bounds = _text_services->measure(_caption_buffer);

			bounds.w += _separator_width + 3.0f * _padding;
			bounds.w += agge_max(_up ? wpl::width(_up->bounds()) : 0.0f, _down ? wpl::width(_down->bounds()) : 0.0f);
			bounds.h = agge_max(bounds.h, _up ? wpl::width(_up->bounds()) : 0.0f);
			bounds.h = agge_max(bounds.h, _down ? wpl::width(_down->bounds()) : 0.0f);
			bounds.h += _separator_width;
			bounds.h += 2.0f * _padding;
			return create_box(static_cast<int>(bounds.w + 0.999f), static_cast<int>(bounds.h + 0.999f));
		}

		void header_basic::draw_item(gcontext &ctx, gcontext::rasterizer_ptr &ras, const rect_r &b,
			const columns_model &model, index_type item, unsigned /*item_state_flags*/ state) const
		{
			auto halign_ = /*item ? align_far :*/ align_near;
			auto box = b;

			if ((state & sorted) && _bg_sorted.a)
			{
				add_path(*ras, rectangle(b.x1, b.y1, b.x2, b.y2));
				ctx(ras, blender(_bg_sorted), winding<>());
			}

			box.x2 -= _separator_width;
			box.y2 -= _separator_width;
			inflate(box, -_padding, -_padding);

			// 1. Draw sort mark, if any. Adjust text box.
			if (const auto g = sorted & state ? ascending & state ? _up.get() : _down.get() : nullptr)
			{
				const auto gbox = g->bounds();

				if (halign_ == align_far)
					add_path(*ras, offset(*g, box.x1 - gbox.x1, box.y1 - gbox.y1)), box.x1 += wpl::width(gbox) + _padding;
				else
					add_path(*ras, offset(*g, box.x2 - gbox.x2, box.y1 - gbox.y1)), box.x2 -= wpl::width(gbox) + _padding;
				ctx(ras, blender(_fg_indicator), winding<>());
			}

			// 2. Draw text.
			_caption_buffer.clear();
			model.get_caption(item, _caption_buffer);
			ctx.text_engine.render(*ras, _caption_buffer, halign_, align_near, box);
			ctx(ras, blender(state & sorted ? _fg_sorted : _fg_normal), winding<>());

			// 3. Draw right separator.
			add_path(*ras, rectangle(b.x2 - _separator_width, box.y1, b.x2, box.y2));
			ctx(ras, blender(_fg_separator), winding<>());
		}
	}
}
