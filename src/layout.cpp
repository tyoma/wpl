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

#include <wpl/layout.h>

#include <algorithm>
#include <wpl/helpers.h>

using namespace std;

namespace wpl
{
	namespace
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
	}


	void container::add(control &child)
	{
		_connections.push_back(child.layout_changed += [this] (bool hierarchy_changed) {
			layout_changed(hierarchy_changed);
		});
		layout_changed(true);
	}


	stack::stack(int spacing, bool horizontal)
		: _spacing(spacing), _horizontal(horizontal)
	{	}

	void stack::add(shared_ptr<control> child, int size, int tab_order)
	{
		item i = { child, size, tab_order };
		_children.push_back(i);
		container::add(*child);
	}

	void stack::layout(const placed_view_appender &append_view, const agge::box<int> &box)
	{
		auto b = box;
		const auto zero = b.w - b.w; // '0' in coordinates type
		auto location = zero;
		auto remainder = _horizontal ? box.w : box.h;
		auto relative_base = zero;

		for (auto i = _children.begin(); i != _children.end(); ++i)
			(i->size > zero ? remainder : relative_base) -= i->size;
		remainder -= (static_cast<int>(_children.size()) - 1) * _spacing;
		for (auto i = _children.begin(); i != _children.end(); ++i)
		{
			const auto size = i->size > zero ? i->size : -i->size * remainder / relative_base;

			(_horizontal ? b.w : b.h) = size;
			i->child->layout(offset(append_view, _horizontal ? location : 0, _horizontal ? 0 : location, i->tab_order), b);
			location += size + _spacing;
		}
	}


	padding::padding(shared_ptr<control> inner, int px, int py)
		: _inner(inner), _px(px), _py(py)
	{
		_connection = inner->layout_changed += [this] (bool hierarchy_changed) {
			layout_changed(hierarchy_changed);
		};
	}

	void padding::layout(const placed_view_appender &append_view, const agge::box<int> &box)
	{
		auto b = box;

		b.w -= 2 * _px, b.h -= 2 * _py;
		_inner->layout(offset(append_view, _px, _py, 0), b);
	}


	void overlay::add(shared_ptr<control> child)
	{
		_children.push_back(child);
		container::add(*child);
	}

	void overlay::layout(const placed_view_appender &append_view, const agge::box<int> &box)
	{
		for_each(_children.begin(), _children.end(), [&append_view, &box] (const shared_ptr<control> &ctl) {
			ctl->layout(append_view, box);
		});
	}


	shared_ptr<control> pad_control(shared_ptr<control> inner, int px, int py)
	{	return make_shared<padding>(inner, px, py);	}
}
