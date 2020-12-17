#include "../stylesheet.h"

#include <agge.text/text_engine.h>
#include <wpl/stylesheet_db.h>

using namespace std;

#ifdef _WIN32
	const auto c_defaultFont = L"Segoe UI";
#else
	const auto c_defaultFont = L"Lucida Grande";
#endif

namespace wpl
{
	shared_ptr<stylesheet> create_sample_stylesheet(std::shared_ptr<gcontext::text_engine_type> text_engine)
	{
		const auto ss = make_shared<stylesheet_db>();

		ss->set_font("text", text_engine->create_font(c_defaultFont, 13, false, false, agge::font::key::gf_strong));
		ss->set_font("text.header", text_engine->create_font(c_defaultFont, 13, true, false, agge::font::key::gf_strong));
		ss->set_color("background", agge::color::make(16, 16, 16));
		ss->set_color("background.selected", agge::color::make(192, 192, 192));
		ss->set_color("background.listview.odd", agge::color::make(48, 48, 48));
		ss->set_color("text", agge::color::make(192, 192, 192));
		ss->set_color("text.selected", agge::color::make(16, 16, 16));
		ss->set_color("border", agge::color::make(64, 64, 96));

		ss->set_value("padding", 3);
		ss->set_value("border", 1);
		return ss;
	}
}
