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

#pragma once

#include "../controls.h"
#include "native_view.h"
#include "utf8.h"

#include <agge.text/richtext.h>

namespace wpl
{
	namespace win32
	{
		class listview : public wpl::listview, public native_view, index_traits
		{
		public:
			listview();

		private:
			enum sort_direction	{	dir_none, dir_ascending, dir_descending	};
			typedef std::pair< index_type /*last_index*/, std::shared_ptr<const trackable> > tracked_item;
			typedef std::vector<tracked_item> selection_trackers;

		private:
			// control methods
			virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

			// listview methods
			virtual void set_columns_model(std::shared_ptr<headers_model> cm) override;
			virtual void set_selection_model(std::shared_ptr<dynamic_set_model> model) override;
			virtual void set_model(std::shared_ptr<richtext_table_model> model) override;

			virtual void focus(index_type item) override;

			// native_view methods
			virtual HWND materialize(HWND hparent_for) override;
			virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
				const window::original_handler_t &previous) override;

			void setup_columns(HWND hlistview, const headers_model &cm);
			static void setup_data(HWND hlistview, index_type item_count);
			static void setup_selection(HWND hlistview, const selection_trackers &selection);
			void update_sort_order(headers_model::index_type new_ordering_column, bool ascending);
			void invalidate_view();
			void update_focus();
			void update_selection();
			void ensure_tracked_visibility();
			bool is_item_visible(index_type item) const throw();
			static void set_column_direction(HWND hlistview, headers_model::index_type column,
				sort_direction direction) throw();

			// TEMPORARY
			/*virtual*/ void select(index_type item, bool reset_previous) /*override*/;
			signal<void (table_model_base::index_type /*item*/, bool /*became selected*/)> selection_changed;

		private:
			bool _avoid_notifications;
			agge::richtext_t _text_buffer;
			utf_converter _converter;
			std::shared_ptr<headers_model> _columns_model;
			std::shared_ptr<richtext_table_model> _model;
			std::shared_ptr<void> _invalidated_connection, _sort_order_changed_connection;
			headers_model::index_type _sort_column;
			std::shared_ptr<const trackable> _focused_item;
			selection_trackers _selected_items;
			tracked_item _visible_item;
		};
	}
}
