#include "piechart.h"

#include <samples/common/platform.h>
#include <samples/common/stylesheet.h>
#include <wpl/factory.h>
#include <wpl/form.h>
#include <wpl/layout.h>

using namespace std;
using namespace wpl;

int main()
{
	auto ss = create_sample_stylesheet();
	auto fct = factory::create_default(ss);
	auto f = fct->create_form();
	const auto c = f->close += &exit_message_loop;

	auto root = make_shared<stack>(5, true);
		const auto p1 = make_shared<piechart>();
		root->add(p1, 400);

		const auto p2 = make_shared<piechart>();
		root->add(p2, -100);

		const auto p3 = make_shared<piechart>();
		root->add(p3, 400);

	f->set_root(root);
	f->set_visible(true);
	run_message_loop();
}
