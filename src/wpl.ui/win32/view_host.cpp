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

#include "view_host.h"

#include <wpl/ui/win32/controls.h>
#include <wpl/ui/win32/native_view.h>

#include <algorithm>
#include <iterator>
#include <olectl.h>
#include <windowsx.h>

using namespace agge;
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
				LRESULT passthrough(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
				{	return previous(message, wparam, lparam);	}
			}

			class paint_sequence : noncopyable, public PAINTSTRUCT
			{
			public:
				paint_sequence(HWND hwnd)
					: _hwnd(hwnd)
				{	::BeginPaint(_hwnd, this);	}

				~paint_sequence()
				{	::EndPaint(_hwnd, this);	}

				count_t width()
				{	return rcPaint.right - rcPaint.left;	}

				count_t height()
				{	return rcPaint.bottom - rcPaint.top;	}

			private:
				HWND _hwnd;
			};

			struct capture_context : noncopyable
			{
				~capture_context()
				{	::ReleaseCapture();	}
			};



			view_host::view_host(HWND hwnd, const window::user_handler_t &user_handler)
				: _user_handler(user_handler), _surface(1, 1, 0), _rasterizer(new gcontext::rasterizer_type), _renderer(1),
					_mouse_in(false)
			{
				_window = window::attach(hwnd, bind(&view_host::wndproc, this, _1, _2, _3, _4));
			}

			view_host::~view_host()
			{	}

			void view_host::set_view(const shared_ptr<view> &v)
			{
				_view = v;
				_connections.clear();
				if (!v)
					return;
				_connections.push_back(v->invalidate += [this] (const agge::rect_i *rc) {
					RECT rc2;

					if (rc)
						rc2.left = rc->x1, rc2.top = rc->y1, rc2.right = rc->x2, rc2.bottom = rc->y2;
					::InvalidateRect(_window->hwnd(), rc ? &rc2 : NULL, FALSE);
				});
				_connections.push_back(v->capture += [this] (shared_ptr<void> &handle) {
					if (::SetCapture(_window->hwnd()), ::GetCapture() == _window->hwnd())	
						handle.reset(new capture_context);
					else
						handle.reset(); // untested
				});
				_connections.push_back(v->force_layout += [this] () {
					RECT rc;

					::GetClientRect(_window->hwnd(), &rc);
					resize_view(rc.right, rc.bottom);
				});
				v->force_layout();
			}

			LRESULT view_host::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
			{
				switch (message)
				{
				case WM_COMMAND:
					::SendMessage(reinterpret_cast<HWND>(lparam), OCM_COMMAND, wparam, lparam);
					return 0;

				case WM_ERASEBKGND:
					return 1;
				}

				if (_view)
				{
					switch (message)
					{
					case WM_CAPTURECHANGED:
						_view->lost_capture();
						return 0;

					case WM_SIZE:
						resize_view(LOWORD(lparam), HIWORD(lparam));
						break;

					case WM_LBUTTONDOWN:
					case WM_LBUTTONUP:
					case WM_RBUTTONDOWN:
					case WM_RBUTTONUP:
					case WM_MOUSEMOVE:
					case WM_MOUSELEAVE:
						dispatch_mouse(message, wparam, lparam);
						break;

					case WM_PAINT:
						paint_sequence ps(_window->hwnd());
						rect_i update_area = { ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom };

						_surface.resize(ps.width(), ps.height());
						gcontext ctx(_surface, _renderer, update_area);
						_rasterizer->reset();
						_view->draw(ctx, _rasterizer);
						_surface.blit(ps.hdc, update_area.x1, update_area.y1, ps.width(), ps.height());
						return 0;
					}
				}
				return _user_handler(message, wparam, lparam, previous);
			}

			void view_host::dispatch_mouse(UINT message, WPARAM /*wparam*/, LPARAM lparam)
			{
				const int x = GET_X_LPARAM(lparam), y = GET_Y_LPARAM(lparam);

				switch (message)
				{
				case WM_MOUSEMOVE:
					if (!_mouse_in)
					{
						TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, _window->hwnd(), 0 };

						_mouse_in = true;
						TrackMouseEvent(&tme);
						_view->mouse_enter();
					}
					_view->mouse_move(0, x, y);
					break;

				case WM_MOUSELEAVE:
					_mouse_in = false;
					_view->mouse_leave();
					break;

				case WM_LBUTTONDOWN:
					_view->mouse_down(mouse_input::left, 0, x, y);
					break;

				case WM_LBUTTONUP:
					_view->mouse_up(mouse_input::left, 0, x, y);
					break;

				case WM_RBUTTONDOWN:
					_view->mouse_down(mouse_input::right, 0, x, y);
					break;

				case WM_RBUTTONUP:
					_view->mouse_up(mouse_input::right, 0, x, y);
					break;
				}
			}

			void view_host::resize_view(unsigned cx, unsigned cy) throw()
			{
				_view->resize(cx, cy, _positioned_views);

				HDWP hdwp = ::BeginDeferWindowPos(static_cast<int>(_positioned_views.size()));

				for (visual::positioned_native_views::const_iterator i = _positioned_views.begin();
					i != _positioned_views.end(); ++i)
				{
					HWND h = i->get_view().get_window();

					if (::GetParent(h) != _window->hwnd())
					{
						::SetWindowLong(h, GWL_STYLE, (::GetWindowLong(h, GWL_STYLE) | WS_CHILD) & ~(WS_POPUP | WS_OVERLAPPED));
						::SetParent(h, _window->hwnd());
					}
					hdwp = ::DeferWindowPos(hdwp, h, NULL, i->location.left, i->location.top,
						i->location.width, i->location.height, SWP_NOZORDER);
				}
				::EndDeferWindowPos(hdwp);
				_positioned_views.clear();
			}
		}

		shared_ptr<view_host> wrap_view_host(HWND hwnd)
		{	return shared_ptr<view_host>(new win32::view_host(hwnd, &win32::passthrough));	}
	}
}
