#include <samples/common/application.h>
#include <wpl/controls.h>
#include <wpl/factory.h>
#include <wpl/form.h>
#include <wpl/layout.h>

using namespace std;
using namespace wpl;

namespace
{
	struct my_model : list_model<string>
	{
		my_model(const queue &queue_)
			: _queue(queue_), _elapsed(0), _alive(make_shared<bool>(true))
		{	on_timer(_alive);	}

		~my_model()
		{	*_alive = false;	}

		void on_timer(const shared_ptr<bool> &alive)
		{
			if (*alive)
			{
				char buffer[100];

				sprintf(buffer, "Dynamic: %d", _elapsed += 10);
				_dynamic_item = buffer;
				invalidate(npos());
				_queue([this, alive] {	on_timer(alive);	}, 10);
			}
		}

		virtual index_type get_count() const throw() override
		{	return 4;	}

		virtual void get_value(index_type index, string &text) const override
		{
			switch (index)
			{
			case 0: text = "foo"; break;
			case 1: text = _dynamic_item; break;
			case 2: text = "baz"; break;
			case 3: text = "doodle"; break;
			}
		}

		virtual shared_ptr<const trackable> track(index_type row) const override
		{
			shared_ptr<my_trackable> t(new my_trackable);
			t->_index = row;
			return t;
		}

		struct my_trackable : trackable
		{
			virtual index_type index() const override {	return _index;	}
			index_type _index;
		};

		queue _queue;
		int _elapsed;
		string _dynamic_item;
		shared_ptr<bool> _alive;
	};

	struct my_scroll_model : scroll_model
	{
		my_scroll_model()
			: _window(0, 7)
		{	}

		virtual pair<double /*range_min*/, double /*range_width*/> get_range() const override
		{	return make_pair(0, 100);	}

		virtual pair<double /*window_min*/, double /*window_width*/> get_window() const override
		{	return _window;	}

		virtual double get_increment() const override
		{	return 1.0;	}

		virtual void scrolling(bool /*begins*/) override
		{	}

		virtual void scroll_window(double window_min, double window_width) override
		{
			_window = make_pair(window_min, window_width);
			invalidate(false);
		}

	private:
		pair<double, double> _window;
	};
}

int main()
{
	application app;
	const auto fct = app.create_default_factory();
	const rect_i l = { 100, 100, 400, 300 };
	shared_ptr<form> f = fct->create_form();
	slot_connection c = f->close += [&app] {	app.exit();	};
	shared_ptr<my_scroll_model> scrl_model(new my_scroll_model);

	const auto root = make_shared<overlay>();
		root->add(fct->create_control<control>("background"));

		const auto vstack = fct->create_control<stack>("vstack");
		vstack->set_spacing(5);
		root->add(pad_control(vstack, 5, 5));
			auto cb = fct->create_control<combobox>("combobox");
			vstack->add(cb, pixels(40), false);
			cb->set_model(shared_ptr<my_model>(new my_model(app.get_application_queue())));

			auto scrl = fct->create_control<scroller>("hscroller");
			vstack->add(scrl, pixels(20), false);
			scrl->set_model(scrl_model);

			auto fill = fct->create_control<stack>("hstack");
			fill->set_spacing(5);
			vstack->add(fill, percents(100), false);
				auto scrl2 = fct->create_control<scroller>("vscroller");
				fill->add(scrl2, pixels(8), false);
				scrl2->set_model(scrl_model);

	f->set_root(root);
	f->set_location(l);
	f->set_visible(true);
	
	app.run();
}
