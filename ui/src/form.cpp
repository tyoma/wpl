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

#include <ui/form.h>

#include <base/concepts.h>
#include <ui/win32/window.h>

#include <algorithm>
#include <iterator>
#include <tchar.h>
#include <olectl.h>
#include <windows.h>
#include <windowsx.h>

using namespace agge;
using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace ui
	{
		namespace
		{
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

			class form_impl : public form
			{
			public:
				form_impl();
				~form_impl();

			private:
				virtual void set_view(const shared_ptr<view> &v);
				virtual void set_visible(bool value);
				virtual void set_caption(const std::wstring &caption);

				LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);

				void dispatch_mouse(UINT message, WPARAM wparam, LPARAM lparam);

			private:
				shared_ptr<window> _window;
				shared_ptr<view> _view;
				gcontext::surface_type _surface;
				gcontext::rasterizer_ptr _rasterizer;
				gcontext::renderer_type _renderer;
				slot_connection _invalidate_connection, _capture_connection;
				bool _mouse_in : 1;
			};



			form_impl::form_impl()
				: _surface(1, 1, 0), _rasterizer(new gcontext::rasterizer_type), _renderer(1), _mouse_in(false)
			{
				HWND hwnd = ::CreateWindow(_T("#32770"), 0, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0, 0, 100, 20, 0, 0, 0, 0);

				_window = window::attach(hwnd, bind(&form_impl::wndproc, this, _1, _2, _3, _4));
			}

			form_impl::~form_impl()
			{	::DestroyWindow(_window->hwnd());	}

			void form_impl::set_view(const shared_ptr<view> &v)
			{
				_view = v;
				_invalidate_connection = v->invalidate += [this] (const agge::rect_i *rc) {
					RECT rc2;

					if (rc)
						rc2.left = rc->x1, rc2.top = rc->y1, rc2.right = rc->x2, rc2.bottom = rc->y2;
					::InvalidateRect(_window->hwnd(), rc ? &rc2 : NULL, FALSE);
				};
				_capture_connection = v->capture += [this] (shared_ptr<void> &handle) {
					if (::SetCapture(_window->hwnd()), ::GetCapture() == _window->hwnd())	
						handle.reset(new capture_context);
					else
						handle.reset(); // untested
				};
			}

			void form_impl::set_visible(bool value)
			{	::ShowWindow(_window->hwnd(), value ? SW_SHOW : SW_HIDE);	}

			void form_impl::set_caption(const std::wstring &caption)
			{	::SetWindowTextW(_window->hwnd(), caption.c_str());	}

			LRESULT form_impl::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
			{
				switch (message)
				{
				case WM_CLOSE:
					close();
					return 0;

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
						_view->resize(LOWORD(lparam), HIWORD(lparam));
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
				return previous(message, wparam, lparam);
			}

			void form_impl::dispatch_mouse(UINT message, WPARAM /*wparam*/, LPARAM lparam)
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
		}

		shared_ptr<form> form::create()
		{
			return shared_ptr<form>(new form_impl());
		}
	}
}
