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
				int _sort_column;
				bool _sort_ascending;

				virtual void set_model(shared_ptr<model> model);
				virtual void add_column(const wstring &caption, sort_direction default_sort_direction);

				void invalidate_view(index_type new_count) throw();

				LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);

				void set_column_direction(index_type column, sort_direction direction) throw();

			public:
				listview_impl(HWND hwnd);
			};


			listview_impl::listview_impl(HWND hwnd)
				: _listview(window::attach(hwnd)), _sort_column(-1)
			{
				_advisory = _listview->advise(bind(&listview_impl::wndproc, this, _1, _2, _3, _4));
			}

			void listview_impl::set_model(shared_ptr<model> model)
			{
				if (model && _sort_column != -1)
					model->set_order(_sort_column, _sort_ascending);
				_invalidated_connection = model ? model->invalidated += bind(&listview_impl::invalidate_view, this, _1) : 0;
				invalidate_view(model ? model->get_count() : 0);
				_model = model;
			}

			void listview_impl::add_column(const wstring &caption, sort_direction default_sort_direction)
			{
				int index = _default_sorts.size();
				LVCOLUMN column = {
					/* mask = */ LVCF_SUBITEM | LVCF_WIDTH,
					/* fmt = */ 0,
					/* cx = */ -1,
					/* pszText = */ 0,
					/* cchTextMax = */ 0,
					/* iSubItem = */ index,
				};

				ListView_InsertColumn(_listview->hwnd(), index, &column);
				_default_sorts.push_back(default_sort_direction);
			}

			void listview_impl::invalidate_view(index_type new_count) throw()
			{
				if (new_count != static_cast<index_type>(ListView_GetItemCount(_listview->hwnd())))
					ListView_SetItemCountEx(_listview->hwnd(), new_count, 0);
				else
					::InvalidateRect(_listview->hwnd(), NULL, FALSE);
			}

			LRESULT listview_impl::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
			{
				if (OCM_NOTIFY == message)
				{
					UINT code = reinterpret_cast<const NMHDR *>(lparam)->code;
					const NMLISTVIEW *pnmlv = reinterpret_cast<const NMLISTVIEW *>(lparam);

					if (LVN_ITEMCHANGED == code && (pnmlv->uOldState & LVIS_SELECTED) != (pnmlv->uNewState & LVIS_SELECTED))
						selection_changed(pnmlv->iItem, 0 != (pnmlv->uNewState & LVIS_SELECTED));
					else if (LVN_ITEMACTIVATE == code)
						item_activate(reinterpret_cast<const NMITEMACTIVATE *>(lparam)->iItem);
					else if (LVN_GETDISPINFO == code)
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
						int column = pnmlv->iSubItem;
						sort_direction default_sort = _default_sorts[column];

						if (default_sort != dir_none)
						{
							bool sort_ascending = _sort_column != column ? default_sort == dir_ascending : !_sort_ascending;
							
							if (_sort_column != column)
								set_column_direction(_sort_column, dir_none);
							_model->set_order(column, sort_ascending);
							set_column_direction(column, sort_ascending ? dir_ascending : dir_descending);
							_sort_column = column;
							_sort_ascending = sort_ascending;
						}
					}
					return 0;
				}
				else
					return previous(message, wparam, lparam);
			}

			void listview_impl::set_column_direction(index_type column, sort_direction dir) throw()
			{
				HDITEM item = { 0 };
				HWND hheader = ListView_GetHeader(_listview->hwnd());

				item.mask = HDI_FORMAT;
				item.fmt = HDF_STRING | (dir == dir_ascending ? HDF_SORTUP : dir == dir_descending ? HDF_SORTDOWN : 0);
				Header_SetItem(hheader, column, &item);
			}
		}

		shared_ptr<listview> wrap_listview(void *hwnd)
		{
			return shared_ptr<listview>(new listview_impl(reinterpret_cast<HWND>(hwnd)));
		}
	}
}
