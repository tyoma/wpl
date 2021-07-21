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

#include <algorithm>
#include <wpl/helpers.h>

using namespace std;

namespace wpl
{
	void container::remove(const control &child)
	{
		const auto i = find_if(_connections.begin(), _connections.end(),
			[&child] (const pair<const control *, slot_connection> &e) {	return e.first == &child;	});

		if (_connections.end() == i)
			return;
		_connections.erase(i);
		layout_changed(true);
	}

	void container::add(control &child)
	{
		_connections.push_back(make_pair(&child, child.layout_changed += [this] (bool hierarchy_changed) {
			layout_changed(hierarchy_changed);
		}));
		layout_changed(true);
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

	int padding::min_height(int for_width) const
	{	return _inner->min_height(maximum_size == for_width ? maximum_size : for_width - 2 * _px) + 2 * _py;	}

	int padding::min_width(int for_height) const
	{	return _inner->min_width(maximum_size == for_height ? maximum_size : for_height - 2 * _py) + 2 * _px;	}



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

	int overlay::min_height(int for_width) const
	{
		auto v = 0;

		for (auto i = _children.begin(); i != _children.end(); ++i)
			v = (max)(v, (*i)->min_height(for_width));
		return v;
	}

	int overlay::min_width(int for_height) const
	{
		auto v = 0;

		for (auto i = _children.begin(); i != _children.end(); ++i)
			v = (max)(v, (*i)->min_width(for_height));
		return v;
	}


	shared_ptr<control> pad_control(shared_ptr<control> inner, int px, int py)
	{	return make_shared<padding>(inner, px, py);	}
}
