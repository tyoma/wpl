#include <wpl/view_helpers.h>
#include <wpl/view.h>

#include <ut/assert.h>
#include <ut/test.h>

namespace wpl
{
	namespace tests
	{
		begin_test_suite( MiscTests )
			test( HasNoFocusOnCreation )
			{
				// INIT / ACT
				on_focus_invalidate<view> v;

				// ASSERT
				assert_is_false(v.has_focus);
			}


			test( FocusInOutSetsFlag )
			{
				// INIT
				on_focus_invalidate<view> v;

				// ACT
				static_cast<keyboard_input &>(v).got_focus();

				// ASSERT
				assert_is_true(v.has_focus);

				// ACT
				static_cast<keyboard_input &>(v).lost_focus();

				// ASSERT
				assert_is_false(v.has_focus);
			}


			test( FocusInOutInvalidatesView )
			{
				// INIT
				auto invalidates = 0;
				on_focus_invalidate<view> v;
				auto c = v.invalidate += [&] (const void *r) {
					assert_null(r);
					invalidates++;
				};

				// ACT
				static_cast<keyboard_input &>(v).got_focus();

				// ASSERT
				assert_equal(1, invalidates);

				// ACT
				static_cast<keyboard_input &>(v).lost_focus();
				static_cast<keyboard_input &>(v).got_focus();

				// ASSERT
				assert_equal(3, invalidates);
			}
		end_test_suite
	}
}
