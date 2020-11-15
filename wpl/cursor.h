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

#include "concepts.h"
#include "visual.h"

#include <memory>

namespace wpl
{
	class cursor : gcontext::surface_type, noncopyable
	{
	private:
		typedef gcontext::surface_type base;

	public:
		cursor(unsigned int width_, unsigned int height_, unsigned int hot_x_, unsigned int hot_y_);

		using base::width;
		using base::height;
		using base::row_ptr;
		using base::native;
		const unsigned int hot_x;
		const unsigned int hot_y;
	};

	struct cursor_manager
	{
		enum standard_cursor {
			arrow,
			i_beam,
			crosshair,
			hand,
			h_resize, l_resize, r_resize,
			v_resize, u_resize, b_resize,
		};

		virtual std::shared_ptr<const cursor> get(standard_cursor id) const = 0;
		virtual void set(std::shared_ptr<const cursor> cursor_) = 0;
	};
}
