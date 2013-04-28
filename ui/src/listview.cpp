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

#include "../win32/controls.h"
#include "../win32/window.h"
#include "../win32/native_view.h"

#include <commctrl.h>
#include <olectl.h>
#include <atlstr.h>
#include <algorithm>
#include <iterator>

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
				typedef pair< index_type /*last_index*/, shared_ptr<const trackable> > tracked_item;
				typedef vector<tracked_item> selection_trackers;

				bool _avoid_notifications;
				wstring _text_buffer;
				shared_ptr<model> _model;
				shared_ptr<window> _listview;
				vector<sort_direction> _default_sorts;
				shared_ptr<destructible> _advisory, _invalidated_connection;
				int _sort_column;
				bool _sort_ascending;
				shared_ptr<const trackable> _focused_item;
				selection_trackers _selected_items;
				tracked_item _visible_item;

				//	widget interface
				virtual shared_ptr<view> create_view(const native_root &r);

				// listview interface
				virtual void set_model(shared_ptr<model> model);

				virtual void add_column(const wstring &caption, sort_direction default_sort_direction);
				virtual void adjust_column_widths();

				virtual void select(index_type item, bool reset_previous);
				virtual void clear_selection();

				virtual void ensure_visible(index_type item);

				LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);

				void invalidate_view(index_type new_count);
				void update_focus();
				void update_selection();
				void ensure_tracked_visibility();
				bool is_item_visible(index_type item) const throw();
				void set_column_direction(index_type column, sort_direction direction) throw();

			public:
				listview_impl(HWND hwnd);
			};


			listview_impl::listview_impl(HWND hwnd)
				: _avoid_notifications(false), _listview(window::attach(hwnd)), _sort_column(-1)
			{	_advisory = _listview->advise(bind(&listview_impl::wndproc, this, _1, _2, _3, _4));	}

			shared_ptr<view> listview_impl::create_view(const native_root &r)
			{	return shared_ptr<view>(new native_view(shared_from_this(), _listview->hwnd(), r));	}

			void listview_impl::set_model(shared_ptr<model> model)
			{
				if (model && _sort_column != -1)
					model->set_order(_sort_column, _sort_ascending);
				_invalidated_connection = model ?
					model->invalidated += bind(&listview_impl::invalidate_view, this, _1) : slot_connection();
				invalidate_view(model ? model->get_count() : 0);
				_focused_item.reset();
				_selected_items.clear();
				_visible_item.second.reset();
				_model = model;
			}

			void listview_impl::add_column(const wstring &caption, sort_direction default_sort_direction)
			{
				int index = static_cast<int>(_default_sorts.size());
				CString tchar_buffer(caption.c_str());
				LVCOLUMN column = {
					/* mask = */ LVCF_SUBITEM | LVCF_TEXT,
					/* fmt = */ 0,
					/* cx = */ 0,
					(LPTSTR)(LPCTSTR)tchar_buffer,
					/* cchTextMax = */ 0,
					/* iSubItem = */ index,
				};

				ListView_InsertColumn(_listview->hwnd(), index, &column);
				_default_sorts.push_back(default_sort_direction);
			}

			void listview_impl::adjust_column_widths()
			{
				for (int i = 0; i != static_cast<int>(_default_sorts.size()); ++i)
					ListView_SetColumnWidth(_listview->hwnd(), i, LVSCW_AUTOSIZE_USEHEADER);
			}

			void listview_impl::select(index_type item, bool reset_previous)
			{
				if (reset_previous)
					clear_selection();
				ListView_SetItemState(_listview->hwnd(), item, LVIS_SELECTED, LVIS_SELECTED);
			}

			void listview_impl::clear_selection()
			{	ListView_SetItemState(_listview->hwnd(), -1, 0, LVIS_SELECTED);	}

			void listview_impl::ensure_visible(index_type item)
			{
				_visible_item = make_pair(item, _model->track(item));
				ListView_EnsureVisible(_listview->hwnd(), item, FALSE);
			}

			LRESULT listview_impl::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
			{
				if (OCM_NOTIFY != message)
					return previous(message, wparam, lparam);
				if (_model && !_avoid_notifications)
				{
					UINT code = reinterpret_cast<const NMHDR *>(lparam)->code;
					const NMLISTVIEW *pnmlv = reinterpret_cast<const NMLISTVIEW *>(lparam);

					if (LVN_ITEMCHANGED == code)
					{
						if ((pnmlv->uOldState & LVIS_FOCUSED) != (pnmlv->uNewState & LVIS_FOCUSED))
							_focused_item = pnmlv->uNewState & LVIS_FOCUSED ? _model->track(pnmlv->iItem) : shared_ptr<const trackable>();
						if ((pnmlv->uOldState & LVIS_SELECTED) != (pnmlv->uNewState & LVIS_SELECTED))
							if (pnmlv->uNewState & LVIS_SELECTED)
								_selected_items.push_back(make_pair(pnmlv->iItem, _model->track(pnmlv->iItem)));
							else if (-1 != pnmlv->iItem)
							{
								for (selection_trackers::iterator i = _selected_items.begin(); i != _selected_items.end(); ++i)
									if (static_cast<index_type>(pnmlv->iItem) == i->first)
									{
										_selected_items.erase(i);
										break;
									}
							}
							else
								_selected_items.clear();
					}
					
					if (LVN_ITEMCHANGED == code && (pnmlv->uOldState & LVIS_SELECTED) != (pnmlv->uNewState & LVIS_SELECTED))
						selection_changed(pnmlv->iItem, 0 != (pnmlv->uNewState & LVIS_SELECTED));
					else if (LVN_ITEMACTIVATE == code)
						item_activate(reinterpret_cast<const NMITEMACTIVATE *>(lparam)->iItem);
					else if (LVN_GETDISPINFOA == code)
					{
						const NMLVDISPINFOA *pdi = reinterpret_cast<const NMLVDISPINFOA *>(lparam);

						if (pdi->item.mask & LVIF_TEXT)
						{
							_model->get_text(pdi->item.iItem, pdi->item.iSubItem, _text_buffer);
							wcstombs_s(0, pdi->item.pszText, pdi->item.cchTextMax, _text_buffer.c_str(), _TRUNCATE);
						}
					}
					else if (LVN_GETDISPINFOW == code)
					{
						const NMLVDISPINFOW *pdi = reinterpret_cast<const NMLVDISPINFOW *>(lparam);

						if (pdi->item.mask & LVIF_TEXT)
						{
							_model->get_text(pdi->item.iItem, pdi->item.iSubItem, _text_buffer);
							wcsncpy_s(pdi->item.pszText, pdi->item.cchTextMax, _text_buffer.c_str(), _TRUNCATE);
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
							::InvalidateRect(_listview->hwnd(), NULL, FALSE);
						}
					}
				}
				return 0;
			}

			void listview_impl::invalidate_view(index_type new_count)
			{
				if (new_count != static_cast<index_type>(ListView_GetItemCount(_listview->hwnd())))
					ListView_SetItemCountEx(_listview->hwnd(), new_count, 0);
				else
					::InvalidateRect(_listview->hwnd(), NULL, FALSE);
				_avoid_notifications = true;
				update_focus();
				update_selection();
				ensure_tracked_visibility();
				_avoid_notifications = false;
			}

			void listview_impl::update_focus()
			{
				if (_focused_item)
				{
					index_type new_focus = _focused_item->index();

					ListView_SetItemState(_listview->hwnd(), new_focus, npos != new_focus ? LVIS_FOCUSED : 0, LVIS_FOCUSED);
					if (npos == new_focus)
						_focused_item.reset();
				}
			}

			void listview_impl::update_selection()
			{
				vector<index_type> selection_before, selection_after, d;

				for (int i = -1; i = ListView_GetNextItem(_listview->hwnd(), i, LVNI_ALL | LVNI_SELECTED), i != -1; )
					selection_before.push_back(i);
				for (selection_trackers::iterator i = _selected_items.begin(); i != _selected_items.end(); )
					if (!i->second || (i->first = i->second->index()) != npos)
						selection_after.push_back(i->first), ++i;
					else
						i = _selected_items.erase(i);
				sort(selection_after.begin(), selection_after.end());
				set_difference(selection_after.begin(), selection_after.end(), selection_before.begin(), selection_before.end(), back_inserter(d));
				for (vector<index_type>::const_iterator i = d.begin(); i != d.end(); ++i)
					ListView_SetItemState(_listview->hwnd(), *i, LVIS_SELECTED, LVIS_SELECTED);
				d.clear();
				set_difference(selection_before.begin(), selection_before.end(), selection_after.begin(), selection_after.end(), back_inserter(d));
				for (vector<index_type>::const_iterator i = d.begin(); i != d.end(); ++i)
					ListView_SetItemState(_listview->hwnd(), *i, 0, LVIS_SELECTED);
			}

			void listview_impl::ensure_tracked_visibility()
			{
				if (_visible_item.second)
				{
					index_type new_position = _visible_item.second->index();
					if (new_position != _visible_item.first && is_item_visible(_visible_item.first) && !is_item_visible(new_position))
						ListView_EnsureVisible(_listview->hwnd(), new_position, FALSE);
					_visible_item.first = new_position;
				}
			}

			bool listview_impl::is_item_visible(index_type item) const throw()
			{
				RECT rc1, rc2, rc;

				::GetClientRect(_listview->hwnd(), &rc1);
				ListView_GetItemRect(_listview->hwnd(), item, &rc2, LVIR_BOUNDS);
				if (!::IntersectRect(&rc, &rc1, &rc2))
					return false;
				if (HWND hheader = ListView_GetHeader(_listview->hwnd()))
					if (::IsWindowVisible(hheader))
					{
						rc2 = rc;
						::GetWindowRect(hheader, &rc1);
						::MapWindowPoints(HWND_DESKTOP, _listview->hwnd(), reinterpret_cast<POINT *>(&rc1), 2);
						::UnionRect(&rc, &rc1, &rc2);
						if (::EqualRect(&rc, &rc1))
							return false;
					}
				return true;
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
