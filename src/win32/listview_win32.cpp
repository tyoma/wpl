//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <wpl/win32/listview.h>

#include <agge.text/utf8.h>
#include <agge.text/richtext.h>
#include <algorithm>
#include <commctrl.h>
#include <iterator>
#include <tchar.h>
#include <olectl.h>

using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace win32
	{
		namespace
		{
			enum {
				style = LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | WS_BORDER,
				listview_style = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER,
			};

			void convert_cp(wstring &to, const agge::richtext_t &from)
			{
				to.clear();
				for (auto i = from.ranges_begin(); i != from.ranges_end(); ++i)
					to.insert(to.end(), i->begin(), i->end());
			}
		}



		listview::listview()
			: native_view("text.listview")
		{
			_avoid_notifications = false,
			_sort_column = columns_model::npos();
		}

		void listview::layout(const placed_view_appender &append_view, const agge::box<int> &box)
		{	native_view::layout(append_view, box);	}

		void listview::set_columns_model(shared_ptr<columns_model> cm)
		{
			setup_columns(get_window(), *cm);

			pair<columns_model::index_type, bool> sort_order = cm->get_sort_order();

			if (columns_model::npos() != sort_order.first)
			{
				if (_model)
					_model->set_order(sort_order.first, sort_order.second);
				set_column_direction(get_window(), sort_order.first,
					sort_order.second ? dir_ascending : dir_descending);
			}
			_sort_column = sort_order.first;

			_sort_order_changed_connection =
				cm->sort_order_changed += bind(&listview::update_sort_order, this, _1, _2);
			_columns_model = cm;
		}

		void listview::set_model(shared_ptr<table_model> model)
		{
			if (_columns_model && model)
			{
				pair<columns_model::index_type, bool> sort_order = _columns_model->get_sort_order();

				if (columns_model::npos() != sort_order.first)
					model->set_order(sort_order.first, sort_order.second);
			}
			_invalidated_connection = model ?
				model->invalidate += bind(&listview::invalidate_view, this) : slot_connection();
			_focused_item.reset();
			_selected_items.clear();
			_visible_item.second.reset();
			_model = model;
			invalidate_view();
		}

		void listview::adjust_column_widths()
		{
			for (int i = 0, count = Header_GetItemCount(ListView_GetHeader(get_window())); i < count; ++i)
				ListView_SetColumnWidth(get_window(), i, LVSCW_AUTOSIZE_USEHEADER);
		}

		void listview::select(index_type item, bool reset_previous)
		{
			HWND hwnd = get_window();

			if (reset_previous)
			{
				for (int i; i = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED), i != -1; )
					ListView_SetItemState(hwnd, i, 0, LVIS_SELECTED);
			}
			if (npos() != item)
				ListView_SetItemState(get_window(), item, LVIS_SELECTED, LVIS_SELECTED);
		}

		void listview::focus(index_type item)
		{
			_visible_item = make_pair(item, _model->track(item));
			if (npos() != item)
			{
				ListView_SetItemState(get_window(), item, LVIS_FOCUSED, LVIS_FOCUSED);
			}
			else if (_focused_item)
			{
				ListView_SetItemState(get_window(), _focused_item->index(), 0, LVIS_FOCUSED);
			}
			ListView_EnsureVisible(get_window(), item, FALSE);
		}

		HWND listview::materialize(HWND hparent)
		{
			HWND hwnd = ::CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | style, 0, 0, 1, 1, hparent, NULL, NULL, NULL);

			ListView_SetExtendedListViewStyle(hwnd, listview_style | ListView_GetExtendedListViewStyle(hwnd));
			if (_columns_model)
				setup_columns(hwnd, *_columns_model);
			if (_model)
				setup_data(hwnd, _model->get_count());
			setup_selection(hwnd, _selected_items);
			return hwnd;
		}

		LRESULT listview::on_message(UINT message, WPARAM wparam, LPARAM lparam,
			const window::original_handler_t &previous)
		{
			switch (message)
			{
			case WM_NOTIFY:
				if (const NMHEADER *pnmhd = reinterpret_cast<const NMHEADER *>(lparam))
				{
					if (_columns_model && HDN_ITEMCHANGED == pnmhd->hdr.code && 0 != (HDI_WIDTH & pnmhd->pitem->mask))
					{
						_columns_model->update_column(static_cast<columns_model::index_type>(pnmhd->iItem),
							static_cast<short>(pnmhd->pitem->cxy));
					}
				}
				break;

			case OCM_NOTIFY:
				if (_model && !_avoid_notifications)
				{
					UINT code = reinterpret_cast<const NMHDR *>(lparam)->code;
					const NMLISTVIEW *pnmlv = reinterpret_cast<const NMLISTVIEW *>(lparam);

					switch (code)
					{
					case LVN_ITEMCHANGED:
						if ((pnmlv->uOldState & LVIS_FOCUSED) != (pnmlv->uNewState & LVIS_FOCUSED))
							_focused_item = pnmlv->uNewState & LVIS_FOCUSED ? _model->track(pnmlv->iItem) : shared_ptr<const trackable>();
						if ((pnmlv->uOldState & LVIS_SELECTED) != (pnmlv->uNewState & LVIS_SELECTED))
						{
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
							selection_changed(pnmlv->iItem, 0 != (pnmlv->uNewState & LVIS_SELECTED));
						}
						return 0;

					case LVN_ITEMACTIVATE:
						item_activate(reinterpret_cast<const NMITEMACTIVATE *>(lparam)->iItem);
						return 0;

					case LVN_GETDISPINFOW:
						if (const NMLVDISPINFOW *pdi = reinterpret_cast<const NMLVDISPINFOW *>(lparam))
							if (pdi->item.mask & LVIF_TEXT)
							{
								_model->get_text(pdi->item.iItem, pdi->item.iSubItem, _text_buffer);
								wcsncpy_s(pdi->item.pszText, pdi->item.cchTextMax, _converter(_text_buffer.c_str()), _TRUNCATE);
							}
						return 0;

					case LVN_COLUMNCLICK:
						_columns_model->activate_column( static_cast<columns_model::index_type>(reinterpret_cast<const NMLISTVIEW *>(lparam)->iSubItem));
						return 0;
					}
				}
			}
			return previous(message, wparam, lparam);
		}

		void listview::setup_columns(HWND hlistview, const columns_model &cm)
		{
			short width;
			agge::richtext_t caption((agge::font_style_annotation()));
			wstring caption_plain;
			LVCOLUMNW lvcolumn = { };
			pair<columns_model::index_type, bool> sort_order = cm.get_sort_order();

			for (int i = Header_GetItemCount(ListView_GetHeader(hlistview)) - 1; i >= 0; --i)
				ListView_DeleteColumn(hlistview, i);
			for (columns_model::index_type i = 0, count = cm.get_count(); i != count; ++i)
			{
				caption.clear();
				cm.get_value(i, width);
				cm.get_caption(i, caption);
				convert_cp(caption_plain, caption);
				lvcolumn.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
				lvcolumn.pszText = (LPWSTR)caption_plain.c_str();
				lvcolumn.iSubItem = i;
				lvcolumn.cx = width;
				::SendMessageW(hlistview, LVM_INSERTCOLUMNW, i, (LPARAM)&lvcolumn);
			}
			set_column_direction(hlistview, sort_order.first, sort_order.second ? dir_ascending : dir_descending);
		}

		void listview::setup_data(HWND hlistview, index_type item_count)
		{
			if (item_count != static_cast<index_type>(ListView_GetItemCount(hlistview)))
				ListView_SetItemCountEx(hlistview, item_count, 0);
			else
				ListView_RedrawItems(hlistview, 0, item_count);
		}

		void listview::setup_selection(HWND hlistview, const selection_trackers &selection)
		{
			for (selection_trackers::const_iterator i = selection.begin(); i != selection.end(); ++i)
				ListView_SetItemState(hlistview, i->first, LVIS_SELECTED, LVIS_SELECTED);
		}

		void listview::update_sort_order(columns_model::index_type new_ordering_column, bool ascending)
		{
			if (_model)
				_model->set_order(new_ordering_column, ascending);
			if (columns_model::npos() != _sort_column)
				set_column_direction(get_window(), _sort_column, dir_none);
			set_column_direction(get_window(), new_ordering_column, ascending ? dir_ascending : dir_descending);
			_sort_column = new_ordering_column;
			ListView_RedrawItems(get_window(), 0, ListView_GetItemCount(get_window()));
		}

		void listview::invalidate_view()
		{
			setup_data(get_window(), _model ? _model->get_count() : 0);
			_avoid_notifications = true;
			update_focus();
			update_selection();
			ensure_tracked_visibility();
			_avoid_notifications = false;
		}

		void listview::update_focus()
		{
			if (_focused_item)
			{
				index_type new_focus = _focused_item->index();

				ListView_SetItemState(get_window(), new_focus, npos() != new_focus ? LVIS_FOCUSED : 0, LVIS_FOCUSED);
				if (npos() == new_focus)
					_focused_item.reset();
			}
		}

		void listview::update_selection()
		{
			vector<index_type> selection_before, selection_after, d;

			if (get_window())
				for (int i = -1; i = ListView_GetNextItem(get_window(), i, LVNI_ALL | LVNI_SELECTED), i != -1; )
					selection_before.push_back(i);
			for (selection_trackers::iterator i = _selected_items.begin(); i != _selected_items.end(); )
				if (!i->second || (i->first = i->second->index()) != npos())
					selection_after.push_back(i->first), ++i;
				else
					i = _selected_items.erase(i);
			sort(selection_after.begin(), selection_after.end());
			set_difference(selection_after.begin(), selection_after.end(), selection_before.begin(), selection_before.end(), back_inserter(d));
			for (vector<index_type>::const_iterator i = d.begin(); i != d.end(); ++i)
				ListView_SetItemState(get_window(), *i, LVIS_SELECTED, LVIS_SELECTED);
			d.clear();
			set_difference(selection_before.begin(), selection_before.end(), selection_after.begin(), selection_after.end(), back_inserter(d));
			for (vector<index_type>::const_iterator i = d.begin(); i != d.end(); ++i)
				ListView_SetItemState(get_window(), *i, 0, LVIS_SELECTED);
		}

		void listview::ensure_tracked_visibility()
		{
			if (_visible_item.second)
			{
				index_type new_position = _visible_item.second->index();
				if (new_position != _visible_item.first && is_item_visible(_visible_item.first) && !is_item_visible(new_position))
					ListView_EnsureVisible(get_window(), new_position, FALSE);
				_visible_item.first = new_position;
			}
		}

		bool listview::is_item_visible(index_type item) const throw()
		{
			RECT rc1, rc2, rc;

			::GetClientRect(get_window(), &rc1);
			ListView_GetItemRect(get_window(), item, &rc2, LVIR_BOUNDS);
			if (!::IntersectRect(&rc, &rc1, &rc2))
				return false;
			if (HWND hheader = ListView_GetHeader(get_window()))
				if (::IsWindowVisible(hheader))
				{
					rc2 = rc;
					::GetWindowRect(hheader, &rc1);
					::MapWindowPoints(HWND_DESKTOP, get_window(), reinterpret_cast<POINT *>(&rc1), 2);
					::UnionRect(&rc, &rc1, &rc2);
					if (::EqualRect(&rc, &rc1))
						return false;
				}
			return true;
		}

		void listview::set_column_direction(HWND hlistview, columns_model::index_type column,
			sort_direction dir) throw()
		{
			HDITEM item = { };
			HWND hheader = ListView_GetHeader(hlistview);

			item.mask = HDI_FORMAT;
			item.fmt = HDF_STRING | (dir == dir_ascending ? HDF_SORTUP : dir == dir_descending ? HDF_SORTDOWN : 0);
			Header_SetItem(hheader, column, &item);
		}
	}
}
