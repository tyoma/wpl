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

#pragma once

#include "concepts.h"
#include "control.h"

#include <vector>

namespace wpl
{
	struct keyboard_input;

	struct keyboard_router_host
	{
		virtual void set_focus(native_view &nview) = 0;
	};

	class keyboard_router : noncopyable
	{
	public:
		keyboard_router(const std::vector<placed_view> &views, keyboard_router_host &host);

		void reload_views();
		bool set_focus(const keyboard_input *input);

		// keyboard_input methods
		void key_down(unsigned code, int modifiers);
		void character(wchar_t symbol, unsigned repeats, int modifiers);
		void key_up(unsigned code, int modifiers);

	private:
		typedef std::vector<placed_view> placed_views;

	private:
		void switch_focus(placed_views::const_iterator new_focus);

	private:
		const std::vector<placed_view> &_views;
		keyboard_router_host &_host;
		placed_views _ordered;
		placed_views::const_iterator _focus;
	};
}
