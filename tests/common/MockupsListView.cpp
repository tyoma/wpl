#include "MockupsListView.h"

#include <ut/assert.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			listview_trackable::~listview_trackable()
			{
				if (container && track_result != npos())
					container->erase(track_result);
			}


			headers_model::index_type headers_model::get_count() const throw()
			{	return static_cast<index_type>(columns.size());	}

			void headers_model::get_value(index_type index, short int &width) const
			{	width = columns[index].width;	}

			void headers_model::get_caption(index_type index, agge::richtext_t &column) const
			{	column << columns[index].caption.c_str();	}

			void headers_model::set_width(index_type index, short int width)
			{
				columns[index].width = width, column_update_log.push_back(index);
				if (invalidate_on_update)
					invalidate(index);
			}

			pair<headers_model::index_type, bool> headers_model::get_sort_order() const throw()
			{	return make_pair(sort_column, sort_ascending);	}

			void headers_model::activate_column(index_type column)
			{	column_activation_log.push_back(column);	}

			shared_ptr<headers_model> headers_model::create(const string &caption, short int width)
			{
				column columns[] = {	{	caption, width	}, };

				return create(columns, npos(), false);
			}

			shared_ptr<headers_model> headers_model::create()
			{	return shared_ptr<headers_model>(new headers_model);	}

			void headers_model::set_sort_order(index_type column, bool ascending)
			{
				sort_column = column;
				sort_ascending = ascending;
				sort_order_changed(sort_column, sort_ascending);
			}


			listview_model::listview_model(index_type count, index_type columns)
				: columns_count(columns)
			{	items.resize(count, vector<string>(columns));	}

			void listview_model::set_count(index_type new_count)
			{
				items.resize(new_count, vector<string>(columns_count));
				invalidate(static_cast<index_type>(items.size()));
			}

			listview_model::index_type listview_model::get_count() const throw()
			{	return static_cast<index_type>(items.size());	}

			void listview_model::get_text(index_type row, index_type column, agge::richtext_t &text) const
			{
				assert_is_true(row < items.size());
				assert_is_true(text.empty());
				text << items[row][column].c_str();
			}

			void listview_model::precache(index_type from, index_type count)
			{	precached.push_back(make_pair(from, count));	}

			shared_ptr<const trackable> listview_model::track(index_type row) const
			{
				tracking_requested.push_back(row);

				map< index_type, shared_ptr<const trackable> >::const_iterator i = trackables.find(row);

				return i != trackables.end() ? i->second : shared_ptr<const trackable>();
			}


			autotrackable_table_model::autotrackable_table_model(index_type count, index_type columns)
				: listview_model(count, columns), auto_trackables(new trackables_map)
			{	}

			void autotrackable_table_model::move_tracking(index_type position, index_type new_position)
			{
				shared_ptr<listview_trackable> t((*auto_trackables)[position]);

				assert_equal(auto_trackables->end(), auto_trackables->find(new_position));
				auto_trackables->erase(position);
				t->track_result = new_position;
				(*auto_trackables)[new_position] = t;
			}

			vector<autotrackable_table_model::index_type> autotrackable_table_model::get_trackables() const
			{
				vector<autotrackable_table_model::index_type> result;

				for (auto i = auto_trackables->begin(); i != auto_trackables->end(); ++i)
					result.push_back(i->first);
				return result;
			}

			shared_ptr<const trackable> autotrackable_table_model::track(index_type row) const
			{
				pair<trackables_map::iterator, bool> i = auto_trackables->insert(make_pair(row, shared_ptr<listview_trackable>()));

				if (i.second)
				{
					shared_ptr<listview_trackable> t(new listview_trackable);

					t->track_result = row;
					t->container = auto_trackables;
					i.first->second = t;
					return t;
				}
				else
				{
					return i.first->second.lock();
				}
			}
		}
	}
}
