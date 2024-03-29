#include <samples/common/application.h>
#include <set>
#include <wpl/controls.h>
#include <wpl/factory.h>
#include <wpl/form.h>
#include <wpl/layout.h>
#include <wpl/stylesheet_db.h>

using namespace std;
using namespace wpl;

namespace
{
	agge::richtext_t &operator <<(agge::richtext_t &lhs, const char *rhs)
	{	return lhs.append(rhs, rhs + strlen(rhs)), lhs;	}

	class my_selection : public dynamic_set_model
	{
		virtual void clear() throw() override {	selection.clear(), invalidate(npos());	}
		virtual void add(index_type item) override {	selection.insert(item), invalidate(npos());	}
		virtual void remove(index_type item) override {	selection.erase(item), invalidate(npos());	}
		virtual bool contains(index_type item) const throw() override {	return !!selection.count(item);	}

		std::set<index_type> selection;
	};

	class my_columns : public headers_model
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
			caption.clear();
			switch (index)
			{
			case 0:
				caption << "Total\n" << agge::style::height_scale_base(0.8) << "(inclusive)";
				break;

			case 1:
				caption << "Total\n" << agge::style::height_scale_base(0.8) << "(exclusive)";
				break;

			case 2:
				caption << "Average\n" << agge::style::height_scale_base(0.8) << "(inclusive)";
				break;

			case 3:
				caption << "Average\n" << agge::style::height_scale_base(0.8) << "(exclusive)";
				break;
			}
		}
		virtual void set_width(index_type index, short int width) override { _widths[index] = width, invalidate(index); }
		virtual agge::full_alignment get_alignment(index_type index) const override
		{	return agge::full_alignment::create(index ? agge::align_far : agge::align_near, agge::align_center);	}

		virtual pair<index_type, bool> get_sort_order() const throw() override { return _sort_order; }
		virtual agge::full_alignment get_header_alignment(index_type /*index*/) const override
		{	return agge::full_alignment::create(agge::align_center, agge::align_center);	}

		virtual void activate_column(index_type index) override
		{
			_sort_order.first = index, _sort_order.second = !_sort_order.second;
			sort_order_changed(_sort_order.first, _sort_order.second);
		}

	private:
		vector<short> _widths;
		pair<index_type, bool> _sort_order;
	};

	class my_model : public richtext_table_model
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

		virtual void get_text(index_type row, index_type column, agge::richtext_t &text) const override
		{
			row++, column++;
			text << to_string(static_cast<long double>(row * _n + 13 * column * _n)).c_str()
				<< agge::style::height_scale(0.7) << "ms";
		}

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

	lv->set_columns_model(cm);
	lv->set_model(m);
	lv->set_selection_model(make_shared<my_selection>());

	f->set_root(root);
	f->set_location(l);
	f->set_visible(true);

	app.run();
}
