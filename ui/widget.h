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

#pragma once

#include "../base/signals.h"

#include <memory>

namespace std
{
	using tr1::shared_ptr;
}

namespace wpl
{
	namespace ui
	{
		class view;
		struct native_view;
		class transform;

		struct node
		{
			struct visitor;

			virtual void visit(visitor &visitor) = 0;

		protected:
			~node()	{	}	// holding a node via ptr to 'node' is prohibited
		};


		struct widget : node
		{
			virtual ~widget()	{	}
			virtual void visit(visitor &visitor);
			virtual std::shared_ptr<view> create_custom_view();
		};


		class view
		{
			std::shared_ptr<wpl::ui::transform> _transform;

			const view &operator =(const view &rhs);

		public:
			struct visitor;

		public:
			explicit view(std::shared_ptr<widget> widget);
			virtual ~view();

			std::shared_ptr<const wpl::ui::transform> transform() const;
			virtual void move(int left, int top, int width, int height);
			virtual void visit(visitor &visitor);
			
			const std::shared_ptr<widget> widget;
		};


		class container : public node
		{
			typedef std::vector< std::shared_ptr<view> > children_list_;

			children_list_ _children;

		public:
			typedef children_list_ children_list;

		public:
			virtual ~container()	{	}
			virtual void visit(node::visitor &visitor);
			virtual std::shared_ptr<view> add(std::shared_ptr<widget> widget);
			virtual void get_children(children_list &children) const;

			signal<void (unsigned int width, unsigned int height)> resized;
		};


		struct node::visitor
		{
			virtual void visited(widget &w) = 0;
			virtual void visited(container &c) = 0;
		};


		struct view::visitor
		{
			virtual void generic_view_visited(view &v) = 0;
			virtual void native_view_visited(native_view &v) = 0;
		};
	}
}
