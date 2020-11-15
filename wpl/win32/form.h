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

#include "view_host.h"

#include "../form.h"
#include <memory>

namespace wpl
{
	namespace win32
	{
		class form : public wpl::form
		{
		public:
			form(const form_context &context, HWND howner = NULL);
			~form();

		private:
			// view_host methods
			virtual void set_view(const std::shared_ptr<view> &v);
			virtual void set_background_color(agge::color color);

			// form methods
			virtual view_location get_location() const;
			virtual void set_location(const view_location &location);
			virtual void set_visible(bool value);
			virtual void set_caption(const std::wstring &caption);
			virtual void set_caption_icon(const gcontext::surface_type &icon);
			virtual void set_task_icon(const gcontext::surface_type &icon);
			virtual std::shared_ptr<wpl::form> create_child();
			virtual void set_style(unsigned /*styles*/ style);
			virtual void set_font(const font &font_);

			LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);

		private:
			HWND _hwnd;
			std::shared_ptr<win32::view_host> _host;
			std::shared_ptr<void> _font;
		};
	}
}
