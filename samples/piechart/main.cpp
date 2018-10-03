#include "piechart.h"
#include "view_fill.h"

#include <wpl/ui/container.h>
#include <wpl/ui/form.h>
#include <wpl/ui/layout.h>

using namespace std;
using namespace wpl;
using namespace wpl::ui;

void run_message_loop();
void exit_message_loop();

int main()
{
	shared_ptr<form> f = form::create();
	shared_ptr<container> cc(new view_fill<container>);
	int sizes[] = { 500, 900, 500, };
	shared_ptr<layout_manager> lm(new hstack(begin(sizes), end(sizes), 5));
	shared_ptr<piechart> charts[] = {
		shared_ptr<piechart>(new piechart),
		shared_ptr<piechart>(new piechart),
		shared_ptr<piechart>(new piechart),
	};
	slot_connection c = f->close += &exit_message_loop;

	cc->set_layout(lm);
	cc->add_view(charts[0]);
	cc->add_view(charts[1]);
	cc->add_view(charts[2]);
	f->set_view(cc);
	f->set_visible(true);
	run_message_loop();
}
