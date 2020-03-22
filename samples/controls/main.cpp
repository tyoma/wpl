#include <wpl/ui/combobox.h>
#include <wpl/ui/form.h>

#include <wpl/ui/win32/controls.h>
#include <wpl/ui/win32/form.h>

#include <samples/common/platform.h>

using namespace std;
using namespace wpl;
using namespace wpl::ui;

namespace
{
	struct my_model : list_model
	{
		virtual index_type get_count() const throw()
		{	return 4;	}

		virtual void get_text(index_type index, wstring &text) const
		{
			switch (index)
			{
			case 0: text = L"foo"; break;
			case 1: text = L"bar"; break;
			case 2: text = L"baz"; break;
			case 3: text = L"doodle"; break;
			}
		}
	};
}

int main()
{
	font fnt = { L"", 8 };
	view_location l = { 100, 100, 300, 200 };
	shared_ptr<form> f = create_form();
	slot_connection c = f->close += &exit_message_loop;
	shared_ptr<combobox> cb = create_combobox();

	cb->set_model(shared_ptr<my_model>(new my_model));

	f->set_font(fnt);
	f->set_view(cb);
	f->set_location(l);
	f->set_visible(true);
	run_message_loop();
}
