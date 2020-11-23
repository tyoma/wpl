#include <wpl/win32/font_manager.h>

#include <ut/assert.h>
#include <ut/test.h>
#include <windows.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			LOGFONTW get_logfont(shared_ptr<void> hfont)
			{
				LOGFONTW r = { };

				assert_not_equal(0, ::GetObjectW(static_cast<HFONT>(hfont.get()), sizeof LOGFONTW, &r));
				return r;
			}
		}

		begin_test_suite( NativeFontManagerTests )
			test( DifferentFontsAreReturnedForTheDifferentKeys )
			{
				// INIT
				win32::font_manager m;

				// ACT
				shared_ptr<void> fonts[] = {
					m.get_font(agge::font::key(L"Arial", 10, true, false)),
					m.get_font(agge::font::key(L"Arial", 12, true, false)),
					m.get_font(agge::font::key(L"Arial", 12, true, true)),
					m.get_font(agge::font::key(L"Tahoma", -13, true, true)),
					m.get_font(agge::font::key(L"Times New Roman", 17, false, true)),
				};

				// ASSERT
				sort(begin(fonts), end(fonts));

				assert_equal(end(fonts), unique(begin(fonts), end(fonts)));
			}


			test( FontCreatedHasPropertiesCorrespondingToKey )
			{
				// INIT
				win32::font_manager m;

				// ACT
				shared_ptr<void> fonts[] = {
					m.get_font(agge::font::key(L"Arial", 10, true, false)),
					m.get_font(agge::font::key(L"Times New Roman", 17, false, false)),
					m.get_font(agge::font::key(L"Tahoma", -12, false, true)),
				};

				// ASSERT
				LOGFONTW lf[] = {
					get_logfont(fonts[0]),
					get_logfont(fonts[1]),
					get_logfont(fonts[2]),
				};

				assert_equal(L"Arial", wstring(lf[0].lfFaceName));
				assert_equal(10, lf[0].lfHeight);
				assert_equal(FW_BOLD, lf[0].lfWeight);
				assert_equal(0u, lf[0].lfItalic);
				assert_equal(L"Times New Roman", wstring(lf[1].lfFaceName));
				assert_equal(17, lf[1].lfHeight);
				assert_equal(FW_NORMAL, lf[1].lfWeight);
				assert_equal(0u, lf[1].lfItalic);
				assert_equal(L"Tahoma", wstring(lf[2].lfFaceName));
				assert_equal(-12, lf[2].lfHeight);
				assert_equal(FW_NORMAL, lf[2].lfWeight);
				assert_equal(1u, lf[2].lfItalic);
			}


			test( SameHandleIsReturnedForTheSameKeys )
			{
				// INIT
				win32::font_manager m;
				shared_ptr<void> fonts[] = {
					m.get_font(agge::font::key(L"Arial", 10, true, false)),
					m.get_font(agge::font::key(L"Times New Roman", 17, false, false)),
					m.get_font(agge::font::key(L"Tahoma", -12, false, true)),
				};

				// ACT / ASSERT
				assert_equal(fonts[0], m.get_font(agge::font::key(L"Arial", 10, true, false)));
				assert_equal(fonts[1], m.get_font(agge::font::key(L"Times New Roman", 17, false, false)));
				assert_equal(fonts[2], m.get_font(agge::font::key(L"Tahoma", -12, false, true)));
			}
		end_test_suite
	}
}
