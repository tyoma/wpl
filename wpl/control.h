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

#include "signal.h"
#include "types.h"

namespace wpl
{
	class native_view;
	struct placed_view;
	struct view;

	typedef std::function<void (const placed_view &pv)> placed_view_appender;

	struct placed_view
	{
		std::shared_ptr<view> regular;
		std::shared_ptr<native_view> native;
		rect_i location;
		int tab_order;
		bool overlay;
	};

	struct control
	{
		virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) = 0;
		virtual int min_height(int for_width = maximum_size) const;
		virtual int min_width(int for_height = maximum_size) const;

		signal<void (bool hierarchy_changed)> layout_changed;
	};



	inline int control::min_height(int /*for_width*/) const
	{	return 0;	}

	inline int control::min_width(int /*for_height*/) const
	{	return 0;	}
}
