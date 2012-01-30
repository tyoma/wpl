//	Copyright (C) 2011-2012 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../win32/containers.h"

#include <windows.h>
#include <tchar.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace
		{
			class form_impl : public form
			{
				HWND _hwnd;

				virtual shared_ptr<widget_site> add(shared_ptr<widget> widget);
				virtual void set_visible(bool value);

			public:
				form_impl();
				~form_impl();
			};


			form_impl::form_impl()
				: _hwnd(::CreateWindow(_T("#32770"), 0, WS_POPUP, 0, 0, 0, 0, 0, 0, 0, 0))
			{	}

			form_impl::~form_impl()
			{	::DestroyWindow(_hwnd);	}

			shared_ptr<container::widget_site> form_impl::add(shared_ptr<widget> widget)
			{
				throw 0;
			}

			void form_impl::set_visible(bool value)
			{
				::ShowWindow(_hwnd, value ? SW_SHOW : SW_HIDE);
			}
		}

		shared_ptr<form> create_form()
		{
			shared_ptr<form> f(new form_impl);
			
			return f;
		}
	}
}
