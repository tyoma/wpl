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

#include "visual.h"

#include <agge/types.h>
#include <agge.text/text_engine.h>

namespace wpl
{
	enum valign {	va_top, va_bottom, va_center,	};


	template <typename T>
	inline T width(const agge::rect<T> &rect_)
	{	return rect_.x2 - rect_.x1;	}

	template <typename T>
	inline T height(const agge::rect<T> &rect_)
	{	return rect_.y2 - rect_.y1;	}

	template <typename T>
	inline void inflate(agge::rect<T> &rect_, T dx, T dy)
	{	rect_.x1 -= dx, rect_.y1 -= dy, rect_.x2 += dx, rect_.y2 += dy;	}

	template <typename T>
	inline void offset(agge::rect<T> &rect_, T dx, T dy)
	{	rect_.x1 += dx, rect_.y1 += dy, rect_.x2 -= dx, rect_.y2 -= dy;	}

	inline void render_string(gcontext::rasterizer_type &target, const std::wstring &text,
		gcontext::text_engine_type &text_engine, const agge::font &font, const agge::rect_r &box,
		agge::layout::halign halign, valign valign_)
	{
		const auto align_vcenter = [&] () -> agge::real_t {
			const auto gm = font.get_metrics();
			const auto fh = gm.ascent + gm.descent;

			return 0.5f * wpl::height(box) + fh - gm.descent;
		};
		const auto y = valign_ == va_top ? box.y1 + font.get_metrics().ascent : valign_ == va_bottom
			? box.y2 - font.get_metrics().descent : align_vcenter();
		const auto x = halign == agge::layout::near ? box.x1 : halign == agge::layout::far ? box.x2
			: box.x1 + 0.5f * wpl::width(box);

		text_engine.render_string(target, font, text.c_str(), halign, x, y, wpl::width(box));
	}
}
