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

#pragma once

#include "window.h"

#include "../view.h"
#include "../view_host.h"

namespace wpl
{
	namespace win32
	{
		class view_host : public wpl::view_host
		{
		public:
			view_host(HWND hwnd, const window::user_handler_t &user_handler);
			~view_host();

			virtual void set_view(const std::shared_ptr<view> &v);
			virtual void set_background_color(agge::color color);

		private:
			LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);
				
			void dispatch_key(UINT message, WPARAM wparam, LPARAM lparam);
			void update_modifier(UINT message, unsigned code);
			void dispatch_tab();

			void dispatch_mouse(UINT message, WPARAM wparam, LPARAM lparam);

			void resize_view(unsigned cx, unsigned cy) throw();
			static mouse_input::mouse_buttons get_button(UINT message);
			void set_focus(std::vector<keyboard_input::tabbed_control>::const_iterator focus_i);

		private:
			window::user_handler_t _user_handler;
			std::shared_ptr<window> _window;
			std::shared_ptr<view> _view;
			std::vector<keyboard_input::tabbed_control> _tabbed_controls;
			std::vector<keyboard_input::tabbed_control>::const_iterator _focus_i;
			std::shared_ptr<keyboard_input> _focus;
			gcontext::surface_type _surface;
			gcontext::rasterizer_ptr _rasterizer;
			gcontext::renderer_type _renderer;
			std::vector<slot_connection> _connections;
			std::vector<visual::positioned_native_view> _positioned_views;
			agge::color _background_color;
			unsigned _input_modifiers;
			bool _mouse_in : 1;
		};
	}
}
