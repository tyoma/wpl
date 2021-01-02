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


			columns_model::column::column()
				: width(0)
			{	}

			columns_model::column::column(const std::wstring &caption_, short int width_)
				: caption(caption_), width(width_)
			{	}

			columns_model::index_type columns_model::get_count() const throw()
			{	return static_cast<index_type>(columns.size());	}

			void columns_model::get_value(index_type index, short int &width) const
			{	width = columns[index].width;	}

			void columns_model::get_caption(index_type index, agge::richtext_t &column) const
			{	column = columns[index].caption.c_str();	}

			void columns_model::update_column(index_type index, short int width)
			{	columns[index].width = width;	}

			pair<columns_model::index_type, bool> columns_model::get_sort_order() const throw()
			{	return make_pair(sort_column, sort_ascending);	}

			void columns_model::activate_column(index_type column)
			{	column_activation_log.push_back(column);	}

			shared_ptr<columns_model> columns_model::create(const wstring &caption, short int width)
			{
				column columns[] = { column(caption.c_str(), width), };

				return create(columns, npos(), false);
			}

			shared_ptr<columns_model> columns_model::create()
			{	return shared_ptr<columns_model>(new columns_model);	}

			void columns_model::set_sort_order(index_type column, bool ascending)
			{
				sort_column = column;
				sort_ascending = ascending;
				sort_order_changed(sort_column, sort_ascending);
			}


			listview_model::listview_model(index_type count, index_type columns)
				: columns_count(columns)
			{	items.resize(count, vector<wstring>(columns));	}

			void listview_model::set_count(index_type new_count)
			{
				items.resize(new_count, vector<wstring>(columns_count));
				invalidate(static_cast<index_type>(items.size()));
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
