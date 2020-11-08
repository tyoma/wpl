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
#include "../container.h"
#include "../controls.h"
#include "../factory.h"
#include "../layout.h"
#include "../stylesheet.h"

namespace wpl
{
	namespace controls
	{
		class listview_complex_layout : public layout_manager
		{
		public:
			listview_complex_layout(agge::real_t header_height)
				: _header_height(static_cast<int>(0.5f + header_height))
			{	}

		private:
			virtual void layout(unsigned width, unsigned height, container::positioned_view *views, size_t count) const
			{
				const int scroller_width = 15;
				const int height2 = height - _header_height;

				// listview core
				if (count >= 1)
					views[0].location.left = 0, views[0].location.top = _header_height, views[0].location.width = width, views[0].location.height = height2;

				// horizontal scroller
				if (count >= 3)
					views[1].location.left = 0, views[1].location.top = height - scroller_width, views[1].location.width = width, views[1].location.height = scroller_width;

				// vertical scroller
				if (count >= 2)
					views[2].location.left = width - scroller_width, views[2].location.top = _header_height, views[2].location.width = scroller_width, views[2].location.height = height2;

				// header
				if (count >= 4)
					views[3].location.left = 0, views[3].location.top = 0, views[3].location.width = width, views[3].location.height = _header_height;
			}

		private:
			int _header_height;
		};

		template <typename BaseControlT>
		class external_view_contol : public BaseControlT
		{
		public:
			external_view_contol(const std::shared_ptr<stylesheet> &stylesheet_, const std::shared_ptr<header_basic> &header_)
				: BaseControlT(stylesheet_), _header(header_)
			{
				_scroll_connection = this->get_hscroll_model()->invalidated += [this] {
					_header->set_offset(this->get_hscroll_model()->get_window().first);
				};
				_header->set_offset(this->get_hscroll_model()->get_window().first);
			}

		public:
			std::weak_ptr<view> external_view;

		private:
			virtual std::shared_ptr<view> get_view()
			{	return external_view.lock();	}

			virtual void set_columns_model(std::shared_ptr<columns_model> m)
			{
				_header->set_model(m);
				BaseControlT::set_columns_model(m);
			}

		private:
			std::shared_ptr<header_basic> _header;
			wpl::slot_connection _scroll_connection;
		};

		struct composite_container : container
		{
			virtual void key_down(unsigned code, int modifiers)
			{	input->key_down(code, modifiers);	}

			virtual void character(wchar_t symbol, unsigned repeats, int modifiers)
			{	input->character(symbol, repeats, modifiers);	}

			virtual void key_up(unsigned code, int modifiers)
			{	input->key_up(code, modifiers);	}

			virtual void got_focus()
			{	input->got_focus();	}

			virtual void lost_focus()
			{	input->lost_focus();	}

			virtual void mouse_scroll(int depressed, int x, int y, int delta_x, int delta_y)
			{
				scrollers[0]->mouse_scroll(depressed, x, y, delta_x, delta_y);
				scrollers[1]->mouse_scroll(depressed, x, y, delta_x, delta_y);
			}

			std::shared_ptr<keyboard_input> input;
			std::shared_ptr<mouse_input> scrollers[2];
		};


		template <typename ControlT>
		std::shared_ptr<ControlT> create_listview(const factory &factory_, const std::shared_ptr<stylesheet> &stylesheet_)
		{
			using namespace std;

			const auto header_font_metrics = stylesheet_->get_font("text.header")->get_metrics();
			const auto header_height = 2.0f * stylesheet_->get_value("padding.header")
				+ header_font_metrics.ascent + header_font_metrics.descent + stylesheet_->get_value("border.header");

			shared_ptr<listview_complex_layout> layout(new listview_complex_layout(header_height));
			shared_ptr<composite_container> composite(new composite_container);
			shared_ptr<header_basic> header_(static_pointer_cast<header_basic>(factory_.create_control("header")));
			shared_ptr< external_view_contol<ControlT> > lv(new external_view_contol<ControlT>(stylesheet_, header_));
			shared_ptr<wpl::scroller> hscroller(static_pointer_cast<wpl::scroller>(factory_.create_control("hscroller")));
			shared_ptr<wpl::scroller> vscroller(static_pointer_cast<wpl::scroller>(factory_.create_control("vscroller")));

			lv->external_view = composite;

			composite->input = lv->ControlT::get_view();
			composite->scrollers[0] = hscroller->get_view();
			composite->scrollers[1] = vscroller->get_view();
			composite->set_layout(layout);
			composite->add_view(lv);

			hscroller->set_model(make_shared<animated_scroll_model>(lv->get_hscroll_model(), nullptr));
			composite->add_view(hscroller->get_view());

			vscroller->set_model(make_shared<animated_scroll_model>(lv->get_vscroll_model(), nullptr));
			composite->add_view(vscroller->get_view());

			composite->add_view(header_->get_view());

			return shared_ptr<ControlT>(composite, lv.get());
		}
	}
}
