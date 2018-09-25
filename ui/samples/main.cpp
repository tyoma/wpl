#include "piechart.h"

#include <ui/form.h>

using namespace std;
using namespace wpl;
using namespace wpl::ui;

void run_message_loop();
void exit_message_loop();

int main()
{
	shared_ptr<form> f = form::create();
	shared_ptr<piechart> pc(new piechart);
	slot_connection c = f->close += &exit_message_loop;

	f->set_view(pc);
	f->set_visible(true);
	run_message_loop();
}
