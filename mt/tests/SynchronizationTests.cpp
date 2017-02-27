#include <wpl/mt/synchronization.h>

#include <ut/assert.h>
#include <ut/test.h>

namespace wpl
{
	namespace mt
	{
		namespace tests
		{
			begin_test_suite( SynchronizationTests )
				test( EventFlagCreateRaised )
				{
					// INIT
					event_flag e(true, false);

					// ACT / ASSERT
					assert_equal(waitable::satisfied, e.wait(0));
					assert_equal(waitable::satisfied, e.wait(10000));
					assert_equal(waitable::satisfied, e.wait(waitable::infinite));
				}


				test( EventFlagCreateLowered )
				{
					// INIT
					event_flag e(false, false);

					// ACT / ASSERT
					assert_equal(waitable::timeout, e.wait(0));
					assert_equal(waitable::timeout, e.wait(200));
				}


				test( EventFlagCreateAutoResettable )
				{
					// INIT
					event_flag e(true, true);

					e.wait(100);

					// ACT / ASSERT
					assert_equal(waitable::timeout, e.wait(100));
				}


				test( RaisingEventFlag )
				{
					// INIT
					event_flag e(false, false);

					// ACT
					e.raise();

					// ACT / ASSERT
					assert_equal(waitable::satisfied, e.wait());
				}


				test( LoweringEventFlag )
				{
					// INIT
					event_flag e(true, false);

					// ACT
					e.lower();

					// ACT / ASSERT
					assert_equal(waitable::timeout, e.wait(0));
				}
			end_test_suite
		}
	}
}
