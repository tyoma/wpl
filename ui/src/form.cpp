//	Copyright (c) 2011-2018 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "native_container.h"

#include "../form.h"
#include "../layout.h"
#include "../win32/window.h"
#include "../win32/native_view.h"

#include <algorithm>
#include <iterator>
#include <tchar.h>
#include <olectl.h>
#include <windows.h>

using namespace std;
using namespace std::placeholders;

namespace wpl
{
	namespace ui
	{
		namespace
		{
			class form_impl : public form
			{
				shared_ptr<window> _window;
				shared_ptr<void> _advisory;
				shared_ptr<native_container> _container;

				virtual shared_ptr<container> get_root_container();
				virtual void set_visible(bool value);
				virtual void set_caption(const std::wstring &caption);

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
				_container.reset(new native_container(_window->hwnd()));
			}

			form_impl::~form_impl()
			{	::DestroyWindow(_window->hwnd());	}

			shared_ptr<container> form_impl::get_root_container()
			{	return _container;	}

			void form_impl::set_visible(bool value)
			{	::ShowWindow(_window->hwnd(), value ? SW_SHOW : SW_HIDE);	}

			void form_impl::set_caption(const std::wstring &caption)
			{	::SetWindowTextW(_window->hwnd(), caption.c_str());	}

			LRESULT form_impl::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
			{
				switch (message)
				{
					case WM_SIZE:
					{
						void *hdwp = ::BeginDeferWindowPos(100);
						layout_manager::position new_position = { 0, 0, LOWORD(lparam), HIWORD(lparam) };

						_container->reposition(hdwp, new_position);
						::EndDeferWindowPos(hdwp);
						break;
					}

					case WM_CLOSE:
						close();
						return 0;

					case WM_COMMAND:
						::SendMessage(reinterpret_cast<HWND>(lparam), OCM_COMMAND, wparam, lparam);
						return 0;
				};
				return previous(message, wparam, lparam);
			}
		}

		shared_ptr<form> form::create()
		{
			return shared_ptr<form>(new form_impl());
		}
	}
}
