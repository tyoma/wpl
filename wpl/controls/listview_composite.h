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

#pragma once

#include "header_basic.h"

#include "../animated_models.h"
#include "../controls.h"
#include "../factory.h"
#include "../layout.h"
#include "../stylesheet.h"

namespace wpl
{
	namespace controls
	{
		template <typename BaseControlT>
		class listview_composite : public BaseControlT
		{
		public:
			listview_composite(const factory &factory_, const control_context &context)
				: BaseControlT(context.stylesheet_)
			{
				using namespace std;

				const auto header_font_metrics = context.stylesheet_->get_font("text.header")->get_metrics();
				
				_header_height = static_cast<int>(2.0f * context.stylesheet_->get_value("padding.header")
					+ header_font_metrics.ascent + header_font_metrics.descent
					+ context.stylesheet_->get_value("border.header"));

				_header = factory_.create_control<header_basic>("header");
				_hscroller = factory_.create_control<scroller>("hscroller");
				_hscroller->set_model(shared_ptr<animated_scroll_model>(new animated_scroll_model(get_hscroll_model(),
					context.clock_, context.queue_, smooth_animation())));
				_vscroller = factory_.create_control<scroller>("vscroller");
				_vscroller->set_model(shared_ptr<animated_scroll_model>(new animated_scroll_model(get_vscroll_model(),
					context.clock_, context.queue_, smooth_animation())));

				_scroll_connection = get_hscroll_model()->invalidated += [this] {
					_header->set_offset(get_hscroll_model()->get_window().first);
				};
				get_hscroll_model()->invalidated();
			}

			virtual void set_columns_model(std::shared_ptr<columns_model> m)
			{
				_header->set_model(m);
				BaseControlT::set_columns_model(m);
			}

		private:
			virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box)
			{
				const int scroller_width = 15;
				const int height2 = box.h - _header_height;

				BaseControlT::layout(offset(append_view, 0, _header_height), make_box(box.w, height2));
				_hscroller->layout(offset(append_view, 0, box.h - scroller_width), make_box(box.w, scroller_width));
				_vscroller->layout(offset(append_view, box.w - scroller_width, _header_height), make_box(scroller_width, height2));
				_header->layout(append_view, make_box(box.w, _header_height));
			}

			static placed_view_appender offset(const placed_view_appender &inner, int dx, int dy)
			{
				// TODO: use custom appender functor, as std::function may allocate storage dynamically.

				return [&inner, dx, dy] (placed_view pv) {
					pv.location.x1 += dx, pv.location.x2 += dx;
					pv.location.y1 += dy, pv.location.y2 += dy;
					inner(pv);
				};
			}

			static agge::box<int> make_box(int width, int height)
			{
				agge::box<int> b = { width, height };
				return b;
			}

		private:
			std::shared_ptr<header_basic> _header;
			std::shared_ptr<wpl::scroller> _hscroller, _vscroller;
			int _header_height;
			wpl::slot_connection _scroll_connection;
		};
	}
}
