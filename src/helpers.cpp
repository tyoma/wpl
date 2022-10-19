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

#include "helpers.h"

#include <agge.text/text_engine.h>
#include <wpl/helpers.h>

using namespace agge;
using namespace std;

namespace wpl
{
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
