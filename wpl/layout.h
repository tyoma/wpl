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

#pragma once

#include "concepts.h"
#include "control.h"

#include <vector>

namespace wpl
{
	class stack : public control, noncopyable
	{
	public:
		stack(int spacing, bool horizontal);

		void add(std::shared_ptr<control> child, int size, int tab_order = 0);

		// control methods
		virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

	private:
		struct item
		{
			std::shared_ptr<control> child;
			int size;
			int tab_order;
		};

	private:
		std::vector<item> _children;
		int _spacing;
		bool _horizontal;
	};


	class padding : public control
	{
	public:
		padding(std::shared_ptr<control> inner, int px, int py);

		virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

	private:
		std::shared_ptr<control> _inner;
		int _px, _py;
	};


	class overlay : public control
	{
	public:
		void add(std::shared_ptr<control> child);

		// control methods
		virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

	private:
		std::vector< std::shared_ptr<control> > _children;
	};



	std::shared_ptr<control> pad_control(std::shared_ptr<control> inner, int px, int py);
}
