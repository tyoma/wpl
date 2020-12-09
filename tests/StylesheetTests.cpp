#include <wpl/stylesheet_db.h>

#include "helpers-visual.h"

#include <agge.text/font.h>
#include <stdexcept>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		begin_test_suite( StylesheetTests )

			struct dummy_accessor_ : agge::font::accessor
			{
				virtual agge::font::metrics get_metrics() const override {	return agge::zero();	}
				virtual agge::uint16_t get_glyph_index(wchar_t) const override {	throw 0;	}
				virtual agge::glyph::outline_ptr load_glyph(agge::uint16_t, agge::glyph::glyph_metrics &) const override {	throw 0;	}
			};

			shared_ptr<dummy_accessor_> dummy_accessor;
			agge::font::ptr font1, font2, font3;

			init( Init )
			{
				dummy_accessor.reset(new dummy_accessor_);
				font1.reset(new agge::font(agge::font::key(L"", 1), dummy_accessor));
				font2.reset(new agge::font(agge::font::key(L"", 1), dummy_accessor));
				font3.reset(new agge::font(agge::font::key(L"", 1), dummy_accessor));
			}


			test( ExceptionIsThrownOnMissingStyle )
			{
				// INIT / ACT
				stylesheet_db sdb;
				stylesheet &s = sdb;

				// ACT / ASSERT
				assert_throws(s.get_color("background"), invalid_argument);
				assert_throws(s.get_font("text"), invalid_argument);
				assert_throws(s.get_value("rounding.button"), invalid_argument);
			}


			test( StyleIsFoundOnExactBasis )
			{
				// INIT / ACT
				stylesheet_db sdb;
				stylesheet &s = sdb;

				sdb.set_color("background", agge::color::make(123, 11, 1, 201));
				sdb.set_color("chart.focused", agge::color::make(127, 13, 2, 210));
				sdb.set_color("text.listview.selected", agge::color::make(255, 0, 0, 128));
				sdb.set_font("text.listview", font1);
				sdb.set_font("text.edit", font2);
				sdb.set_font("text", font3);
				sdb.set_value("rounding", 0.5f);
				sdb.set_value("rounding.button", 2.0f);

				// ACT / ASSERT
				assert_equal(agge::color::make(123, 11, 1, 201), s.get_color("background"));
				assert_equal(agge::color::make(127, 13, 2, 210), s.get_color("chart.focused"));
				assert_equal(agge::color::make(255, 0, 0, 128), s.get_color("text.listview.selected"));

				assert_equal(font1, s.get_font("text.listview"));
				assert_equal(font2, s.get_font("text.edit"));
				assert_equal(font3, s.get_font("text"));

				assert_equal(0.5f, s.get_value("rounding"));
				assert_equal(2.0f, s.get_value("rounding.button"));
			}


			test( ColorStyleIsFoundOnSegmentedMatch )
			{
				// INIT / ACT
				stylesheet_db sdb;
				stylesheet &s = sdb;

				sdb.set_color("background", agge::color::make(123, 11, 1, 201));
				sdb.set_color("focus", agge::color::make(127, 13, 2, 210));
				sdb.set_color("text.selected.listview", agge::color::make(255, 0, 0, 128));
				sdb.set_color("text.selected", agge::color::make(100, 10, 10, 128));

				// ACT / ASSERT
				assert_equal(agge::color::make(123, 11, 1, 201), s.get_color("background.button"));
				assert_equal(agge::color::make(127, 13, 2, 210), s.get_color("focus.unfocused.combobox"));
				assert_equal(agge::color::make(255, 0, 0, 128), s.get_color("text.selected.listview.odd.codd"));
				assert_equal(agge::color::make(100, 10, 10, 128), s.get_color("text.selected.edit"));
			}


			test( FontStyleIsFoundOnSegmentedMatch )
			{
				// INIT / ACT
				stylesheet_db sdb;
				stylesheet &s = sdb;

				sdb.set_font("background", font1);
				sdb.set_font("text", font2);
				sdb.set_font("text.selected.listview", font3);

				// ACT / ASSERT
				assert_equal(font1, s.get_font("background.button"));
				assert_equal(font2, s.get_font("text.combobox"));
				assert_equal(font3, s.get_font("text.selected.listview.odd.codd"));
			}


			test( ValueStyleIsFoundOnSegmentedMatch )
			{
				// INIT / ACT
				stylesheet_db sdb;
				stylesheet &s = sdb;

				sdb.set_value("background", 0.13f);
				sdb.set_value("focus", 19.2f);
				sdb.set_value("text.selected.listview", 21.32f);
				sdb.set_value("text.selected", 31.32f);

				// ACT / ASSERT
				assert_equal(0.13f, s.get_value("background.button"));
				assert_equal(19.2f, s.get_value("focus.unfocused.combobox"));
				assert_equal(21.32f, s.get_value("text.selected.listview.odd.codd"));
				assert_equal(31.32f, s.get_value("text.selected.edit"));
			}
		end_test_suite
	}
}
