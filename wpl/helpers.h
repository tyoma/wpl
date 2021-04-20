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

#include "visual.h"

#include <agge/tools.h>
#include <agge.text/font.h>
#include <agge.text/types.h>

namespace wpl
{
	using agge::create_point;
	using agge::create_rect;
	using agge::width;
	using agge::height;

	template <typename T>
	inline void inflate(agge::rect<T> &rect_, T dx, T dy)
	{	rect_.x1 -= dx, rect_.y1 -= dy, rect_.x2 += dx, rect_.y2 += dy;	}

	template <typename T>
	inline void offset(agge::rect<T> &rect_, T dx, T dy)
	{	rect_.x1 += dx, rect_.y1 += dy, rect_.x2 += dx, rect_.y2 += dy;	}

	template <typename T>
	inline bool is_empty(const agge::rect<T> &rect_)
	{	return !(rect_.x1 < rect_.x2) | !(rect_.y1 < rect_.y2);	}

	template <typename T>
	inline void unite(agge::rect<T> &lhs, const agge::rect<T> &rhs)
	{
		if (rhs.x1 < lhs.x1) lhs.x1 = rhs.x1;
		if (rhs.y1 < lhs.y1) lhs.y1 = rhs.y1;
		if (rhs.x2 > lhs.x2) lhs.x2 = rhs.x2;
		if (rhs.y2 > lhs.y2) lhs.y2 = rhs.y2;
	}

	template <typename T>
	inline void intersect(agge::rect<T> &lhs, const agge::rect<T> &rhs)
	{
		if (rhs.x1 > lhs.x1) lhs.x1 = rhs.x1;
		if (rhs.y1 > lhs.y1) lhs.y1 = rhs.y1;
		if (rhs.x2 < lhs.x2) lhs.x2 = rhs.x2;
		if (rhs.y2 < lhs.y2) lhs.y2 = rhs.y2;
	}

	template <typename T>
	inline bool are_intersecting(agge::rect<T> lhs, const agge::rect<T> &rhs)
	{
		intersect(lhs, rhs);
		return !is_empty(lhs);
	}
}
