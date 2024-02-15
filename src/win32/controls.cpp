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

#include <wpl/win32/controls.h>

#include <wpl/win32/native_view.h>

#include <commctrl.h>
#include <olectl.h>
#include <tchar.h>

using namespace std;

namespace wpl
{
	namespace win32
	{
		button::button()
			: text_container("text.button")
		{	}

		void button::layout(const placed_view_appender &append_view, const agge::box<int> &box)
		{	native_view::layout(append_view, box);	}

		HWND button::materialize(HWND hparent)
		{
			return ::CreateWindowW(WC_BUTTON, _converter(_text.c_str()), WS_CHILD | WS_VISIBLE | BS_NOTIFY, 0, 0, 100, 100,
				hparent, NULL, NULL, NULL);
		}

		LRESULT button::on_message(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &handler)
		{
			switch (message)
			{
			default:
				return handler(message, wparam, lparam);

			case OCM_COMMAND:
				if (HIWORD(wparam) == BN_CLICKED)
					clicked();
				return 0;
			}
		}


		link::link()
			: text_container("text.link")
		{	_halign = agge::align_near;	}

		void link::layout(const placed_view_appender &append_view, const agge::box<int> &box)
		{	native_view::layout(append_view, box);	}

		HWND link::materialize(HWND hparent)
		{
			return ::CreateWindowW(L"SysLink", _converter(_text.c_str()), helpers::update_flag(WS_CHILD | WS_VISIBLE,
				agge::align_far == _halign, LWS_RIGHT), 0, 0, 100, 100, hparent, NULL, NULL, NULL);
		}

		LRESULT link::on_message(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &handler)
		{
			switch (message)
			{
			default:
				return handler(message, wparam, lparam);

			case OCM_NOTIFY:
				const NMLINK *nmlink = reinterpret_cast<const NMLINK *>(lparam);

				if (NM_CLICK == nmlink->hdr.code)
					clicked(nmlink->item.iLink, _converter(nmlink->item.szUrl));
				return 0;
			}
		}

		void link::set_halign(agge::text_alignment value)
		{
			text_container::set_halign(value);
			::SetWindowLong(get_window(), GWL_STYLE, helpers::update_flag(::GetWindowLong(get_window(), GWL_STYLE),
				agge::align_far == _halign, LWS_RIGHT));
		}


		editbox::editbox()
			: native_view("text.edit")
		{	}

		bool editbox::get_value(value_type &value) const
		{	return _converter.get(value, get_window()), true;	}

		void editbox::set_value(const value_type &value)
		{	_converter.set(get_window(), _text = value);	}

		void editbox::layout(const placed_view_appender &append_view, const agge::box<int> &box)
		{	native_view::layout(append_view, box);	}

		HWND editbox::materialize(HWND hparent)
		{
			return ::CreateWindowW(WC_EDITW, _converter.convert(_text), WS_BORDER | WS_CHILD | WS_VISIBLE, 0, 0, 100, 100, hparent,
				NULL, NULL, NULL);
		}

		LRESULT editbox::on_message(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &handler)
		{
			wchar_t c;

			switch (message)
			{
			case WM_CHAR:
				c = static_cast<wchar_t>(wparam);
				translate_char(c);
				wparam = c;
				break;

			case WM_KEYDOWN:
				if (VK_RETURN == wparam)
				{
					_converter.get(_text, get_window());
					accept(_text);
					_converter.set(get_window(), _text);
				}
				break;

			case OCM_COMMAND:
				if (HIWORD(wparam) == EN_CHANGE)
					changed();
				return 0;
			}
			return handler(message, wparam, lparam);
		}
	}
}
