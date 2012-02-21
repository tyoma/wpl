//	Copyright (C) 2011-2012 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../widget.h"

#include "../geometry.h"

using namespace std;

namespace wpl
{
	namespace ui
	{
		void widget::visit(node::visitor &visitor)
		{	visitor.visited(*this);	}

		shared_ptr<view> widget::create_view()
		{	return shared_ptr<view>(new view(shared_from_this()));	}



		void container::visit(node::visitor &visitor)
		{	visitor.visited(*this);	}

		shared_ptr<view> container::add(shared_ptr<widget> widget)
		{
			shared_ptr<view> v(widget->create_view());

			_children.push_back(v);
			return v;
		}

		void container::get_children(children_list &children) const
		{	children.assign(_children.begin(), _children.end());	}



		view::view(shared_ptr<wpl::ui::widget> widget_)
			: _transform(new wpl::ui::transform), widget(widget_)
		{	}

		view::~view()
		{	}

		shared_ptr<const transform> view::transform() const
		{	return _transform;	}

		void view::move(int left, int top, int /*width*/, int /*height*/)
		{	_transform->set_origin(left, top);	}

		void view::visit(visitor &visitor)
		{	visitor.generic_view_visited(*this);	}



		void composition::visit(node::visitor &visitor)
		{	visitor.visited((container &)*this);	}
	}
}
