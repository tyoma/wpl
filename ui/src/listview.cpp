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
#include <atlstr.h>

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
				shared_ptr<model> _model;
				shared_ptr<window> _listview;
				vector<sort_direction> _default_sorts;
				shared_ptr<destructible> _advisory, _invalidated_connection;

				virtual void set_model(shared_ptr<model> model);
				virtual void add_column(const wstring &caption, sort_direction default_sort_direction);

				void on_invalidated(index_type new_count);

				LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);

			public:
				listview_impl(HWND hwnd);
			};


			listview_impl::listview_impl(HWND hwnd)
				: _listview(window::attach(hwnd))
			{
				_advisory = _listview->advise(bind(&listview_impl::wndproc, this, _1, _2, _3, _4));
			}

			void listview_impl::set_model(shared_ptr<model> model)
			{
				ListView_SetItemCountEx(_listview->hwnd(), model->get_count(), 0);
				_invalidated_connection = model->invalidated += bind(&listview_impl::on_invalidated, this, _1);
				_model = model;
			}

			void listview_impl::add_column(const wstring &caption, sort_direction default_sort_direction)
			{
				_default_sorts.push_back(default_sort_direction);
			}

			void listview_impl::on_invalidated(index_type new_count)
			{
				ListView_SetItemCountEx(_listview->hwnd(), new_count, 0);
			}

			LRESULT listview_impl::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
			{
				if (OCM_NOTIFY == message)
				{
					UINT code = reinterpret_cast<const NMHDR *>(lparam)->code;

					if (LVN_GETDISPINFO == code)
					{
						const NMLVDISPINFO *pdi = reinterpret_cast<const NMLVDISPINFO *>(lparam);

						if (pdi->item.mask & LVIF_TEXT)
						{
							wstring b1;
	
							_model->get_text(pdi->item.iItem, pdi->item.iSubItem, b1);
							CString b2(b1.c_str());
							_tcsncpy_s(pdi->item.pszText, pdi->item.cchTextMax, b2, _TRUNCATE);
						}
					}
					else if (LVN_COLUMNCLICK == code)
					{
						int column = reinterpret_cast<const NMLISTVIEW *>(lparam)->iSubItem;
						sort_direction default_sort = _default_sorts[column];

						if (default_sort != dir_none)
						{
							_model->set_order(column, default_sort == dir_ascending);
						}
					}
					return 0;
				}
				else
					return previous(message, wparam, lparam);
			}
		}

		shared_ptr<listview> wrap_listview(void *hwnd)
		{
			return shared_ptr<listview>(new listview_impl(reinterpret_cast<HWND>(hwnd)));
		}
	}
}
