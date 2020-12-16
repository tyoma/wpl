#include "../stylesheet.h"

#include "../text_engine.h"

#include <agge.text/text_engine.h>
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

	shared_ptr<stylesheet> create_sample_stylesheet()
	{
		auto ssc = make_shared<stylesheet_composite>();
		shared_ptr<stylesheet> ss(ssc, &ssc->ss);

		if (ssc->text_engine)
		{
			ssc->ss.set_font("text", ssc->text_engine->create_font(L"Segoe UI", -15, false, false, agge::font::key::gf_vertical));
			ssc->ss.set_font("text.header", ssc->text_engine->create_font(L"Segoe UI", -15, true, false, agge::font::key::gf_vertical));

//			ssc->ss.set_font("text", ssc->text_engine->create_font(L"/System/Library/Fonts/HelveticaNeue.ttc", 15, false, false, agge::font::key::gf_vertical));
//			ssc->ss.set_font("text.header", ssc->text_engine->create_font(L"/System/Library/Fonts/HelveticaNeue.ttc", 15, true, false, agge::font::key::gf_vertical));
//			ssc->ss.set_font("text", ssc->text_engine->create_font(L"c:\\Windows\\Fonts\\segoeui.ttf", 15, false, false, agge::font::key::gf_vertical));
//			ssc->ss.set_font("text.header", ssc->text_engine->create_font(L"c:\\Windows\\Fonts\\segoeuib.ttf", 15, true, false, agge::font::key::gf_vertical));
		}
		ssc->ss.set_color("background", agge::color::make(16, 16, 16));
		ssc->ss.set_color("background.selected", agge::color::make(192, 192, 192));
		ssc->ss.set_color("background.listview.odd", agge::color::make(48, 48, 48));
		ssc->ss.set_color("text", agge::color::make(192, 192, 192));
		ssc->ss.set_color("text.selected", agge::color::make(16, 16, 16));
		ssc->ss.set_color("border", agge::color::make(64, 64, 96));

		ssc->ss.set_value("padding", 3);
		ssc->ss.set_value("border", 1);
		return ss;
	}
}
