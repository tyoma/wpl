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
#include "../helpers.h"
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
			{
				using namespace std;

				_header = factory_.create_control<header_basic>("header");
				_hscroller = factory_.create_control<scroller>("hscroller");
				_hscroller->set_model(shared_ptr<animated_scroll_model>(new animated_scroll_model(this->get_hscroll_model(),
					context.clock_, context.queue_, smooth_animation())));
				_vscroller = factory_.create_control<scroller>("vscroller");
				_vscroller->set_model(shared_ptr<animated_scroll_model>(new animated_scroll_model(this->get_vscroll_model(),
					context.clock_, context.queue_, smooth_animation())));

				_scroll_connection = this->get_hscroll_model()->invalidate += [this] {
					_header->set_offset(this->get_hscroll_model()->get_window().first);
				};
				this->get_hscroll_model()->invalidate();
			}

			void apply_styles(const stylesheet &ss)
			{
				const auto header_font_metrics = ss.get_font("text.header")->get_metrics();
				
				_header_height = static_cast<int>(2.0f * ss.get_value("padding.header")
					+ header_font_metrics.ascent + header_font_metrics.descent
					+ ss.get_value("separator.header"));
				BaseControlT::apply_styles(ss);
				this->layout_changed(false);
			}

			virtual void set_columns_model(std::shared_ptr<columns_model> m) override
			{
				_header->set_model(m);
				BaseControlT::set_columns_model(m);
			}

		private:
			virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override
			{
				const int scroller_width = 15;
				const int height2 = box.h - _header_height;

				_scrollers_views.clear();
				BaseControlT::layout(offset(append_view, 0, _header_height), make_box(box.w, height2));
				_hscroller->layout(offset(collect(append_view, _scrollers_views), 0, box.h - scroller_width),
					make_box(box.w, scroller_width));
				_vscroller->layout(offset(collect(append_view, _scrollers_views), box.w - scroller_width, _header_height),
					make_box(scroller_width, height2));
				_header->layout(append_view, make_box(box.w, _header_height));
			}

			virtual void mouse_scroll(int depressed, int x, int y, int delta_x, int delta_y) override
			{
				for (auto i = _scrollers_views.begin(); i != _scrollers_views.end(); ++i)
					i->regular->mouse_scroll(depressed, x, y, delta_x, delta_y);
			}

			static placed_view_appender offset(const placed_view_appender &inner, int dx, int dy)
			{
				// TODO: use custom appender functor, as std::function may allocate storage dynamically.

				return [&inner, dx, dy] (placed_view pv) {
					wpl::offset(pv.location, dx, dy);
					inner(pv);
				};
			}

			static placed_view_appender collect(const placed_view_appender &inner, std::vector<placed_view> &views)
			{
				return [&inner, &views] (placed_view pv) {
					views.push_back(pv);
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
			std::vector<placed_view> _scrollers_views;
		};
	}
}
