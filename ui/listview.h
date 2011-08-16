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

#pragma once

#include "../base/signals.h"

#include <string>

namespace wpl
{
	namespace ui
	{
		struct listview : destructible
		{
			typedef unsigned int index_type;
			struct datasource;
			enum sort_direction	{	dir_none, dir_ascending, dir_descending	};

			virtual void set_datasource(std::shared_ptr<datasource> ds) = 0;
			virtual void add_column(const std::wstring &caption, sort_direction default_sort_direction) = 0;

			signal<void (index_type /*item*/)> item_activate;
			signal<void (index_type /*item*/, bool /*became selected*/)> selection_changed;
		};

		struct listview::datasource : destructible
		{
			typedef listview::index_type index_type;

			virtual index_type get_count() const throw() = 0;
			virtual void get_text(index_type item, index_type subitem, std::wstring &text) const = 0;
			virtual void set_order(index_type column, bool ascending) = 0;
			virtual void precache(index_type from, index_type count) const;

			signal<void (index_type /*new_count*/)> invalidated;
		};


		inline void listview::datasource::precache(index_type /*from*/, index_type /*count*/) const
		{	}
	}
}