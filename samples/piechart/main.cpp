#include "piechart.h"

#include <samples/common/application.h>
#include <samples/common/stylesheet.h>
#include <wpl/factory.h>
#include <wpl/form.h>
#include <wpl/layout.h>

using namespace std;
using namespace wpl;

int main()
{
	application app;

	auto ss = create_sample_stylesheet();
	auto fct = factory::create_default(ss);
	auto f = fct->create_form();
	const auto c = f->close += [&app] {	app.exit();	};

	const auto root = make_shared<overlay>();
		root->add(fct->create_control<control>("background"));

		auto vstack = make_shared<stack>(5, true);
		root->add(pad_control(vstack, 5, 5));
			const auto p1 = make_shared<piechart>();
			vstack->add(p1, 400);

			const auto p2 = make_shared<piechart>();
			vstack->add(p2, -100);

			const auto p3 = make_shared<piechart>();
			vstack->add(p3, 400);

	f->set_root(root);
	f->set_visible(true);

	app.run();
}
