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

#include <wpl/layout.h>

#include "helpers.h"

#include <agge/math.h>
#include <algorithm>
#include <wpl/helpers.h>
#include <wpl/static_visitor.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace
	{
		struct calculate_width : unit_visitor< pair<size_t /*columns*/, double /*column_width*/> >
		{
			calculate_width(double container_width)
				: _container_width(container_width)
			{	}

			pair<size_t, double> visit_pixel(double base_width) const
			{
				auto columns = _container_width > base_width ? static_cast<size_t>(_container_width / base_width) : 1u;

				return make_pair(columns, _container_width / columns);
			}

		private:
			double _container_width;
		};
	}


	bool staggered::next::operator <(const next &rhs) const
	{	return bottom > rhs.bottom ? true : bottom < rhs.bottom ? false : x0 > rhs.x0;	}


	staggered::staggered()
		: _base_width(pixels(0))
	{	}

	void staggered::add(shared_ptr<control> child)
	{
		_children.push_back(child);
		container::add(*child);
	}

	void staggered::set_base_width(display_unit width)
	{
		_base_width = width;
		layout_changed(false);
	}

	void staggered::layout(const placed_view_appender &append_view, const box<int> &box_)
	{
		const auto cw = _base_width.apply(calculate_width(box_.w));
		auto x = 0;
		auto remainder = 0.0;

		_next_items_buffer.clear();
		for (auto i = 0u; i != cw.first; ++i)
		{
			const next item = {	0, x, static_cast<int>(iround(static_cast<real_t>(cw.second + remainder)))	};

			remainder = cw.second - item.width;
			x += item.width;
			_next_items_buffer.push_back(item);
			push_heap(_next_items_buffer.begin(), _next_items_buffer.end());
		}
		for (auto i = _children.begin(); i != _children.end(); ++i)
		{
			pop_heap(_next_items_buffer.begin(), _next_items_buffer.end());

			auto &item = _next_items_buffer.back();
			const auto item_box = create_box(item.width, (*i)->min_height(item.width));

			(*i)->layout(offset(append_view, item.x0, item.bottom, 0), item_box);
			item.bottom += item_box.h;
			push_heap(_next_items_buffer.begin(), _next_items_buffer.end());
		}
	}
}
