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

#include "../layout.h"

#include <numeric>

using namespace std;

namespace wpl
{
	namespace ui
	{
		class vstack::vstack_view : public view
		{
			typedef vector< pair<shared_ptr<view>, vstack::size> > views_container;

			views_container _views;

		public:
			explicit vstack_view(shared_ptr<wpl::ui::widget> widget, const vstack::widgets_container &contents);

			virtual void move(int left, int top, int width, int height);
		};

		vstack::vstack_view::vstack_view(shared_ptr<wpl::ui::widget> widget, const vstack::widgets_container &contents)
			: view(widget)
		{
			for (vstack::widgets_container::const_iterator i = contents.begin(); i != contents.end(); ++i)
				_views.push_back(make_pair(i->first->create_view(), i->second));
		}

		void vstack::vstack_view::move(int left, int top, int width, int height)
		{
			struct subtract_remaining
			{
				int operator ()(int acc, const views_container::value_type &entry)
				{	return !entry.second.is_relative ? acc - entry.second.value.absolute : acc; }
			};

			int y = top;
			
			height = accumulate(_views.begin(), _views.end(), height, subtract_remaining());
	
			for (views_container::const_iterator i = _views.begin(); i != _views.end(); ++i)
				if (!i->second.is_relative)
				{
					i->first->move(left, y, width, i->second.value.absolute);
					y += i->second.value.absolute;
				}
				else
				{
					int h = static_cast<int>(i->second.value.relative * height);

					i->first->move(left, y, width, h);
					y += h;
				}
		}

		void vstack::add(shared_ptr<widget> widget, unsigned int height)
		{
			_contents.push_back(make_pair(widget, size(height)));
		}

		void vstack::add_proportional(shared_ptr<widget> widget, double height_part)
		{
			_contents.push_back(make_pair(widget, size(height_part)));
		}

		shared_ptr<view> vstack::create_view()
		{
			return shared_ptr<view>(new vstack_view(shared_from_this(), _contents));
		}
	}
}
