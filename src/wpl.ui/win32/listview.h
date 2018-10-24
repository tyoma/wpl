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

#include <wpl/ui/listview.h>

#include "native_view.h"

namespace wpl
{
	namespace ui
	{
		namespace win32
		{
			class reflector_host : noncopyable
			{
			public:
				reflector_host();
				~reflector_host();

				HWND get_host_hwnd() throw();

			private:
				class reflector_class;

			private:
				static LRESULT CALLBACK windowproc_proxy(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

			private:
				static reflector_class _class;
				HWND _hwnd;
			};

			class listview : reflector_host, public native_view<wpl::ui::listview>
			{
			public:
				listview();
				listview(HWND hwnd);

			private:
				enum sort_direction	{	dir_none, dir_ascending, dir_descending	};
				typedef std::pair< index_type /*last_index*/, std::shared_ptr<const trackable> > tracked_item;
				typedef std::vector<tracked_item> selection_trackers;

			private:
				void init();

				// listview interface
				virtual void set_columns_model(std::shared_ptr<columns_model> cm);
				virtual void set_model(std::shared_ptr<table_model> model);

				virtual void adjust_column_widths();

				virtual void select(index_type item, bool reset_previous);
				virtual void clear_selection();

				virtual void ensure_visible(index_type item);

				virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
					const window::original_handler_t &previous);

				void update_sort_order(columns_model::index_type new_ordering_column, bool ascending);
				void invalidate_view(index_type new_count);
				void update_focus();
				void update_selection();
				void ensure_tracked_visibility();
				bool is_item_visible(index_type item) const throw();
				void set_column_direction(index_type column, sort_direction direction) throw();

			private:
//				std::auto_ptr<reflector_host> _reflector_parent;
//				HWND _hwnd_listview;
				bool _avoid_notifications;
				std::wstring _text_buffer;
				std::shared_ptr<columns_model> _columns_model;
				std::shared_ptr<table_model> _model;
//				std::shared_ptr<window> _listview;
				std::shared_ptr<void> _invalidated_connection, _sort_order_changed_connection;
				columns_model::index_type _sort_column;
				std::shared_ptr<const trackable> _focused_item;
				selection_trackers _selected_items;
				tracked_item _visible_item;
			};
		}
	}
}
