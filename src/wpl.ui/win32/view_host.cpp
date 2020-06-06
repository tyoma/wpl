//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
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
					_mouse_in(false), _input_modifiers(0)
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

			void view_host::set_background_color(agge::color color)
			{
				_background_color = color;
				::InvalidateRect(_window->hwnd(), NULL, TRUE);
			}

			LRESULT view_host::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
			{
				switch (message)
				{
				case WM_COMMAND:
					return ::SendMessage(reinterpret_cast<HWND>(lparam), OCM_COMMAND, wparam, lparam);

				case WM_NOTIFY:
					return SendMessage(reinterpret_cast<const NMHDR*>(lparam)->hwndFrom, OCM_NOTIFY, wparam, lparam);

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
					case WM_MOUSEMOVE:
					case WM_MOUSELEAVE:
						dispatch_mouse(message, wparam, lparam);
						break;

					case WM_PAINT:
						paint_sequence ps(_window->hwnd());
						rect_i update_area = { ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom };
						rect_i update_size = { 0, 0, update_area.x2 - update_area.x1, update_area.y2 - update_area.y1 };

						_surface.resize(ps.width(), ps.height());
						fill(_surface, update_size,
							blender_solid_color<simd::blender_solid_color, order_bgra>(_background_color));

						gcontext ctx(_surface, _renderer, update_area);

						_rasterizer->reset();
						_view->draw(ctx, _rasterizer);
						_surface.blit(ps.hdc, update_area.x1, update_area.y1, ps.width(), ps.height());
						return 0;
					}
				}
				return _user_handler(message, wparam, lparam, previous);
			}

			void view_host::dispatch_key(UINT message, WPARAM wparam, LPARAM /*lparam*/)
			{
				int code = 0;

				switch (wparam)
				{
				case VK_CONTROL: update_modifier(message, keyboard_input::control); return;
				case VK_SHIFT: update_modifier(message, keyboard_input::shift); return;

				case VK_LEFT: code = keyboard_input::left; break;
				case VK_RIGHT: code = keyboard_input::right; break;
				case VK_UP: code = keyboard_input::up; break;
				case VK_DOWN: code = keyboard_input::down; break;
				case VK_TAB: code = keyboard_input::tab; break;
				case VK_RETURN: code = keyboard_input::enter; break;

				default: code = static_cast<int>(wparam); break;
				}
				switch (message)
				{
				case WM_KEYDOWN: _view->key_down(code, _input_modifiers); break;
				case WM_KEYUP: _view->key_up(code, _input_modifiers); break;
				}
			}

			void view_host::update_modifier(UINT message, unsigned code)
			{
				if (WM_KEYDOWN == message)
					_input_modifiers |= code;
				else if (WM_KEYUP == message)
					_input_modifiers &= ~code;
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
				case WM_RBUTTONDOWN:
					_view->mouse_down(get_button(message), _input_modifiers, x, y);
					break;

				case WM_LBUTTONUP:
				case WM_RBUTTONUP:
					_view->mouse_up(get_button(message), _input_modifiers, x, y);
					break;

				case WM_LBUTTONDBLCLK:
				case WM_RBUTTONDBLCLK:
					_view->mouse_double_click(get_button(message), _input_modifiers, x, y);
					break;
				}
			}

			void view_host::resize_view(unsigned cx, unsigned cy) throw()
			{
				_view->resize(cx, cy, _positioned_views);

				const HWND hwnd = _window->hwnd();
				HDWP hdwp = ::BeginDeferWindowPos(static_cast<int>(_positioned_views.size()));

				for (visual::positioned_native_views::const_iterator i = _positioned_views.begin();
					i != _positioned_views.end(); ++i)
				{
					HWND h = i->get_view().get_window(hwnd);

					hdwp = ::DeferWindowPos(hdwp, h, NULL, i->location.left, i->location.top,
						i->location.width, i->location.height, SWP_NOZORDER);
				}
				::EndDeferWindowPos(hdwp);
				_positioned_views.clear();
			}

			mouse_input::mouse_buttons view_host::get_button(UINT message)
			{
				switch(message)
				{
				case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK: return mouse_input::left;
				case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK: return mouse_input::right;
				default: return mouse_input::base;
				}
			}
		}

		shared_ptr<view_host> wrap_view_host(HWND hwnd)
		{	return shared_ptr<view_host>(new win32::view_host(hwnd, &win32::passthrough));	}
	}
}
