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
			class form_impl : public form
			{
				shared_ptr<window> _window;
				shared_ptr<destructible> _advisory;

				virtual shared_ptr<view> add(shared_ptr<widget> widget);
				virtual void set_visible(bool value);

				LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);

			public:
				form_impl();
				~form_impl();
			};


			struct hierarchy_reparenter : node::visitor, noncopyable
			{
				hierarchy_reparenter(HWND parent_, shared_ptr<const transform> root_transform)
					: parent(parent_), transforms(1, root_transform)
				{	}

				virtual void visited(widget &w);
				virtual void visited(container &c);

				const HWND parent;
				native_view::transform_chain transforms;
			};


			struct view_reparenter : public view::visitor, noncopyable
			{
				view_reparenter(HWND parent_, const native_view::transform_chain &transforms_)
					: parent(parent_), transforms(transforms_)
				{	}

				virtual void generic_view_visited(view &/*view*/)
				{	}

				virtual void native_view_visited(native_view &view)
				{	view.set_parent(transforms, parent);	}

				const HWND parent;
				const native_view::transform_chain &transforms;
			};



			form_impl::form_impl()
			{
				HWND hwnd = ::CreateWindow(_T("#32770"), 0, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0, 0, 100, 20, 0, 0, 0, 0);

				_window = window::attach(hwnd);
				_advisory = _window->advise(bind(&form_impl::wndproc, this, _1, _2, _3, _4));
			}

			form_impl::~form_impl()
			{	::DestroyWindow(_window->hwnd());	}

			shared_ptr<view> form_impl::add(shared_ptr<widget> w)
			{
				native_view::transform_chain tc;
				view_reparenter vreparenter(_window->hwnd(), tc);
				shared_ptr<view> v(container::add(w));
				hierarchy_reparenter reparenter(_window->hwnd(), v->transform());

				v->visit(vreparenter);
				w->visit(reparenter);
				return v;
			}

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



			void hierarchy_reparenter::visited(widget &/*w*/)
			{	}

			void hierarchy_reparenter::visited(container &c)
			{
				container::children_list children;
				
				c.get_children(children);
				for (container::children_list::const_iterator i = children.begin(); i != children.end(); ++i)
				{
					view_reparenter reparenter(parent, transforms);

					transforms.push_back((*i)->transform());
					(*i)->visit(reparenter);
					(*i)->widget->visit(*this);
					transforms.pop_back();
				}
			}
		}

		shared_ptr<form> create_form()
		{
			shared_ptr<form> f(new form_impl);
			
			return f;
		}
	}
}
