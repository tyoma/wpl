#include <samples/common/application.h>
#include <samples/common/stylesheet.h>
#include <samples/common/timer.h>
#include <wpl/controls.h>
#include <wpl/factory.h>
#include <wpl/form.h>
#include <wpl/layout.h>

using namespace agge;
using namespace std;
using namespace wpl;

namespace
{
	color make_color(unsigned char r, unsigned char g, unsigned char b)
	{
		color c = { r, g, b, 255 };
		return c;
	}

	struct my_model : list_model<wstring>
	{
		my_model()
		{
			_timer = create_timer(20, [this] (int elapsed) {
				wchar_t buffer[100];

				swprintf(buffer, sizeof(buffer), L"Dynamic: %d", elapsed);
				_dynamic_item = buffer;
				invalidate();
			});
		}

		virtual index_type get_count() const throw() override
		{	return 4;	}

		virtual void get_value(index_type index, wstring &text) const override
		{
			switch (index)
			{
			case 0: text = L"foo"; break;
			case 1: text = _dynamic_item; break;
			case 2: text = L"baz"; break;
			case 3: text = L"doodle"; break;
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

		shared_ptr<void> _timer;
		wstring _dynamic_item;
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
			invalidate();
		}

	private:
		pair<double, double> _window;
	};
}

int main()
{
	application app;

	auto ss = create_sample_stylesheet();
	auto fct = factory::create_default(ss);
	view_location l = { 100, 100, 300, 200 };
	shared_ptr<form> f = fct->create_form();
	slot_connection c = f->close += [&app] {	app.exit();	};
	shared_ptr<my_scroll_model> scrl_model(new my_scroll_model);

	const auto root = make_shared<overlay>();
		root->add(fct->create_control<control>("background"));

		const auto vstack = make_shared<stack>(5, false);
		root->add(pad_control(vstack, 5, 5));
			auto cb = fct->create_control<combobox>("combobox");
			vstack->add(cb, 40, 1);
			cb->set_model(shared_ptr<my_model>(new my_model));

			auto scrl = fct->create_control<scroller>("hscroller");
			vstack->add(scrl, 20);
			scrl->set_model(scrl_model);

			auto fill = make_shared<stack>(5, true);
			vstack->add(fill, -100);
				auto scrl2 = fct->create_control<scroller>("vscroller");
				fill->add(scrl2, 8);
				scrl2->set_model(scrl_model);

	f->set_root(root);
	f->set_location(l);
	f->set_visible(true);
	
	app.run();
}
