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

#include <wpl/ui/layout.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		template <int container::positioned_view::*SharedPosition, int container::positioned_view::*SharedSize,
			int container::positioned_view::*CommonPosition, int container::positioned_view::*CommonSize>
		inline void stack<SharedPosition, SharedSize, CommonPosition, CommonSize>::layout(unsigned shared_size,
			unsigned common_size, container::positioned_view *widgets, size_t count) const
		{
			vector<int>::const_iterator i;
			int remainder, relative_base, location;
			container::positioned_view *w;
			size_t c;

			for (i = _sizes.begin(), remainder = shared_size, relative_base = 0; count && i != _sizes.end(); ++i)
			{
				remainder -= *i > 0 ? *i : 0;
				relative_base += *i < 0 ? *i : 0;
			}

			remainder -= (static_cast<unsigned>(_sizes.size()) - 1) * _spacing;

			for (i = _sizes.begin(), location = 0, w = widgets, c = count; c; ++w, ++i, --c)
			{
				int size = *i > 0 ? *i : *i * remainder / relative_base;

				w->*SharedPosition = location;
				w->*CommonPosition = 0;
				w->*SharedSize = size;
				w->*CommonSize = common_size;

				location += size + _spacing;
			}
		}


		void hstack::layout(unsigned width, unsigned height, container::positioned_view *widgets, size_t count) const
		{	base::layout(width, height, widgets, count);	}


		void vstack::layout(unsigned width, unsigned height, container::positioned_view *widgets, size_t count) const
		{	base::layout(height, width, widgets, count);	}
	}
}
