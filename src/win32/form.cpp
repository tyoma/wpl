//	Copyright (c) 2011-2022 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <wpl/win32/form.h>

#include <tchar.h>
#include <wpl/helpers.h>
#include <wpl/win32/view_host.h>

using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace win32
	{
		namespace
		{
			const DWORD c_form_style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_SYSMENU;

			void set_icon(HWND hwnd, const gcontext::surface_type &icon, int type)
			{
				ICONINFO ii = { TRUE, 0, 0, icon.native(), icon.native() };
				HICON hicon = ::CreateIconIndirect(&ii);

				::SendMessage(hwnd, WM_SETICON, type, reinterpret_cast<LPARAM>(hicon));
			}

			rect_i get_window_rect(HWND hwnd)
			{
				RECT rc;

				::GetWindowRect(hwnd, &rc);
				return create_rect<int>(rc.left, rc.top, rc.right, rc.bottom);
			}

			point_i center_point(const rect_i &r)
			{	return create_point((r.x1 + r.x2) / 2, (r.y1 + r.y2) / 2);	}
		}



		form::form(const form_context &context, HWND howner)
			: _hwnd(::CreateWindow(_T("#32770"), 0, c_form_style, 0, 0, 100, 20, howner, 0, 0, 0))
		{
			_host.reset(new win32::view_host(_hwnd, context, bind(&form::wndproc, this, _1, _2, _3, _4)));
		}

		form::~form()
		{	}

		void form::set_root(shared_ptr<control> root)
		{	_host->set_root(root);	}

		rect_i form::get_location() const
		{	return get_window_rect(_hwnd);	}

		void form::set_location(const rect_i &location)
		{	::MoveWindow(_hwnd, location.x1, location.y1, width(location), height(location), TRUE);	}

		void form::center_parent()
		{
			const auto hparent = reinterpret_cast<HWND>(::GetWindowLongPtr(_hwnd, GWLP_HWNDPARENT));
			const auto pc = center_point(get_window_rect(hparent));
			auto r = get_location();
			const auto cc = center_point(r);

			offset(r, pc.x - cc.x, pc.y - cc.y);
			set_location(r);
		}

		void form::set_visible(bool value)
		{
			if (_hwnd)
				::ShowWindow(_hwnd, value ? SW_SHOW : SW_HIDE);
		}

		void form::set_caption(const string &caption)
		{
			if (_hwnd)
				::SetWindowTextW(_hwnd, _converter(caption.c_str()));
		}

		void form::set_caption_icon(const gcontext::surface_type &icon)
		{	set_icon(_hwnd, icon, ICON_SMALL);	}

		void form::set_task_icon(const gcontext::surface_type &icon)
		{	set_icon(_hwnd, icon, ICON_BIG);	}

		shared_ptr<wpl::form> form::create_child()
		{	return shared_ptr<form>(new form(_host->context, _hwnd));	}

		void form::set_features(unsigned /*features*/ features_)
		{
			auto style = helpers::update_flag(::GetWindowLong(_hwnd, GWL_STYLE), !!(features_ & resizeable), WS_SIZEBOX);

			style = helpers::update_flag(style, !!(features_ & minimizable), WS_MINIMIZEBOX);
			::SetWindowLong(_hwnd, GWL_STYLE, helpers::update_flag(style, !!(features_ & maximizable), WS_MAXIMIZEBOX));
		}

		LRESULT form::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
		{
			switch (message)
			{
			case WM_SETFOCUS:
				// just do nothing, as the default procedure will pass it to the first tabstop
				return 0;

			case WM_CLOSE:
				close();
				return 0;

			case WM_NCDESTROY:
				_hwnd.release();
				break;
			}
			return previous(message, wparam, lparam);
		}
	}
}
