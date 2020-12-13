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

#include <wpl/helpers.h>

#include <agge.text/text_engine.h>

using namespace agge;
using namespace std;

namespace wpl
{
	void render_string(gcontext::rasterizer_type &target, const wstring &text, gcontext::text_engine_type &text_engine_,
		const font &font_, const rect_r &box_, layout::halign halign, valign valign_)
	{
		const auto align_vcenter = [&] () -> real_t {
			const auto gm = font_.get_metrics();
			const auto fh = gm.ascent + gm.descent;

			return 0.5f * wpl::height(box_) + fh - gm.descent;
		};
		const auto y = valign_ == va_top ? box_.y1 + font_.get_metrics().ascent : valign_ == va_bottom
			? box_.y2 - font_.get_metrics().descent : align_vcenter();
		const auto x = halign == layout::near_ ? box_.x1 : halign == layout::far_
			? box_.x2 : box_.x1 + 0.5f * wpl::width(box_);

		text_engine_.render_string(target, font_, text.c_str(), halign, x, y, wpl::width(box_));
	}
}
