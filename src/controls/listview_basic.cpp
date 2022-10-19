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

#include <wpl/controls/listview_basic.h>

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/figures.h>
#include <agge/filling_rules.h>
#include <agge/stroke_features.h>
#include <agge.text/limit.h>
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

		listview_basic::listview_basic()
			: _text_buffer(agge::font_style_annotation())
		{	}

		void listview_basic::apply_styles(const stylesheet &ss)
		{
			shared_ptr<font> font_ = ss.get_font("text.listview");
			font_style_annotation a = {	font_->get_key(), ss.get_color("text.listview"),	};

			_text_buffer.set_base_annotation(a);

			_bg = ss.get_color("background.listview");
			_bg_even = ss.get_color("background.listview.even");
			_bg_odd = ss.get_color("background.listview.odd");
			_bg_selected = ss.get_color("background.selected.listview");
			_fg_focus = ss.get_color("text.listview");
			_fg_selected = ss.get_color("text.selected.listview");
			_fg_focus_selected = ss.get_color("text.selected.listview");

			auto m = font_->get_metrics();

			_padding = ss.get_value("padding");
			_baseline_offset = _padding + m.ascent;
			_item_height = _baseline_offset + m.descent + _padding;

			_stroke.set_cap(agge::caps::butt());
			_stroke.set_join(agge::joins::bevel());
			_stroke.width(1.0f);
			_dash.add_dash(1.0f, 1.0f);
			_dash.dash_start(0.5f);
			layout_changed(false);
		}

		void listview_basic::set_columns_model(shared_ptr<columns_model> model)
		{
			_columns_model = model;
			listview_core::set_columns_model(model);
		}

		void listview_basic::set_model(shared_ptr<richtext_table_model> model)
		{
			_model = model;
			listview_core::set_model(model);
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

		void listview_basic::draw_item(gcontext &ctx, gcontext::rasterizer_ptr &ras, const rect_r &b_,
			unsigned layer, index_type row, unsigned state) const
		{
			if (0u == layer)
			{
				const auto bg = state & selected ? _bg_selected : (row & 1) ? _bg_odd : _bg_even;

				if (bg.a)
				{
					add_path(*ras, rectangle(b_.x1, b_.y1, b_.x2, b_.y2));
					ctx(ras, blender(bg), winding<>());
				}
			}
			else if (1u == layer && (state & focused))
			{
				rect_r b(b_);

				inflate(b, -0.5f * _padding, -0.5f * _padding);
				add_path(*ras, assist(assist(rectangle(b.x1, b.y1, b.x2, b.y2), _dash), _stroke));
				ctx(ras, blender(state & selected ? _fg_focus_selected : _fg_focus), winding<>());
			}
		}

		void listview_basic::draw_subitem(gcontext &ctx, gcontext::rasterizer_ptr &ras, const rect_r &b_,
			unsigned layer, index_type row, unsigned state, headers_model::index_type column) const
		{
			if (1u == layer)
			{
				rect_r b(b_);

				inflate(b, -_padding, -_padding);
				_text_buffer.clear();
				_model->get_text(row, column, _text_buffer);

				auto c = (state & focused) && (state & selected) ? _fg_focus_selected :
					(state & focused) ? _fg_focus :
					(state & selected) ? _fg_selected : _text_buffer.current_annotation().foreground;
				auto a = _columns_model->get_alignment(column);

				ctx.text_engine.render(*ras, _text_buffer, a.halign, a.valign, b, agge::limit::ellipsis(width(b)));
				ras->sort(true);
				ctx(ras, blender(c), winding<>());
			}
		}
	}
}
