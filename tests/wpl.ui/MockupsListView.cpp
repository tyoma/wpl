#include "MockupsListView.h"

#include <ut/assert.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			listview_columns_model::index_type listview_columns_model::get_count() const throw()
			{	return static_cast<index_type>(columns.size());	}

			void listview_columns_model::get_value(index_type index, short int &width) const
			{	width = columns[index].width;	}

			void listview_columns_model::get_column(index_type index, column &column) const
			{	column = columns[index];	}

			void listview_columns_model::update_column(index_type index, short int width)
			{	columns[index].width = width;	}

			pair<listview_columns_model::index_type, bool> listview_columns_model::get_sort_order() const throw()
			{	return make_pair(sort_column, sort_ascending);	}

			void listview_columns_model::activate_column(index_type column)
			{	column_activation_log.push_back(column);	}

			shared_ptr<listview_columns_model> listview_columns_model::create(const wstring &caption, short int width)
			{
				column columns[] = { column(caption, width), };

				return create(columns, npos(), false);
			}

			void listview_columns_model::set_sort_order(index_type column, bool ascending)
			{
				sort_column = column;
				sort_ascending = ascending;
				sort_order_changed(sort_column, sort_ascending);
			}


			listview_model::index_type listview_model::get_count() const throw()
			{	return static_cast<index_type>(items.size());	}

			void listview_model::get_text(index_type row, index_type column, wstring &text) const
			{
				assert_is_true(row < items.size());
				text = items[row][column];
			}

			void listview_model::set_order(index_type column, bool ascending)
			{	ordering.push_back(make_pair(column, ascending));	}

			shared_ptr<const trackable> listview_model::track(index_type row) const
			{
				tracking_requested.push_back(row);

				map< index_type, shared_ptr<const trackable> >::const_iterator i = trackables.find(row);

				return i != trackables.end() ? i->second : shared_ptr<const trackable>();
			}

			listview_model::listview_model(index_type count, index_type columns)
				: columns_count(columns)
			{	items.resize(count, vector<wstring>(columns));	}

			void listview_model::set_count(index_type new_count)
			{
				items.resize(new_count, vector<wstring>(columns_count));
				invalidated(static_cast<index_type>(items.size()));
			}
		}
	}
}
