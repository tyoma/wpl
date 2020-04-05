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

#pragma once

#include <wpl/ui/combobox.h>

#include "native_view.h"

namespace wpl
{
	namespace ui
	{
		namespace win32
		{
			class combobox_impl : public native_view<combobox>
			{
			public:
				virtual void set_model(const std::shared_ptr<list_model> &model);
				virtual void select(index_type index);

			private:
				virtual HWND materialize(HWND hparent);
				virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
					const window::original_handler_t &handler);

				void on_invalidated(const list_model *model);
				void update(HWND hcombobox, const list_model *model) const;
				static void update_selection(HWND hcombobox, std::shared_ptr<const trackable> &selected_item);

			private:
				mutable std::wstring _text_buffer;
				std::shared_ptr<list_model> _model;
				std::shared_ptr<void> _invalidated_connection;
				std::shared_ptr<const trackable> _selected_item;
			};
		}
	}
}
