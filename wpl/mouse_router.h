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

#include "concepts.h"
#include "control.h"
#include "input.h"

#include <vector>

namespace wpl
{
	struct mouse_router_host
	{
		virtual void request_focus(std::shared_ptr<keyboard_input> input) = 0;
		virtual std::shared_ptr<void> capture_mouse() = 0;
	};

	class mouse_router : noncopyable
	{
	public:
		mouse_router(const std::vector<placed_view> &views, mouse_router_host &host);

		void reload_views();
		std::shared_ptr<view> from(agge::point<int> &point) const;

		// mouse_input methods
		void mouse_leave();
		void mouse_move(int depressed, agge::point<int> point);
		void mouse_click(void (mouse_input::*fn)(mouse_input::mouse_buttons button, int depressed, int x, int y),
			mouse_input::mouse_buttons button_, int depressed, agge::point<int> point);
		void mouse_scroll(int depressed, agge::point<int> point, int delta_x, int delta_y);

	private:
		typedef std::pair<std::shared_ptr<void> /*handle*/, size_t /*index*/> capture_target;
	
	private:
		std::shared_ptr<view> switch_mouse_over(agge::point<int> &point);

	private:
		const std::vector<placed_view> &_views;
		mouse_router_host &_host;
		std::shared_ptr<view> _mouse_over;
		std::weak_ptr<capture_target> _capture_target;
		std::vector<slot_connection> _connections;
	};
}
