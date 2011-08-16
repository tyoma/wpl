//	Copyright (C) 2011 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../win32/controls.h"
#include "../win32/window.h"

#include <commctrl.h>

using namespace std;
using namespace std::tr1;
using namespace std::tr1::placeholders;

namespace wpl
{
	namespace ui
	{
		namespace
		{
			class listview_impl : public listview
			{
				shared_ptr<window> _listview;
				shared_ptr<destructible> _invalidated_connection;

				virtual void set_datasource(shared_ptr<datasource> ds);
				virtual void add_column(const wstring &caption, sort_direction default_sort_direction);

				void on_invalidated(index_type new_count);

			public:
				listview_impl(HWND hwnd);
			};


			listview_impl::listview_impl(HWND hwnd)
				: _listview(window::attach(hwnd))
			{	}

			void listview_impl::set_datasource(shared_ptr<datasource> ds)
			{
				ListView_SetItemCountEx(_listview->hwnd(), ds->get_count(), 0);
				_invalidated_connection = ds->invalidated += bind(&listview_impl::on_invalidated, this, _1);
			}

			void listview_impl::add_column(const wstring &caption, sort_direction default_sort_direction)
			{	throw 0;	}

			void listview_impl::on_invalidated(index_type new_count)
			{
				ListView_SetItemCountEx(_listview->hwnd(), new_count, 0);
			}
		}

		shared_ptr<listview> wrap_listview(void *hwnd)
		{
			return shared_ptr<listview>(new listview_impl(reinterpret_cast<HWND>(hwnd)));
		}
	}
}
