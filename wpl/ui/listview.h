//	Copyright (c) 2011-2019 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "models.h"
#include "view.h"

namespace wpl
{
	namespace ui
	{
		struct listview : view, index_traits
		{
			struct columns_model;

			virtual void set_columns_model(std::shared_ptr<columns_model> cm) = 0;
			virtual void set_model(std::shared_ptr<table_model> ds) = 0;

			virtual void adjust_column_widths() = 0;

			virtual void select(index_type item, bool reset_previous) = 0;
			virtual void clear_selection() = 0;

			virtual void ensure_visible(index_type item) = 0;

			signal<void (index_type /*item*/)> item_activate;
			signal<void (index_type /*item*/, bool /*became selected*/)> selection_changed;
		};

		struct listview::columns_model : destructible
		{
			typedef short int index_type;

			struct column;

			static index_type npos();

			virtual index_type get_count() const throw() = 0;
			virtual void get_column(index_type index, column &column) const = 0;
			virtual void update_column(index_type index, short int width) = 0;
			virtual std::pair<index_type, bool> get_sort_order() const throw() = 0;
			virtual void activate_column(index_type column) = 0;

			signal<void (index_type /*new_ordering_column*/, bool /*ascending*/)> sort_order_changed;
		};

		struct listview::columns_model::column
		{
			column();
			explicit column(const std::wstring &caption, short int width = 0);

			std::wstring caption;
			short int width;
		};



		inline listview::columns_model::index_type listview::columns_model::npos()
		{	return -1;	}


		inline listview::columns_model::column::column()
		{	}

		inline listview::columns_model::column::column(const std::wstring &caption_, short int width_)
			: caption(caption_), width(width_)
		{	}
	}
}
