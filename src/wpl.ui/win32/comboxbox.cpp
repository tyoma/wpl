//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "combobox.h"

#include <commctrl.h>
#include <olectl.h>
#include <windowsx.h>

using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace ui
	{
		namespace win32
		{
			void combobox_impl::set_model(const shared_ptr<list_model> &model)
			{
				_model = model;
				_invalidated_connection = model
					? model->invalidated += bind(&combobox_impl::on_invalidated, this, model.get())
					: shared_ptr<void>();
				if (const HWND hcombobox = get_window())
					update(hcombobox, model.get());
			}

			void combobox_impl::select(index_type index)
			{
				_selected_item = index;
				if (const HWND hcombobox = get_window())
					update_selection(hcombobox, index);
			}

			HWND combobox_impl::materialize(HWND hparent)
			{
				HWND hwnd = ::CreateWindow(WC_COMBOBOXEX, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 0, 0, 100, 100,
					hparent, NULL, NULL, NULL);

				if (_model)
				{
					update(hwnd, _model.get());
					update_selection(hwnd, _selected_item);
				}
				return hwnd;
			}

			LRESULT combobox_impl::on_message(UINT message, WPARAM wparam, LPARAM lparam,
				const window::original_handler_t &handler)
			{
				switch (message)
				{
				case OCM_COMMAND:
					switch (HIWORD(wparam))
					{
					case CBN_SELCHANGE:
						const int selected_item = ComboBox_GetCurSel(reinterpret_cast<HWND>(lparam));

						_selected_item = selected_item != CB_ERR ? selected_item : npos();
						selection_changed(_selected_item);
						return 0;
					}
					break;

				case OCM_NOTIFY:
					switch (reinterpret_cast<const NMHDR *>(lparam)->code)
					{
					case CBEN_GETDISPINFO:
						NMCOMBOBOXEX *cb_item = reinterpret_cast<NMCOMBOBOXEX *>(lparam);

						_model->get_text(cb_item->ceItem.iItem, _text_buffer);
						wcsncpy(cb_item->ceItem.pszText, _text_buffer.c_str(), cb_item->ceItem.cchTextMax - 1);
						cb_item->ceItem.pszText[cb_item->ceItem.cchTextMax - 1] = 0;
						cb_item->ceItem.mask = CBEIF_TEXT;
						return 0;
					}
					break;
				}
				return handler(message, wparam, lparam);
			}

			void combobox_impl::on_invalidated(const list_model *model)
			{	update(get_window(), model);	}

			void combobox_impl::update(HWND hcombobox, const list_model *model) const
			{
				const index_type count = model ? model->get_count() : 0u;
				const index_type previous_count = ComboBox_GetCount(hcombobox);
				const COMBOBOXEXITEM new_item = { CBEIF_TEXT, -1, LPSTR_TEXTCALLBACK, };
				const HWND hchild = reinterpret_cast<HWND>(::SendMessage(hcombobox, CBEM_GETCOMBOCONTROL, 0, 0));
				COMBOBOXINFO cbi = { sizeof(COMBOBOXINFO), };

				for (index_type i = previous_count; i < count; ++i)
					::SendMessage(hcombobox, CBEM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&new_item));
				for (index_type i = count; i < previous_count; ++i)
					::SendMessage(hcombobox, CBEM_DELETEITEM, count, 0);
				::GetComboBoxInfo(hchild, &cbi);
				::InvalidateRect(cbi.hwndList, NULL, FALSE);
				::InvalidateRect(hcombobox, NULL, FALSE);
			}

			void combobox_impl::update_selection(HWND hcombobox, index_type selected_item)
			{	ComboBox_SetCurSel(hcombobox, selected_item != npos() ? selected_item : -1);	}
		}

		shared_ptr<combobox> create_combobox()
		{	return shared_ptr<combobox>(new win32::combobox_impl);	}
	}
}
