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
	const auto fct = app.create_default_factory();

	fct->register_control("piechart", [] (const factory &, const control_context &ctx) {
		return make_shared<piechart>(ctx);
	});

	auto f = fct->create_form();
	const auto c = f->close += [&app] {	app.exit();	};

	const auto root = make_shared<overlay>();
		root->add(fct->create_control<control>("background"));

		auto vstack = make_shared<stack>(5, true, fct->context.cursor_manager_);
		root->add(pad_control(vstack, 5, 5));
			const auto p1 = fct->create_control<piechart>("piechart");
			vstack->add(p1, percents(20), true);

			const auto p2 = fct->create_control<piechart>("piechart");
			vstack->add(p2, percents(60), true);

			const auto p3 = fct->create_control<piechart>("piechart");
			vstack->add(p3, percents(20), true);

	f->set_root(root);
	f->set_visible(true);

	app.run();
}
