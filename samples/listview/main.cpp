#include <crtdbg.h>

#include <samples/common/platform.h>
#include <samples/common/stylesheet.h>
#include <samples/common/timer.h>
#include <wpl/controls.h>
#include <wpl/factory.h>
#include <wpl/form.h>
#include <wpl/layout.h>
#include <wpl/stylesheet_db.h>

using namespace agge;
using namespace std;
using namespace wpl;

namespace
{
	class my_columns : public columns_model
	{
	public:
		my_columns()
			: _columns(20, column(L"test", 60))
		{	}

	private:
		virtual index_type get_count() const throw() { return static_cast<index_type>(_columns.size()); }
		virtual void get_value(index_type index, short int &w) const { w = _columns[index].width; }
		virtual void get_column(index_type index, column &column_) const { column_ = _columns[index]; }
		virtual void update_column(index_type index, short int width) { _columns[index].width = width, invalidated(); }
		virtual pair<index_type, bool> get_sort_order() const throw() { return make_pair(npos(), false); }
		virtual void activate_column(index_type /*column*/) {	}

	private:
		vector<column> _columns;
	};

	class my_model : public table_model
	{
	public:
		my_model()
			: _n(0)
		{
			_timer = create_timer(20, [this] (unsigned) {
				_n++;
				invalidated(100);
			});
		}

		virtual index_type get_count() const throw()
		{	return 100u;	}

		virtual void get_text(index_type row, index_type column, wstring &text) const
		{
			row++, column++;
			text = to_wstring(long double(row * _n + 13 * column * _n));
		}

		virtual void set_order(index_type /*column*/, bool /*ascending*/)
		{	}

		virtual shared_ptr<const trackable> track(index_type row) const
		{
			shared_ptr<my_trackable> t(new my_trackable);

			t->_index = row;
			return t;
		}

	private:
		struct my_trackable : trackable
		{
			virtual index_type index() const
			{	return _index;	}

			index_type _index;
		};

	private:
		double _n;
		shared_ptr<void> _timer;
	};
}

int main()
{
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	const auto ss = create_sample_stylesheet();
	const auto fct = factory::create_default(ss);
	const view_location l = { 100, 100, 300, 200 };
	const auto f = fct->create_form();
	const auto c = f->close += &exit_message_loop;
	const auto lv = fct->create_control<listview>("listview");
	shared_ptr<my_columns> cm(new my_columns);
	shared_ptr<my_model> m(new my_model);

	lv->set_columns_model(cm);
	lv->set_model(m);

	f->set_root(pad_control(lv, 5, 5));
	f->set_location(l);
	f->set_visible(true);
	lv->select(1, true);
	run_message_loop();
}
