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
		struct widget;
		struct native_widget;

		struct widget_visitor
		{
			virtual void generic_widget_visited(widget &w) = 0;
			virtual void native_widget_visited(native_widget &w) = 0;
		};

		struct widget
		{
			virtual ~widget()	{	}
			virtual void visit(widget_visitor &visitor) {	visitor.generic_widget_visited(*this);	}
		};

		struct container
		{
			typedef std::vector< std::shared_ptr<widget> > children_list;
			struct widget_site;

			virtual ~container()	{	}
			virtual std::shared_ptr<widget_site> add(std::shared_ptr<widget> widget) = 0;
			virtual void get_children(children_list &children) const = 0;

			signal<void (unsigned int width, unsigned int height)> resized;
		};
		
		struct container::widget_site
		{
			virtual ~widget_site()	{	}
			virtual void move(int left, int top, int width, int height) = 0;
		};
	}
}
