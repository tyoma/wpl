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

#include "control.h"
#include "models.h"
#include "signals.h"

#include <string>

namespace wpl
{
	struct text_container
	{
		enum halign { left, center, right };

		virtual ~text_container() {	}
		virtual void set_text(const std::wstring &text) = 0;
		virtual void set_align(halign value) = 0;
	};

	struct button : control, text_container
	{
		signal<void ()> clicked;
	};

	struct link : control, text_container
	{
		signal<void (size_t item, const std::wstring &link_text)> clicked;
	};

	struct combobox : control
	{
		typedef list_model<std::wstring> model_t;

		virtual void set_model(const std::shared_ptr<model_t> &model) = 0;
		virtual void select(model_t::index_type item) = 0;

		signal<void (model_t::index_type item)> selection_changed;
	};

	struct listview : control
	{
		virtual void set_columns_model(std::shared_ptr<columns_model> model) = 0;
		virtual void set_model(std::shared_ptr<table_model> model) = 0;

		virtual void adjust_column_widths() = 0;

		virtual void select(table_model::index_type item, bool reset_previous) = 0;
		virtual void focus(table_model::index_type item) = 0;

		signal<void (table_model::index_type /*item*/)> item_activate;
		signal<void (table_model::index_type /*item*/, bool /*became selected*/)> selection_changed;
	};
}
