#include <crtdbg.h>

#include <samples/common/platform.h>
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
				invalidated();
			});
		}

		virtual index_type get_count() const throw()
		{	return 4;	}

		virtual void get_value(index_type index, wstring &text) const
		{
			switch (index)
			{
			case 0: text = L"foo"; break;
			case 1: text = _dynamic_item; break;
			case 2: text = L"baz"; break;
			case 3: text = L"doodle"; break;
			}
		}

		virtual shared_ptr<const trackable> track(index_type row) const
		{
			shared_ptr<my_trackable> t(new my_trackable);
			t->_index = row;
			return t;
		}

		struct my_trackable : trackable
		{
			virtual index_type index() const {	return _index;	}
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

		virtual pair<double /*range_min*/, double /*range_width*/> get_range() const
		{	return make_pair(0, 100);	}

		virtual pair<double /*window_min*/, double /*window_width*/> get_window() const
		{	return _window;	}

		virtual double get_increment() const
		{	return 1.0;	}

		virtual void scrolling(bool /*begins*/)
		{	}

		virtual void scroll_window(double window_min, double window_width)
		{
			_window = make_pair(window_min, window_width);
			invalidated();
		}

	private:
		pair<double, double> _window;
	};
}

int main()
{
	auto ss = create_sample_stylesheet();
	auto fct = factory::create_default(ss);
	view_location l = { 100, 100, 300, 200 };
	shared_ptr<form> f = fct->create_form();
	slot_connection c = f->close += &exit_message_loop;

	auto root = make_shared<stack>(5, false);
		auto cb = fct->create_control<combobox>("combobox");
			root->add(cb, 40, 1);

		auto scrl = fct->create_control<scroller>("hscroller");
			root->add(scrl, 20);

		auto fill = make_shared<stack>(5, true);
			root->add(fill, -100);
				auto scrl2 = fct->create_control<scroller>("vscroller");
				fill->add(scrl2, 8);

	cb->set_model(shared_ptr<my_model>(new my_model));

	shared_ptr<my_scroll_model> scrl_model(new my_scroll_model);

	scrl->set_model(scrl_model);
	scrl2->set_model(scrl_model);

	new int;

	f->set_root(pad_control(root, 5, 5));
	f->set_location(l);
	f->set_visible(true);
	run_message_loop();
}
