//	Copyright (c) 2011-2019 by Artem A. Gevorkyan (gevorkyan.org)
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
			namespace
			{
				void set_icon(HWND hwnd, const gcontext::surface_type &icon, int type)
				{
					ICONINFO ii = { TRUE, 0, 0, icon.native(), icon.native() };
					HICON hicon = ::CreateIconIndirect(&ii);

					::SendMessage(hwnd, WM_SETICON, type, reinterpret_cast<LPARAM>(hicon));
				}
			}

			class form : public wpl::ui::form
			{
			public:
				form(HWND howner);
				~form();

			private:
				// view_host methods
				virtual void set_view(const shared_ptr<view> &v);
				virtual void set_background_color(agge::color color);

				// form methods
				virtual view_location get_location() const;
				virtual void set_location(const view_location &location);
				virtual void set_visible(bool value);
				virtual void set_caption(const wstring &caption);
				virtual void set_caption_icon(const gcontext::surface_type &icon);
				virtual void set_task_icon(const gcontext::surface_type &icon);
				virtual shared_ptr<ui::form> create_child();

				LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);

			private:
				HWND _hwnd;
				shared_ptr<win32::view_host> _host;
			};



			form::form(HWND howner)
				: _hwnd(::CreateWindow(_T("#32770"), 0, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0, 0, 100, 20, howner, 0, 0,
					0))
			{	_host.reset(new win32::view_host(_hwnd, bind(&form::wndproc, this, _1, _2, _3, _4)));	}

			form::~form()
			{
				if (_hwnd)
					::DestroyWindow(_hwnd);
			}

			void form::set_view(const shared_ptr<view> &v)
			{	_host->set_view(v);	}

			void form::set_background_color(agge::color /*color*/)
			{	}

			view_location form::get_location() const
			{
				RECT rc;

				::GetWindowRect(_hwnd, &rc);
				view_location l = { rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top };
				return l;
			}

			void form::set_location(const view_location &location)
			{	::MoveWindow(_hwnd, location.left, location.top, location. width, location.height, TRUE);	}

			void form::set_visible(bool value)
			{
				if (_hwnd)
					::ShowWindow(_hwnd, value ? SW_SHOW : SW_HIDE);
			}

			void form::set_caption(const wstring &caption)
			{
				if (_hwnd)
					::SetWindowTextW(_hwnd, caption.c_str());
			}

			void form::set_caption_icon(const gcontext::surface_type &icon)
			{	set_icon(_hwnd, icon, ICON_SMALL);	}

			void form::set_task_icon(const gcontext::surface_type &icon)
			{	set_icon(_hwnd, icon, ICON_BIG);	}

			shared_ptr<ui::form> form::create_child()
			{	return shared_ptr<form>(new form(_hwnd));	}

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

		shared_ptr<form> create_form(HWND howner)
		{	return shared_ptr<form>(new win32::form(howner));	}
	}
}
