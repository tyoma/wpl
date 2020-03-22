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
				HWND hwnd = ::CreateWindow(WC_COMBOBOX, NULL, WS_CHILD | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST,
					0, 0, 100, 100, hparent, NULL, NULL, NULL);
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
					default:
						break;

					case CBN_SELCHANGE:
						const int selected_item = ComboBox_GetCurSel(reinterpret_cast<HWND>(lparam));

						selection_changed(selected_item != CB_ERR ? selected_item : npos());
						return 0;
					}

				default:
					return handler(message, wparam, lparam);
				}
			}

			void combobox_impl::on_invalidated(const list_model *model)
			{	update(get_window(), model);	}

			void combobox_impl::update(HWND hcombobox, const list_model *model) const
			{
				const index_type count = model ? model->get_count() : 0u;

				ComboBox_ResetContent(hcombobox);
				for (index_type i = 0; i != count; ++i)
				{
					model->get_text(i, _text_buffer);
					ComboBox_AddString(hcombobox, _text_buffer.c_str());
				}
			}

			void combobox_impl::update_selection(HWND hcombobox, index_type selected_item)
			{	ComboBox_SetCurSel(hcombobox, selected_item != npos() ? selected_item : -1);	}
		}

		shared_ptr<combobox> create_combobox()
		{	return shared_ptr<combobox>(new win32::combobox_impl);	}
	}
}
