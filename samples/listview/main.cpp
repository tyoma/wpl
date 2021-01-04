#include <samples/common/application.h>
#include <wpl/controls.h>
#include <wpl/factory.h>
#include <wpl/form.h>
#include <wpl/layout.h>
#include <wpl/stylesheet_db.h>

using namespace std;
using namespace wpl;

namespace
{
	agge::richtext_t &operator <<(agge::richtext_t &lhs, const wchar_t *rhs)
	{	return lhs.append(rhs, rhs + wcslen(rhs)), lhs;	}

	class my_columns : public columns_model
	{
	public:
		my_columns()
			: _widths(20, 60), _sort_order(npos(), false)
		{	}

	private:
		virtual index_type get_count() const throw() override { return static_cast<index_type>(_widths.size()); }
		virtual void get_value(index_type index, short int &w) const override { w = _widths[index]; }
		virtual void get_caption(index_type index, agge::richtext_t &caption) const override
		{
			auto a = caption.current_annotation();

			switch (index)
			{
			case 0:
				caption << L"Total\n" << agge::style::height(static_cast<int>(0.8 * a.basic.height)) << L"(inclusive)";
				break;

			case 1:
				caption << L"Total\n" << agge::style::height(static_cast<int>(0.8 * a.basic.height)) << L"(exclusive)";
				break;

			case 2:
				caption << L"Average\n" << agge::style::height(static_cast<int>(0.8 * a.basic.height)) << L"(inclusive)";
				break;

			case 3:
				caption << L"Average\n" << agge::style::height(static_cast<int>(0.8 * a.basic.height)) << L"(exclusive)";
				break;
			}
		}
		virtual void update_column(index_type index, short int width) override { _widths[index] = width, invalidate(); }
		virtual pair<index_type, bool> get_sort_order() const throw() override { return _sort_order; }
		virtual void activate_column(index_type index) override {
			_sort_order.first = index, _sort_order.second = !_sort_order.second;
			sort_order_changed(_sort_order.first, _sort_order.second);
		}

	private:
		vector<short> _widths;
		pair<index_type, bool> _sort_order;
	};

	class my_model : public table_model
	{
	public:
		my_model(const queue &queue_)
			: _queue(queue_), _n(0), _alive(make_shared<bool>(true))
		{	on_timer(_alive);	}

		~my_model()
		{	*_alive = false;	}

		void on_timer(const shared_ptr<bool> &alive)
		{
			if (*alive)
			{
				_n++;
				invalidate(100);
				_queue([this, alive] {	on_timer(alive);	}, 25);
			}
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
		queue _queue;
		double _n;
		shared_ptr<bool> _alive;
	};
}

int main()
{
	application app;
	const auto fct = app.create_default_factory();
	const rect_i l = { 100, 100, 400, 300 };
	const auto f = fct->create_form();
	const auto c = f->close += [&app] {	app.exit();	};
	shared_ptr<my_columns> cm(new my_columns);
	shared_ptr<my_model> m(new my_model(app.get_application_queue()));

	const auto root = make_shared<overlay>();
		root->add(fct->create_control<control>("background"));

		const auto lv = fct->create_control<listview>("listview");
		root->add(pad_control(lv, 5, 5));

		new int;

	lv->set_columns_model(cm);
	lv->set_model(m);

	f->set_root(root);
	f->set_location(l);
	f->set_visible(true);
	lv->select(1, true);

	app.run();
}
