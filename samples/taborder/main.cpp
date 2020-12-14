#include <samples/common/application.h>
#include <samples/common/stylesheet.h>
#include <samples/common/timer.h>
#include <wpl/controls.h>
#include <wpl/factory.h>
#include <wpl/form.h>
#include <wpl/layout.h>
#include <wpl/stylesheet_helpers.h>

using namespace agge;
using namespace std;
using namespace wpl;

namespace
{
	class my_columns : public columns_model
	{
	public:
		my_columns()
			: _columns(20, column(L"test", 60)), _sort_order(npos(), false)
		{	}

	private:
		virtual index_type get_count() const throw() override { return static_cast<index_type>(_columns.size()); }
		virtual void get_value(index_type index, short int &w) const override { w = _columns[index].width; }
		virtual void get_column(index_type index, column &column_) const override { column_ = _columns[index]; }
		virtual void update_column(index_type index, short int width) override { _columns[index].width = width, invalidate(); }
		virtual pair<index_type, bool> get_sort_order() const throw() override { return _sort_order; }
		virtual void activate_column(index_type index) override {
			_sort_order.first = index, _sort_order.second = !_sort_order.second;
			sort_order_changed(_sort_order.first, _sort_order.second);
		}

	private:
		vector<column> _columns;
		pair<index_type, bool> _sort_order;
	};

	class my_model : public table_model
	{
	public:
		my_model()
			: _n(0)
		{
			_timer = create_timer(20, [this] (unsigned) {
				_n++;
				invalidate(100);
			});
		}

		virtual index_type get_count() const throw() override
		{	return 100u;	}

		virtual void get_text(index_type row, index_type column, wstring &text) const override
		{
			row++, column++;
			text = to_wstring(static_cast<long double>(row * _n + 13 * column * _n));
		}

		virtual void set_order(index_type /*column*/, bool /*ascending*/) override
		{	}

		virtual shared_ptr<const trackable> track(index_type row) const override
		{
			shared_ptr<my_trackable> t(new my_trackable);

			t->_index = row;
			return t;
		}

	private:
		struct my_trackable : trackable
		{
			virtual index_type index() const override
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
	application app;

	const auto ss = create_sample_stylesheet();
	const auto fct = factory::create_default(ss);
	const view_location l = { 100, 100, 300, 200 };
	const auto f = fct->create_form();
	const auto conn = f->close += [&app] {	app.exit();	};
	shared_ptr<my_columns> cm(new my_columns);
	shared_ptr<my_model> m(new my_model);

	const auto root = make_shared<overlay>();
		root->add(fct->create_control<control>("background"));

		const auto stk = make_shared<stack>(5, false);
		root->add(pad_control(stk, 5, 5));
			const auto lv = fct->create_control<listview>("listview");
			lv->set_columns_model(cm);
			lv->set_model(m);
			stk->add(lv, -100, 1);

			auto btn1 = fct->create_control<button>("button");
			btn1->set_text(L"first");
			stk->add(btn1, 20, 2);

			auto btn2 = fct->create_control<button>("button");
			btn2->set_text(L"second");
			stk->add(btn2, 20, 3);

	f->set_root(root);
	f->set_location(l);
	f->set_visible(true);

	app.run();
}
