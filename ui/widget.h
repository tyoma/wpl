//	Copyright (c) 2011-2014 by Artem A. Gevorkyan (gevorkyan.org)
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

namespace std
{
	using tr1::shared_ptr;
	using tr1::enable_shared_from_this;
}

namespace wpl
{
	namespace ui
	{
		class native_root;
		class transform;
		class view;
		struct widget;

		struct widget : std::enable_shared_from_this<widget>
		{
			virtual ~widget()	{	}
			virtual std::shared_ptr<view> create_view(const native_root &r);
		};

		class view
		{
			std::shared_ptr<wpl::ui::transform> _transform;

			const view &operator =(const view &rhs);

		public:
			struct visitor;

		public:
			explicit view(std::shared_ptr<widget> w);
			virtual ~view();

			std::shared_ptr<const wpl::ui::transform> transform() const;
			virtual void move(int left, int top, int width, int height);
			
			const std::shared_ptr<widget> widget;
		};
	}
}
