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

#include "listview_core.h"
#include "scroller.h"

#include "../container.h"
#include "../layout.h"

namespace wpl
{
	namespace controls
	{
		class listview_complex_layout : public layout_manager
		{
			virtual void layout(unsigned width, unsigned height, container::positioned_view *views, size_t count) const
			{
				const int scroller_width = 10;
				const int header_height = 0;
				const int height2 = height - header_height;

				// listview core
				if (count >= 1)
					views[0].location.left = 0, views[0].location.top = header_height, views[0].location.width = width, views[0].location.height = height2;

				// horizontal scroller
				if (count >= 3)
					views[1].location.left = 0, views[1].location.top = height - scroller_width, views[1].location.width = width, views[1].location.height = scroller_width;

				// vertical scroller
				if (count >= 2)
					views[2].location.left = width - scroller_width, views[2].location.top = header_height, views[2].location.width = scroller_width, views[2].location.height = height2;

				// header
				if (count >= 4)
					views[3].location.left = 0, views[3].location.top = 0, views[3].location.width = width, views[3].location.height = header_height;
			}
		};

		template <typename BaseControlT>
		class external_view_contol : public BaseControlT
		{
		public:
			std::weak_ptr<view> external_view;

		private:
			virtual std::shared_ptr<view> get_view()
			{	return external_view.lock();	}
		};

		struct redirecting_keyboard_container : container
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

			std::shared_ptr<keyboard_input> input;
		};



		template <typename ControlT>
		std::shared_ptr<ControlT> create_listview()
		{
			using namespace std;

			shared_ptr<listview_complex_layout> layout(new listview_complex_layout);
			shared_ptr<redirecting_keyboard_container> composite(new redirecting_keyboard_container);
			shared_ptr< external_view_contol<ControlT> > lv(new external_view_contol<ControlT>);
			shared_ptr<scroller> hscroller(new scroller(scroller::horizontal));
			shared_ptr<scroller> vscroller(new scroller(scroller::vertical));

			lv->external_view = composite;

			composite->input = lv;
			composite->set_layout(layout);
			composite->add_view(lv);

			hscroller->set_model(lv->get_hscroll_model());
			composite->add_view(hscroller);

			vscroller->set_model(lv->get_vscroll_model());
			composite->add_view(vscroller);
			return shared_ptr<ControlT>(lv.get(), [composite] (void *) { });
		}
	}
}
