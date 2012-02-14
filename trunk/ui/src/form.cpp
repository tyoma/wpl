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
#include "../win32/window.h"
#include "../win32/native_widget.h"

#include <windows.h>
#include <tchar.h>

namespace std
{
   namespace placeholders
   {
      using namespace std::tr1::placeholders;
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
			class form_impl : public form
			{
				shared_ptr<window> _window;
				shared_ptr<destructible> _advisory;
				children_list _children;

				virtual shared_ptr<widget_site> add(shared_ptr<widget> widget);
				virtual void get_children(children_list &children) const;
				virtual void set_visible(bool value);

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

			shared_ptr<container::widget_site> form_impl::add(shared_ptr<widget> w)
			{
				struct set_parent_visitor : public widget_visitor
				{
					HWND _parent;

					virtual void generic_widget_visited(widget &/*w*/)
					{	}

					virtual void native_widget_visited(native_widget &w)
					{	w.set_parent(_parent);	}

				public:
					set_parent_visitor(HWND parent)
						: _parent(parent)
					{	}
				};
				
				set_parent_visitor v(_window->hwnd());

				if (!w)
					throw invalid_argument("Non-null widget must be passed in!");
				_children.reserve(_children.size() + 1);
				w->visit(v);
				_children.push_back(w);

				return shared_ptr<container::widget_site>();
			}

			void form_impl::get_children(children_list &children) const
			{	children.assign(_children.begin(), _children.end());	}

			void form_impl::set_visible(bool value)
			{
				::ShowWindow(_window->hwnd(), value ? SW_SHOW : SW_HIDE);
			}

			LRESULT form_impl::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
			{
				if (WM_SIZE == message)
					resized(LOWORD(lparam), HIWORD(lparam));
				return previous(message, wparam, lparam);
			}
		}

		shared_ptr<form> create_form()
		{
			shared_ptr<form> f(new form_impl);
			
			return f;
		}
	}
}