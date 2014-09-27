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

#include "../layout.h"

using namespace std;

namespace wpl
{
	namespace ui
	{
		template <int layout_manager::position::*SharedPosition, int layout_manager::position::*SharedSize,
			int layout_manager::position::*CommonPosition, int layout_manager::position::*CommonSize>
		inline void stack<SharedPosition, SharedSize, CommonPosition, CommonSize>::layout(size_t /*shared_size*/,
			size_t common_size, layout_manager::widget_position *widgets, size_t count) const
		{
			vector<int>::const_iterator i;
			int location;

			for (i = _sizes.begin(), location = 0; count; location += *i + _spacing, ++widgets, ++i, --count)
			{
				widgets->second.*SharedPosition = location;
				widgets->second.*CommonPosition = 0;
				widgets->second.*SharedSize = *i;
				widgets->second.*CommonSize = common_size;
			}
		}


		void hstack::layout(size_t width, size_t height, widget_position *widgets, size_t count) const
		{	base::layout(width, height, widgets, count);	}


		void vstack::layout(size_t width, size_t height, widget_position *widgets, size_t count) const
		{	base::layout(height, width, widgets, count);	}
	}
}
