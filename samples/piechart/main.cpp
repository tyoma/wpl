#include "piechart.h"
#include "view_fill.h"

#include <samples/common/factory.h>
#include <samples/common/platform.h>
#include <wpl/container.h>
#include <wpl/factory.h>
#include <wpl/form.h>
#include <wpl/layout.h>

using namespace std;
using namespace wpl;

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
	auto fct = create_sample_factory();
	shared_ptr<form> f = fct->create_form();
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
	cc[1]->add_view(shared_ptr<view>(new view));
	lm2->add(400);
	cc[1]->add_view(charts[2]);
	f->set_view(cc[0]);
	f->set_visible(true);
	run_message_loop();
}
