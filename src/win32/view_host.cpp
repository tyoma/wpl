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

#include <wpl/win32/view_host.h>

#include <wpl/cursor.h>
#include <wpl/helpers.h>
#include <wpl/win32/helpers.h>
#include <wpl/win32/native_view.h>

#include <algorithm>
#include <olectl.h>
#include <tchar.h>

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
			struct empty_root : control
			{
				virtual void layout(const placed_view_appender& /*append_view*/, const agge::box<int>& /*box*/) override
				{	}
			};
		}

		view_host::view_host(HWND hwnd, const form_context &context_, const window::user_handler_t &user_handler)
			: context(context_), _hwnd(hwnd), _hoverlay(::CreateWindowEx(0, _T("static"), NULL, WS_POPUP, 0, 0, 1, 1, hwnd,
				NULL, NULL, NULL)), _user_handler(user_handler), _root(make_shared<empty_root>()),
				_visual_router(hwnd, _views, context_), _visual_router_overlay(_hoverlay, _overlay_views, context_),
				_mouse_router(_views, *this, context_.cursor_manager_), _keyboard_router(_views, *this)
		{
			_window = window::attach(hwnd, bind(&view_host::wndproc, this, _1, _2, _3, _4));
			_window_overlay = window::attach(_hoverlay, bind(&view_host::wndproc_overlay, this, _1, _2, _3, _4));
		}

		view_host::~view_host()
		{	}

		void view_host::set_root(shared_ptr<control> root)
		{
			const auto layout = [this] (bool hierarchy_changed) {
				RECT rc;

				::GetClientRect(_hwnd, &rc);
				layout_views(create_box<int>(rc.right, rc.bottom));
				if (hierarchy_changed)
				{
					_visual_router.reload_views();
					_visual_router_overlay.reload_views();
					_mouse_router.reload_views();
					_keyboard_router.reload_views();
				}
			};

			_root = root ? root : make_shared<empty_root>();
			layout(true);
			_layout_changed_connection = _root->layout_changed += layout;
		}

		void view_host::request_focus(shared_ptr<keyboard_input> input)
		{
			if (_keyboard_router.set_focus(input.get()))
				::SetFocus(_hwnd);
		}

		shared_ptr<void> view_host::capture_mouse()
		{
			shared_ptr<bool> h(new bool(false), [] (bool *p) {
				if (*p)
					::ReleaseCapture();
				delete p;
			});

			::SetCapture(_hwnd);
			return *h = true, _capture_handle = h, h;
		}

		void view_host::set_focus(native_view &nview)
		{	::SetFocus(nview.get_window());	}

		LRESULT view_host::passthrough(UINT message, WPARAM wparam, LPARAM lparam,
			const window::original_handler_t &previous)
		{	return previous(message, wparam, lparam);	}

		LRESULT view_host::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
		{
			LRESULT result;

			if (_visual_router.handle_message(result, _hwnd, message, wparam, lparam))
				return result;
			else if (_mouse_router.handle_message(result, _hwnd, message, wparam, lparam))
				return result;
			else if (_keyboard_router.handle_message(result, _hwnd, message, wparam, lparam))
				return result;

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

			case WM_CAPTURECHANGED:
				if (const auto h = _capture_handle.lock())
					*h = false;
				return 0;

			case WM_SIZE:
				layout_views(create_box<int>(LOWORD(lparam), HIWORD(lparam)));
				break;
			}
			return _user_handler(message, wparam, lparam, previous);
		}

		LRESULT view_host::wndproc_overlay(UINT message, WPARAM wparam, LPARAM lparam,
			const window::original_handler_t &previous)
		{
			LRESULT result;

			if (_visual_router_overlay.handle_message(result, _hoverlay, message, wparam, lparam))
				return result;
			return previous(message, wparam, lparam);
		}

		void view_host::layout_views(const box<int> &box_)
		{
			_views.clear();
			_overlay_views.clear();
			_root->layout([this] (const placed_view &pv) {
				(pv.overlay ? _overlay_views : _views).emplace_back(pv);
			}, box_);

			helpers::defer_window_pos dwp(count_if(_views.begin(), _views.end(), [] (const placed_view &pv) {
				return !!pv.native;
			}));

			for (auto i = _views.begin(); i != _views.end(); ++i)
			{
				if (i->native)
					dwp.update_location(i->native->get_window(_hwnd), i->location);
			}
			if (!_overlay_views.empty())
			{
				rect_i overlay_rect = {};

				for (auto i = _overlay_views.begin(); i != _overlay_views.end(); ++i)
				{
					if (i == _overlay_views.begin())
						overlay_rect = i->location;
					else
						unite(overlay_rect, i->location);
				}
				_visual_router_overlay.set_offset(create_vector(overlay_rect.x1, overlay_rect.y1));
				helpers::client_to_screen(overlay_rect, _hwnd);
				::MoveWindow(_hoverlay, overlay_rect.x1, overlay_rect.y1,
					wpl::width(overlay_rect), wpl::height(overlay_rect), TRUE);
			}
			::ShowWindow(_hoverlay, _overlay_views.empty() ? SW_HIDE : SW_SHOWNA);
			::InvalidateRect(_hwnd, NULL, TRUE);
		}
	}
}
