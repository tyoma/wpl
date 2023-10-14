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

#pragma once

#include "../concepts.h"
#include "../factory_context.h"
#include "../view_host.h"

#include "helpers.h"
#include "keyboard_router.h"
#include "mouse_router.h"
#include "visual_router.h"
#include "window.h"

namespace wpl
{
	namespace win32
	{
		class view_host : public wpl::view_host, public mouse_router_host, public keyboard_router_host, noncopyable
		{
		public:
			view_host(HWND hwnd, const form_context &context_,
				const window::user_handler_t &user_handler = &view_host::passthrough);
			~view_host();

			// view_host methods
			virtual void set_root(std::shared_ptr<control> root) override;

			// mouse_router_host methods
			virtual void request_focus(std::shared_ptr<keyboard_input> input) override;
			virtual std::shared_ptr<void> capture_mouse() override;

			// keyboard_router_host methods
			virtual void set_focus(native_view &nview) override;

			static LRESULT passthrough(UINT message, WPARAM wparam, LPARAM lparam,
				const window::original_handler_t &previous);

		public:
			const form_context context;

		private:
			LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);
			LRESULT wndproc_overlay(UINT message, WPARAM wparam, LPARAM lparam,
				const window::original_handler_t &previous);

			void layout_views(const agge::box<int> &box);

		private:
			HWND _hwnd;
			const helpers::window_handle _hoverlay;

			window::user_handler_t _user_handler;
			std::shared_ptr<window> _window, _window_overlay;
			std::shared_ptr<control> _root;
			std::vector<placed_view> _views, _overlay_views;
			slot_connection _layout_changed_connection;
			std::weak_ptr<bool> _capture_handle;
			visual_router _visual_router, _visual_router_overlay;
			mouse_router _mouse_router;
			keyboard_router _keyboard_router;
		};
	}
}
