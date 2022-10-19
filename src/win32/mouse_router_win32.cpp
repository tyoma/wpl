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

#include <wpl/win32/mouse_router.h>

#include <windowsx.h>
#include <wpl/cursor.h>
#include <wpl/win32/helpers.h>

using namespace std;

namespace wpl
{
	namespace win32
	{
		namespace
		{
			int convert_mouse_modifiers(WPARAM wparam)
			{
				auto modifiers = 0;

				modifiers |= MK_CONTROL & wparam ? keyboard_input::control : 0;
				modifiers |= MK_SHIFT & wparam ? keyboard_input::shift : 0;
				modifiers |= MK_LBUTTON & wparam ? mouse_input::left : 0;
				modifiers |= MK_MBUTTON & wparam ? mouse_input::middle : 0;
				modifiers |= MK_RBUTTON & wparam ? mouse_input::right : 0;
				return modifiers;
			}
		}


		mouse_router::mouse_router(const vector<placed_view> &views, mouse_router_host &host,
				shared_ptr<cursor_manager> cursor_manager_)
			: wpl::mouse_router(views, host), _cursor_manager(cursor_manager_), _mouse_in(false)
		{	}

		bool mouse_router::handle_message(LRESULT &result, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
		{
			void (mouse_input::*fn)(mouse_input::mouse_buttons button_, int depressed, int x, int y) = nullptr;
			agge::point<int> pt = {	GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)	};

			switch (message)
			{
			default:
				return false;

			case WM_SETCURSOR:
				if (HTCLIENT == LOWORD(lparam) && hwnd == reinterpret_cast<HWND>(wparam))
					return result = TRUE, true;
				else
					return false;

			case WM_MOUSEMOVE:
				if (!_mouse_in)
				{
					TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0 };

					_mouse_in = true;
					::TrackMouseEvent(&tme);
					_cursor_manager->push(_cursor_manager->get(cursor_manager::arrow));
				}
				mouse_move(convert_mouse_modifiers(wparam), pt);
				break;

			case WM_MOUSELEAVE:
				_mouse_in = false;
				mouse_leave();
				_cursor_manager->pop();
				break;

			case WM_LBUTTONDOWN: case WM_RBUTTONDOWN:
				fn = &mouse_input::mouse_down;
				break;

			case WM_LBUTTONUP: case WM_RBUTTONUP:
				fn = &mouse_input::mouse_up;
				break;

			case WM_LBUTTONDBLCLK: case WM_RBUTTONDBLCLK:
				fn = &mouse_input::mouse_double_click;
				break;

			case WM_MOUSEHWHEEL:
			case WM_MOUSEWHEEL:
				const int wheel_delta = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;

				helpers::screen_to_client(pt, hwnd);
				mouse_scroll(convert_mouse_modifiers(wparam), pt, WM_MOUSEHWHEEL == message ? wheel_delta : 0,
					WM_MOUSEWHEEL == message ? wheel_delta : 0);
				break;
			}

			switch(message)
			{
			case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
				mouse_click(fn, mouse_input::left, convert_mouse_modifiers(wparam), pt);
				break;

			case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
				mouse_click(fn, mouse_input::right, convert_mouse_modifiers(wparam), pt);
				break;
			}

			result = 0;
			return true;
		}
	}
}
