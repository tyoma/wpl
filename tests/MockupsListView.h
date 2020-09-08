#pragma once

#include <map>
#include <wpl/models.h>

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			class autotrackable_table_model;
			class columns_model;
			class listview_model;
			class listview_trackable;

			typedef std::shared_ptr<autotrackable_table_model> autotrackable_table_model_ptr;
			typedef std::shared_ptr<listview_model> model_ptr;
			typedef std::shared_ptr<columns_model> columns_model_ptr;
			typedef std::shared_ptr<listview_trackable> trackable_ptr;
			typedef std::map< trackable::index_type, std::weak_ptr<listview_trackable> > trackables_map;

			class listview_trackable : public trackable
			{
			public:
				~listview_trackable();

				template <typename Map>
				static std::shared_ptr<listview_trackable> add(Map &m, index_type index);

			private:
				virtual index_type index() const;

			public:
				index_type track_result;
				std::shared_ptr<trackables_map> container;
			};

			class columns_model : public wpl::columns_model
			{
			public:
				template<size_t n>
				static std::shared_ptr<columns_model> create(const column (&columns)[n], index_type sort_column,
					bool ascending)
				{	return std::shared_ptr<columns_model>(new columns_model(columns, sort_column, ascending));	}

				static std::shared_ptr<columns_model> create(const std::wstring &caption, short int width = 0);
				void set_sort_order(index_type column, bool ascending);

			public:
				std::vector<column> columns;
				std::vector<index_type> column_activation_log;
				index_type sort_column;
				bool sort_ascending;

			private:
				template<size_t n>
				columns_model(const column (&columns_)[n], index_type sort_column_, bool ascending_)
					: columns(columns_, columns_ + n), sort_column(sort_column_), sort_ascending(ascending_)
				{	}

				virtual index_type get_count() const throw();
				virtual void get_value(index_type index, short int &width) const;
				virtual void get_column(index_type index, column &column) const;
				virtual void update_column(index_type index, short int width);
				virtual std::pair<index_type, bool> get_sort_order() const throw();
				virtual void activate_column(index_type column);
			};

			class listview_model : public table_model
			{
			public:
				listview_model(index_type count, index_type columns = 0);
				void set_count(index_type new_count);

			public:
				index_type columns_count;
				std::vector< std::vector<std::wstring> > items;
				std::map< index_type, std::shared_ptr<const trackable> > trackables;
				std::vector< std::pair<index_type, bool> > ordering;
				mutable std::vector<index_type> tracking_requested;

			private:
				virtual index_type get_count() const throw();
				virtual void get_text(index_type row, index_type column, std::wstring &text) const;
				virtual void set_order(index_type column, bool ascending);
				virtual std::shared_ptr<const trackable> track(index_type row) const;
			};

			class autotrackable_table_model : public listview_model
			{
			public:
				autotrackable_table_model(index_type count, index_type columns = 0);

				void move_tracking(index_type position, index_type new_position);

			public:
				std::shared_ptr<trackables_map> auto_trackables;

			private:
				virtual std::shared_ptr<const trackable> track(index_type row) const;
			};



			template <typename Map>
			inline std::shared_ptr<listview_trackable> listview_trackable::add(Map &m, index_type index)
			{
				std::shared_ptr<listview_trackable> t(new listview_trackable());

				m[t->track_result = index] = t;
				return t;
			}

			inline listview_trackable::index_type listview_trackable::index() const
			{	return track_result;	}
		}
	}
}
