#include "../stylesheet.h"

#include <agge.text/text_engine.h>
#include <wpl/stylesheet_db.h>

using namespace agge;
using namespace std;

#ifdef _WIN32
	const auto c_defaultFont = "segoe ui";
#else
	const auto c_defaultFont = "Lucida Grande";
#endif

namespace wpl
{
	shared_ptr<stylesheet> create_sample_stylesheet(std::shared_ptr<gcontext::text_engine_type> text_engine)
	{
		const auto ss = make_shared<stylesheet_db>();

		ss->set_font("text", text_engine->create_font(font_descriptor::create(c_defaultFont, 14, regular, false, hint_vertical)));
		ss->set_font("text.header", text_engine->create_font(font_descriptor::create(c_defaultFont, 15, semi_bold, false, hint_vertical)));
		ss->set_color("background", agge::color::make(16, 16, 16));
		ss->set_color("background.selected", agge::color::make(192, 192, 192));
		ss->set_color("background.listview.odd", agge::color::make(48, 48, 48));
		ss->set_color("text", agge::color::make(192, 192, 192));
		ss->set_color("text.selected", agge::color::make(16, 16, 16));
		ss->set_color("border", agge::color::make(64, 64, 96));
		ss->set_color("separator", agge::color::make(42, 43, 44));

		ss->set_value("padding", 3);
		ss->set_value("border", 1);
		ss->set_value("separator", 1);
		return ss;
	}
}
