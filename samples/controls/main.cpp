#include <wpl/ui/combobox.h>
#include <wpl/ui/container.h>
#include <wpl/ui/form.h>
#include <wpl/ui/layout.h>
#include <wpl/ui/scroller.h>

#include <wpl/ui/win32/controls.h>
#include <wpl/ui/win32/form.h>

#include <samples/common/platform.h>
#include <samples/common/timer.h>

using namespace agge;
using namespace std;
using namespace wpl;
using namespace wpl::ui;

namespace
{
	color make_color(unsigned char r, unsigned char g, unsigned char b)
	{
		color c = { r, g, b, 255 };
		return c;
	}

	struct my_model : list_model
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

		virtual void get_text(index_type index, wstring &text) const
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

		struct my_trackable : wpl::ui::trackable
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
	font fnt = { L"", 8 };
	view_location l = { 100, 100, 300, 200 };
	shared_ptr<form> f = create_form();
	slot_connection c = f->close += &exit_message_loop;
	shared_ptr<container> root(new container);
	shared_ptr<stack> root_layout(new stack(5, false));
	shared_ptr<container> fill(new container);
	shared_ptr<stack> fill_layout(new stack(5, true));
	shared_ptr<combobox> cb = create_combobox();
	shared_ptr<scroller> scrl(new scroller(scroller::horizontal));
	shared_ptr<scroller> scrl2(new scroller(scroller::vertical));

	root->set_layout(root_layout);
	fill->set_layout(fill_layout);

	root_layout->add(40);
	root->add_view(cb);

	root_layout->add(20);
	root->add_view(scrl);

	root_layout->add(-100);
	root->add_view(fill);
		fill_layout->add(4);
		fill->add_view(scrl2);

	cb->set_model(shared_ptr<my_model>(new my_model));

	shared_ptr<my_scroll_model> scrl_model(new my_scroll_model);

	scrl->set_model(scrl_model);
	scrl2->set_model(scrl_model);

	f->set_font(fnt);
	f->set_view(root);
	f->set_location(l);
	f->set_visible(true);
	f->set_background_color(make_color(8, 32, 64));
	run_message_loop();
}
