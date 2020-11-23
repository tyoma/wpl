#include "factory.h"

#include "platform.h"

#include <agge.text/text_engine.h>
#include <wpl/factory.h>
#include <wpl/stylesheet_db.h>

using namespace std;

namespace wpl
{
	namespace
	{
		struct stylesheet_composite
		{
			stylesheet_composite()
				: text_engine(create_text_engine())
			{	}

			shared_ptr<gcontext::text_engine_type> text_engine;
			stylesheet_db ss;
		};
	}

	std::shared_ptr<factory> create_sample_factory()
	{
		auto ssc = make_shared<stylesheet_composite>();
		shared_ptr<stylesheet> ss(ssc, &ssc->ss);
		auto fct = factory::create_default(ss);

		ssc->ss.set_font("text", ssc->text_engine->create_font(L"Segoe UI", 16, false, false, agge::font::key::gf_vertical));
		ssc->ss.set_color("background", agge::color::make(16, 16, 16));
		ssc->ss.set_color("background.selected", agge::color::make(192, 192, 192));
		ssc->ss.set_color("background.listview.odd", agge::color::make(48, 48, 48));
		ssc->ss.set_color("text", agge::color::make(192, 192, 192));
		ssc->ss.set_color("text.selected", agge::color::make(16, 16, 16));
		ssc->ss.set_color("border", agge::color::make(64, 64, 96));

		ssc->ss.set_value("padding", 3);
		ssc->ss.set_value("border", 1);
		return fct;
	}
}
