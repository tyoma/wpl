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

#include <wpl/ui/win32/controls.h>

#include <wpl/ui/win32/native_view.h>
#include <wpl/ui/win32/window.h>

#include <commctrl.h>
#include <atlstr.h>
#include <algorithm>
#include <iterator>

using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace ui
	{
		namespace
		{
			class listview_impl : public listview, private native_view
			{
				enum sort_direction	{	dir_none, dir_ascending, dir_descending	};
				typedef pair< index_type /*last_index*/, shared_ptr<const trackable> > tracked_item;
				typedef vector<tracked_item> selection_trackers;

				bool _avoid_notifications;
				wstring _text_buffer;
				shared_ptr<columns_model> _columns_model;
				shared_ptr<model> _model;
				shared_ptr<window> _listview;
				shared_ptr<void> _invalidated_connection, _sort_order_changed_connection;
				columns_model::index_type _sort_column;
				shared_ptr<const trackable> _focused_item;
				selection_trackers _selected_items;
				tracked_item _visible_item;

				// listview interface
				virtual void set_columns_model(shared_ptr<columns_model> cm);
				virtual void set_model(shared_ptr<model> model);

				virtual void adjust_column_widths();

				virtual void select(index_type item, bool reset_previous);
				virtual void clear_selection();

				virtual void ensure_visible(index_type item);

				// visual interface
				virtual void resize(unsigned cx, unsigned cy, positioned_native_views &native_views);

				// native_view interface
				virtual HWND get_window() throw();

				LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);

				void update_sort_order(columns_model::index_type new_ordering_column, bool ascending);
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
				: _avoid_notifications(false), _sort_column(columns_model::npos)
			{	_listview = window::attach(hwnd, bind(&listview_impl::wndproc, this, _1, _2, _3, _4));	}

			void listview_impl::set_columns_model(shared_ptr<columns_model> cm)
			{
				CString caption;
				LVCOLUMN lvcolumn = { 0 };
				columns_model::column c;

				for (int i = Header_GetItemCount(ListView_GetHeader(_listview->hwnd())) - 1; i >= 0; --i)
					ListView_DeleteColumn(_listview->hwnd(), i);
				for (columns_model::index_type i = 0, count = cm->get_count(); i != count; ++i)
				{
					cm->get_column(i, c);
					caption = c.caption.c_str();
					lvcolumn.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
					lvcolumn.pszText = (LPTSTR)(LPCTSTR)caption;
					lvcolumn.iSubItem = i;
					lvcolumn.cx = c.width;
					ListView_InsertColumn(_listview->hwnd(), i, &lvcolumn);
				}

				pair<columns_model::index_type, bool> sort_order = cm->get_sort_order();

				if (columns_model::npos != sort_order.first)
				{
					if (_model)
						_model->set_order(sort_order.first, sort_order.second);
					set_column_direction(sort_order.first, sort_order.second ? dir_ascending : dir_descending);
				}
				_sort_column = sort_order.first;

				_sort_order_changed_connection =
					cm->sort_order_changed += bind(&listview_impl::update_sort_order, this, _1, _2);
				_columns_model = cm;
			}

			void listview_impl::set_model(shared_ptr<model> model)
			{
				if (_columns_model && model)
				{
					pair<columns_model::index_type, bool> sort_order = _columns_model->get_sort_order();

					if (columns_model::npos != sort_order.first)
						model->set_order(sort_order.first, sort_order.second);
				}
				_invalidated_connection = model ?
					model->invalidated += bind(&listview_impl::invalidate_view, this, _1) : slot_connection();
				invalidate_view(model ? model->get_count() : 0);
				_focused_item.reset();
				_selected_items.clear();
				_visible_item.second.reset();
				_model = model;
			}

			void listview_impl::adjust_column_widths()
			{
				for (int i = 0, count = Header_GetItemCount(ListView_GetHeader(_listview->hwnd())); i < count; ++i)
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

			void listview_impl::resize(unsigned cx, unsigned cy, positioned_native_views &native_views)
			{
				view_location l = { 0, 0, cx, cy };
				native_views.push_back(positioned_native_view(*this, l));
			}

			HWND listview_impl::get_window() throw()
			{	return _listview->hwnd();	}

			LRESULT listview_impl::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
			{
				if (OCM_NOTIFY != message && WM_NOTIFY != message)
					return previous(message, wparam, lparam);
				if (WM_NOTIFY == message)
				{
					const NMHEADER *pnmhd = reinterpret_cast<const NMHEADER *>(lparam);

					if (_columns_model)
						if (HDN_ITEMCHANGED == pnmhd->hdr.code && 0 != (HDI_WIDTH & pnmhd->pitem->mask))
						{
							_columns_model->update_column(static_cast<columns_model::index_type>(pnmhd->iItem),
								static_cast<short>(pnmhd->pitem->cxy));
						}
					return previous(message, wparam, lparam);
				}
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
						_columns_model->activate_column( static_cast<columns_model::index_type>(reinterpret_cast<const NMLISTVIEW *>(lparam)->iSubItem));
				}
				return 0;
			}

			void listview_impl::update_sort_order(columns_model::index_type new_ordering_column, bool ascending)
			{
				if (_model)
					_model->set_order(new_ordering_column, ascending);
				if (columns_model::npos != _sort_column)
					set_column_direction(_sort_column, dir_none);
				set_column_direction(new_ordering_column, ascending ? dir_ascending : dir_descending);
				_sort_column = new_ordering_column;
				ListView_RedrawItems(_listview->hwnd(), 0, ListView_GetItemCount(_listview->hwnd()));
			}

			void listview_impl::invalidate_view(index_type new_count)
			{
				if (new_count != static_cast<index_type>(ListView_GetItemCount(_listview->hwnd())))
					ListView_SetItemCountEx(_listview->hwnd(), new_count, 0);
				else
					ListView_RedrawItems(_listview->hwnd(), 0, new_count);
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

		shared_ptr<listview> wrap_listview(HWND hwnd)
		{
			return shared_ptr<listview>(new listview_impl(hwnd));
		}
	}
}
