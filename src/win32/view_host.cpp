//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <wpl/win32/view_host.h>

#include <wpl/cursor.h>
#include <wpl/helpers.h>
#include <wpl/visual_router.h>
#include <wpl/win32/native_view.h>

#include <algorithm>
#include <iterator>
#include <olectl.h>
#include <windowsx.h>

#pragma warning(disable: 4355)

using namespace agge;
using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace win32
	{
		namespace
		{
			class paint_sequence : noncopyable, public PAINTSTRUCT
			{
			public:
				paint_sequence(HWND hwnd)
					: _hwnd(hwnd)
				{	::BeginPaint(_hwnd, this);	}

				~paint_sequence() throw()
				{	::EndPaint(_hwnd, this);	}

				count_t width() const throw()
				{	return rcPaint.right - rcPaint.left;	}

				count_t height() const throw()
				{	return rcPaint.bottom - rcPaint.top;	}

			private:
				HWND _hwnd;
			};

			agge::point<int> unpack_point(LPARAM lparam)
			{
				const agge::point<int> pt = {	GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)	};
				return pt;
			}

			void screen_to_client(HWND hwnd, agge::point<int> &point_)
			{
				POINT pt = { point_.x, point_.y };

				::ScreenToClient(hwnd, &pt);
				point_.x = pt.x, point_.y = pt.y;
			}

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


		view_host::view_host(HWND hwnd, const form_context &context_, const window::user_handler_t &user_handler)
			: context(context_), _user_handler(user_handler), _rasterizer(new gcontext::rasterizer_type), _mouse_in(false),
				_input_modifiers(0), _visual_router(_views, *this), _mouse_router(_views, *this),
				_keyboard_router(_views, *this)
		{	_window = window::attach(hwnd, bind(&view_host::wndproc, this, _1, _2, _3, _4));	}

		view_host::~view_host()
		{	}

		void view_host::set_root(shared_ptr<control> root)
		{
			const auto layout = [this] {
				RECT rc;

				::GetClientRect(_window->hwnd(), &rc);
				layout_views(rc.right, rc.bottom);
			};
			const auto reload = [this] {
				_visual_router.reload_views();
				_mouse_router.reload_views();
				_keyboard_router.reload_views();
			};

			_root = root;
			layout();
			reload();
			_layout_changed_connection = root ? root->layout_changed += [layout, reload] (bool hierarchy_changed) {
				layout();
				if (hierarchy_changed)
					reload();
			} : slot_connection();
		}

		void view_host::invalidate(const agge::rect_i &area)
		{
			RECT rc = {	area.x1, area.y1, area.x2, area.y2	};

			::InvalidateRect(_window->hwnd(), &rc, FALSE);
		}

		void view_host::request_focus(shared_ptr<keyboard_input> input)
		{
			if (_keyboard_router.set_focus(input.get()))
				::SetFocus(_window->hwnd());
		}

		shared_ptr<void> view_host::capture_mouse()
		{
			shared_ptr<bool> h(new bool(false), [] (bool *p) {
				if (*p)
					::ReleaseCapture();
				delete p;
			});

			::SetCapture(_window->hwnd());
			return *h = true, _capture_handle = h, h;
		}

		void view_host::set_focus(native_view &nview)
		{	::SetFocus(nview.get_window());	}

		LRESULT view_host::passthrough(UINT message, WPARAM wparam, LPARAM lparam,
			const window::original_handler_t &previous)
		{	return previous(message, wparam, lparam);	}

		LRESULT view_host::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
		{
			switch (message)
			{
			case WM_COMMAND:
			case WM_CTLCOLORBTN:
			case WM_CTLCOLOREDIT:
			case WM_CTLCOLORDLG:
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORMSGBOX:
			case WM_CTLCOLORSCROLLBAR:
			case WM_CTLCOLORSTATIC:
				return ::SendMessage(reinterpret_cast<HWND>(lparam), OCM__BASE + message, wparam, lparam);

			case WM_NOTIFY:
				return ::SendMessage(reinterpret_cast<const NMHDR*>(lparam)->hwndFrom, OCM_NOTIFY, wparam, lparam);

			case WM_SETCURSOR:
				if (HTCLIENT == LOWORD(lparam) && _window->hwnd() == reinterpret_cast<HWND>(wparam))
					return TRUE;
				else
					break;

			case WM_CAPTURECHANGED:
				if (const auto h = _capture_handle.lock())
					*h = false;
				return 0;

			case WM_SIZE:
				layout_views(LOWORD(lparam), HIWORD(lparam));
				break;

			case WM_KILLFOCUS:
				_keyboard_router.set_focus(nullptr);
				break;

			case WM_KEYDOWN:
			case WM_KEYUP:
				dispatch_key(message, wparam, lparam);
				break;

			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_RBUTTONDBLCLK:
				dispatch_mouse_click(message, wparam, lparam);
				break;

			case WM_MOUSEMOVE:
			case WM_MOUSELEAVE:
				dispatch_mouse_move(message, wparam, lparam);
				break;

			case WM_MOUSEHWHEEL:
			case WM_MOUSEWHEEL:
				dispatch_mouse_scroll(message, wparam, lparam);
				break;

			case WM_ERASEBKGND:
				return TRUE;

			case WM_PAINT:
				paint_sequence ps(_window->hwnd());
				const vector_i offset = { ps.rcPaint.left, ps.rcPaint.top };
				gcontext ctx(*context.backbuffer, *context.renderer, *context.text_engine, offset,
					create_rect<int>(ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom));

				context.backbuffer->resize(ps.width(), ps.height());
				_rasterizer->reset();
				_visual_router.draw(ctx, _rasterizer);
				context.backbuffer->blit(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.width(), ps.height());
				return 0;
			}
			return _user_handler(message, wparam, lparam, previous);
		}

		void view_host::dispatch_key(UINT message, WPARAM wparam, LPARAM /*lparam*/)
		{
			if (update_modifier(message, static_cast<int>(wparam)))
				return;

			// In here we rely that VK_* key mappings are in accordance with keyboard_input::special_keys.
			switch (message)
			{
			case WM_KEYDOWN: _keyboard_router.key_down(static_cast<int>(wparam), _input_modifiers); break;
			case WM_KEYUP: _keyboard_router.key_up(static_cast<int>(wparam), _input_modifiers); break;
			}
		}

		bool view_host::update_modifier(UINT message, unsigned code)
		{
			auto modifier_flag = 0u;

			switch (code)
			{
			case VK_SHIFT:	modifier_flag = keyboard_input::shift;	break;
			case VK_CONTROL:	modifier_flag = keyboard_input::control;	break;
			default:	return false;
			}
			if (WM_KEYDOWN == message)
				_input_modifiers |= modifier_flag;
			else if (WM_KEYUP == message)
				_input_modifiers &= ~modifier_flag;
			return true;
		}

		void view_host::dispatch_mouse_move(UINT message, WPARAM wparam, LPARAM lparam)
		{
			switch (message)
			{
			case WM_MOUSELEAVE:
				_mouse_in = false;
				_mouse_router.mouse_leave();
				context.cursor_manager_->pop();
				break;

			case WM_MOUSEMOVE:
				if (!_mouse_in)
				{
					TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, _window->hwnd(), 0 };

					_mouse_in = true;
					TrackMouseEvent(&tme);
					context.cursor_manager_->push(context.cursor_manager_->get(cursor_manager::arrow));
				}
				_mouse_router.mouse_move(convert_mouse_modifiers(wparam), unpack_point(lparam));
				break;
			}
		}

		void view_host::dispatch_mouse_click(UINT message, WPARAM wparam, LPARAM lparam)
		{
			void (mouse_input::*fn)(mouse_input::mouse_buttons button_, int depressed, int x, int y);
			mouse_input::mouse_buttons button;

			switch (message)
			{
			case WM_LBUTTONDOWN: case WM_RBUTTONDOWN:	fn = &mouse_input::mouse_down;	break;
			case WM_LBUTTONUP: case WM_RBUTTONUP:	fn = &mouse_input::mouse_up;	break;
			case WM_LBUTTONDBLCLK: case WM_RBUTTONDBLCLK:	fn = &mouse_input::mouse_double_click;	break;
			default:	return;
			}
			switch(message)
			{
			case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:	 button = mouse_input::left;	break;
			case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:	 button = mouse_input::right;	break;
			default:	return;
			}
			_mouse_router.mouse_click(fn, button, convert_mouse_modifiers(wparam), unpack_point(lparam));
		}

		void view_host::dispatch_mouse_scroll(UINT message, WPARAM wparam, LPARAM lparam)
		{
			auto pt = unpack_point(lparam);
			const int wheel_delta = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;

			screen_to_client(_window->hwnd(), pt);
			_mouse_router.mouse_scroll(convert_mouse_modifiers(wparam), pt, WM_MOUSEHWHEEL == message ? wheel_delta : 0,
				WM_MOUSEWHEEL == message ? wheel_delta : 0);
		}

		void view_host::layout_views(int width, int height)
		{
			const agge::box<int> b = { width, height };

			_views.clear();
			if (_root)
				_root->layout([this] (const placed_view &pv) {	_views.emplace_back(pv);	}, b);

			const auto n = count_if(_views.begin(), _views.end(), [] (const placed_view &pv) {
				return !!pv.native;
			});

			const auto hwnd = _window->hwnd();
			auto hdwp = ::BeginDeferWindowPos(static_cast<int>(n));

			for (auto i = _views.begin(); i != _views.end(); ++i)
			{
				if (const auto nv = i->native)
				{
					hdwp = ::DeferWindowPos(hdwp, nv->get_window(hwnd), NULL, i->location.x1, i->location.y1,
						wpl::width(i->location), wpl::height(i->location), SWP_NOZORDER);
				}
			}
			::EndDeferWindowPos(hdwp);
			::InvalidateRect(hwnd, NULL, TRUE);
		}
	}
}
