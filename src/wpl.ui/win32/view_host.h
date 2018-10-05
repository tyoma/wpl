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

#include <wpl/ui/view_host.h>

#include <wpl/ui/view.h>
#include <wpl/ui/win32/window.h>

namespace wpl
{
	namespace ui
	{
		namespace win32
		{
			class view_host : public wpl::ui::view_host
			{
			public:
				view_host(HWND hwnd, const window::user_handler_t &user_handler);
				~view_host();

				virtual void set_view(const std::shared_ptr<view> &v);

			private:
				LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);
				void dispatch_mouse(UINT message, WPARAM wparam, LPARAM lparam);
				void reposition_native_views() throw();

			private:
				window::user_handler_t _user_handler;
				std::shared_ptr<window> _window;
				std::shared_ptr<view> _view;
				gcontext::surface_type _surface;
				gcontext::rasterizer_ptr _rasterizer;
				gcontext::renderer_type _renderer;
				slot_connection _invalidate_connection, _capture_connection;
				std::vector<visual::positioned_native_view> _positioned_views;
				bool _mouse_in : 1;
			};
		}
	}
}
