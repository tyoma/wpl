//	Copyright (c) 2011-2018 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "container.h"

#include "../base/concepts.h"

namespace wpl
{
	namespace ui
	{
		struct layout_manager
		{
			virtual ~layout_manager() {	}

			virtual void layout(unsigned width, unsigned height, container::positioned_view *views, size_t count) const = 0;
		};


		class stack : public layout_manager
		{
		public:
			explicit stack(int spacing, bool horizontal);

			void add(int size);
			virtual void layout(unsigned width, unsigned height, container::positioned_view *views, size_t count) const;

		private:
			std::vector<int> _sizes;
			int _spacing;
			bool _horizontal;
		};

		class spacer : public layout_manager
		{
		public:
			spacer(int space_x, int space_y);

			virtual void layout(unsigned width, unsigned height, container::positioned_view *views, size_t count) const;

		private:
			int _space_x, _space_y;
		};
	}
}
