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

#include <algorithm>
#include <vector>

namespace wpl
{
	struct keyboard_input;

	struct keyboard_router_host
	{
		virtual void set_focus() = 0;
		virtual void set_focus(native_view &nview) = 0;
	};

	class keyboard_router : noncopyable
	{
	public:
		keyboard_router(const std::vector<placed_view> &views, keyboard_router_host &host);

		void reload_views();
		void set_focus(const keyboard_input *input);

		// keyboard_input methods
		void key_down(unsigned code, int modifiers);
		void character(wchar_t symbol, unsigned repeats, int modifiers);
		void key_up(unsigned code, int modifiers);

		template <typename PredicateT>
		void got_native_focus(const PredicateT &predicate);
		void got_focus();
		void lost_focus();

	protected:
		typedef std::vector<placed_view> placed_views;

	private:
		void move_focus(placed_views::const_iterator to);

	private:
		const placed_views &_views;
		keyboard_router_host &_host;
		bool _has_focus;
		placed_views _ordered;
		placed_views::const_iterator _focus;
	};



	template <typename PredicateT>
	inline void keyboard_router::got_native_focus(const PredicateT &predicate)
	{
		_focus = std::find_if(_ordered.begin(), _ordered.end(), [predicate] (const placed_view &pv) {
			return pv.native && predicate(*pv.native);
		});
	}
}
