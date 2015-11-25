//	Copyright (c) 2011-2015 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "native_container.h"

#include "factory.h"

#include <algorithm>
#include <iterator>
#include <windows.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		void native_container::reposition(void *&hdwp, const layout_manager::position &new_position)
		{
			struct widget_native_to_widget_position
			{
				layout_manager::widget_position operator ()(const child &from) const
				{	return make_pair(from.widget, layout_manager::position());	}
			};

			vector<layout_manager::widget_position> widgets;

			std::transform(_children.begin(), _children.end(), back_inserter(widgets), widget_native_to_widget_position());

			layout->layout(new_position.width, new_position.height, widgets.empty() ? 0 : &widgets[0], widgets.size());

			vector<layout_manager::widget_position>::const_iterator i;
			children_container::const_iterator j;

			for (i = widgets.begin(), j = _children.begin(); i != widgets.end() && j != _children.end(); ++i, ++j)
			{
				const layout_manager::position p = {
					i->second.left + new_position.left, i->second.top + new_position.top,
					i->second.width, i->second.height
				};

				if (j->container)
					j->container->reposition(hdwp, p);
				else
					hdwp = ::DeferWindowPos(hdwp, j->hwnd, NULL, p.left, p.top, p.width, p.height,
						SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
			}
		}

		shared_ptr<widget> native_container::create_widget(const wstring &type, const wstring &id)
		{
			widget_native wn = create(_parent, type, id);
			child c = { wn.first, wn.second };

			_children.push_back(c);
			return wn.first;
		}

		shared_ptr<container> native_container::create_container(const wstring &/*id*/)
		{
			shared_ptr<native_container> nc(new native_container(_parent));
			child c = { nc, NULL, nc };

			_children.push_back(c);
			return nc;
		}		
	}
}
