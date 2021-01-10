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

#include "helpers.h"

#include <agge.text/text_engine.h>
#include <wpl/helpers.h>

using namespace agge;
using namespace std;

namespace wpl
{
	void render_string(gcontext::rasterizer_type &target, const wstring &text, gcontext::text_engine_type &text_engine_,
		const font &font_, const rect_r &box_, text_alignment halign, text_alignment valign_)
	{
		const auto align_vcenter = [&] () -> real_t {
			const auto gm = font_.get_metrics();
			const auto fh = gm.ascent + gm.descent;

			return box_.y1 + 0.5f * wpl::height(box_) + 0.5f * fh - gm.descent;
		};
		const auto y = valign_ == align_near ? box_.y1 + font_.get_metrics().ascent : valign_ == align_far
			? box_.y2 - font_.get_metrics().descent : align_vcenter();
		const auto x = halign == align_near ? box_.x1 : halign == align_far
			? box_.x2 : box_.x1 + 0.5f * wpl::width(box_);

		text_engine_.render_string(target, font_, text.c_str(), halign, x, y, wpl::width(box_));
	}

	placed_view_appender offset(const placed_view_appender &inner, int dx, int dy, int tab_override)
	{
		// TODO: use custom appender functor, as function may allocate storage dynamically.

		return [&inner, dx, dy, tab_override] (placed_view pv) {
			wpl::offset(pv.location, dx, dy);
			pv.tab_order = pv.tab_order ? tab_override ? tab_override : pv.tab_order : 0;
			inner(pv);
		};
	}

	placed_view_appender offset(const placed_view_appender &inner, int d, bool horizontal, int tab_override)
	{
		return [&inner, d, horizontal, tab_override] (placed_view pv) {
			wpl::offset(pv.location, horizontal ? d : 0, horizontal ? 0 : d);
			pv.tab_order = pv.tab_order ? tab_override ? tab_override : pv.tab_order : 0;
			inner(pv);
		};
	}
}
