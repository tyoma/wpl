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

#include "signals.h"

namespace wpl
{
	struct keyboard_input
	{
		typedef std::pair< int /*tab index*/, std::shared_ptr<keyboard_input> > tabbed_control;

		enum special_keys {
			left, up, right, down, enter, tab, home, end, page_down, page_up,
		};

		enum modifier_keys {
			base = 1,
			shift = base << 0,
			control = base << 1,
			command = base << 2,
			alt = base << 3,
			next = base << 4,
		};

		virtual ~keyboard_input() {	}

		virtual void get_tabbed_controls(std::vector<tabbed_control> &tabbed_controls, bool do_clear = true);

		virtual void key_down(unsigned code, int modifiers);
		virtual void character(wchar_t symbol, unsigned repeats, int modifiers);
		virtual void key_up(unsigned code, int modifiers);

		virtual void got_focus();
		virtual void lost_focus();

		signal<void (const std::shared_ptr<keyboard_input> &view_)> request_focus;
	};

	struct mouse_input
	{
		enum mouse_buttons {
			base = keyboard_input::next,
			left = base << 0,
			middle = base << 1,
			right = base << 2,
		};

		virtual ~mouse_input() {	}

		virtual void mouse_enter();
		virtual void mouse_leave();

		virtual void mouse_move(int depressed, int x, int y);

		virtual void mouse_down(mouse_buttons button_, int depressed, int x, int y);
		virtual void mouse_up(mouse_buttons button_, int depressed, int x, int y);
		virtual void mouse_double_click(mouse_buttons button_, int depressed, int x, int y);

		virtual void mouse_scroll(int depressed, int x, int y, int delta_x, int delta_y);

		virtual void lost_capture();

		signal<void(std::shared_ptr<void> &handle)> capture;
	};
}
