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

#include "widget.h"

namespace wpl
{
	namespace ui
	{
		class vstack : public widget
		{
			class vstack_view;
			struct size_;
			typedef std::pair<std::shared_ptr<widget>, size_> entry;
			typedef std::vector<entry> widgets_container_;

			widgets_container_ _contents;

		public:
			typedef size_ size;
			typedef widgets_container_ widgets_container;

		public:
			void add(std::shared_ptr<widget> widget, unsigned int height);
			void add_proportional(std::shared_ptr<widget> widget, double height_part);

			virtual std::shared_ptr<view> create_view();
		};

		struct vstack::size_
		{
			explicit size_(unsigned int absolute);
			explicit size_(double relative);

			union variant
			{
				unsigned int absolute;
				double relative;
			} value;
			bool is_relative;
		};



		inline vstack::size_::size_(unsigned int absolute)
			: is_relative(false)
		{	value.absolute = absolute;	}

		inline vstack::size_::size_(double relative)
			: is_relative(true)
		{	value.relative = relative;	}
	}
}
