#include "piechart.h"
#include "view_fill.h"

#include <wpl/ui/container.h>
#include <wpl/ui/form.h>
#include <wpl/ui/layout.h>
#include <wpl/ui/listview.h>

#include <wpl/ui/win32/form.h>

using namespace std;
using namespace wpl;
using namespace wpl::ui;

void run_message_loop();
void exit_message_loop();
shared_ptr<listview> create_listview();

class spacer_layout : public layout_manager
{
public:
	spacer_layout(int spacing)
		: _spacing(spacing)
	{	}

	virtual void layout(unsigned width, unsigned height, container::positioned_view *views, size_t count) const
	{
		for (; count--; ++views)
		{
			views->location.left = _spacing, views->location.top = _spacing;
			views->location.width = width - 2 * _spacing, views->location.height = height - 2 * _spacing;
		}
	}

private:
	int _spacing;
};

int main()
{
	shared_ptr<form> f = create_form();
	shared_ptr<container> cc[] = {
		shared_ptr<container>(new view_fill<container>),
		shared_ptr<container>(new view_fill<container>),
	};
	shared_ptr<layout_manager> lm1(new spacer_layout(5));
	shared_ptr<stack> lm2(new stack(5, true));
	shared_ptr<piechart> charts[] = {
		shared_ptr<piechart>(new piechart),
		shared_ptr<piechart>(new piechart),
		shared_ptr<piechart>(new piechart),
	};
	slot_connection c = f->close += &exit_message_loop;

	cc[0]->set_layout(lm1);
	cc[0]->add_view(cc[1]);
	cc[1]->set_layout(lm2);
	lm2->add(400);
	cc[1]->add_view(charts[0]);
	lm2->add(-100);
	cc[1]->add_view(create_listview());
	lm2->add(400);
	cc[1]->add_view(charts[2]);
	f->set_view(cc[0]);
	f->set_visible(true);
	run_message_loop();
}
