//	Copyright (c) 2011-2014 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "factory.h"

#include "../form.h"
#include "../layout.h"
#include "../win32/window.h"
#include "../win32/native_view.h"

#include <windows.h>
#include <tchar.h>

namespace std
{
   namespace placeholders
   {
      using namespace tr1::placeholders;
   }
};

using namespace std;
using namespace std::placeholders;

namespace wpl
{
	namespace ui
	{
		namespace
		{
			class form_impl : public form, public container
			{
				shared_ptr<window> _window;
				shared_ptr<destructible> _advisory;
				vector<widget_native> _children;

				virtual shared_ptr<container> get_root_container();
				virtual void set_visible(bool value);

				virtual std::shared_ptr<widget> create_widget(const std::wstring &type, const std::wstring &id);

				LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);

			public:
				form_impl();
				~form_impl();
			};



			form_impl::form_impl()
			{
				HWND hwnd = ::CreateWindow(_T("#32770"), 0, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0, 0, 100, 20, 0, 0, 0, 0);

				_window = window::attach(hwnd);
				_advisory = _window->advise(bind(&form_impl::wndproc, this, _1, _2, _3, _4));
			}

			form_impl::~form_impl()
			{	::DestroyWindow(_window->hwnd());	}

			shared_ptr<container> form_impl::get_root_container()
			{	return static_pointer_cast<container>(shared_from_this());	}

			void form_impl::set_visible(bool value)
			{
				::ShowWindow(_window->hwnd(), value ? SW_SHOW : SW_HIDE);
			}

			shared_ptr<widget> form_impl::create_widget(const wstring &type, const wstring &id)
			{
				widget_native wn = wpl::ui::create(_window->hwnd(), type, id);

				_children.push_back(wn);
				return wn.first;
			}

			LRESULT form_impl::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
			{
				if (WM_SIZE == message)
					layout->layout(LOWORD(lparam), HIWORD(lparam), 0, 0);
				return previous(message, wparam, lparam);
			}


		}

		shared_ptr<form> form::create()
		{
			return shared_ptr<form>(new form_impl());
		}
	}
}
