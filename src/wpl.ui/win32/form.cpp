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

#include <wpl/ui/form.h>

#include "view_host.h"

#include <tchar.h>

using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace ui
	{
		namespace win32
		{
			class form : public wpl::ui::form
			{
			public:
				form();
				~form();

			private:
				virtual void set_view(const shared_ptr<view> &v);
				virtual void set_visible(bool value);
				virtual void set_caption(const std::wstring &caption);

				LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);

			private:
				HWND _hwnd;
				shared_ptr<win32::view_host> _host;
			};



			form::form()
				: _hwnd(::CreateWindow(_T("#32770"), 0, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0, 0, 100, 20, 0, 0, 0, 0))
			{	_host.reset(new win32::view_host(_hwnd, bind(&form::wndproc, this, _1, _2, _3, _4)));	}

			form::~form()
			{
				if (_hwnd)
					::DestroyWindow(_hwnd);
			}

			void form::set_view(const shared_ptr<view> &v)
			{	_host->set_view(v);	}

			void form::set_visible(bool value)
			{
				if (_hwnd)
					::ShowWindow(_hwnd, value ? SW_SHOW : SW_HIDE);
			}

			void form::set_caption(const std::wstring &caption)
			{
				if (_hwnd)
					::SetWindowTextW(_hwnd, caption.c_str());
			}

			LRESULT form::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
			{
				switch (message)
				{
				case WM_CLOSE:
					close();
					return 0;

				case WM_DESTROY:
					_hwnd = NULL;
					break;
				}
				return previous(message, wparam, lparam);
			}
		}

		shared_ptr<form> form::create()
		{	return shared_ptr<form>(new win32::form());	}
	}
}
