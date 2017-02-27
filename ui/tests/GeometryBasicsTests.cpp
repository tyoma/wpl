#include <wpl/ui/geometry.h>

#include <ut/assert.h>
#include <ut/test.h>

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			begin_test_suite( GeometryBasicsTests )

				test( CreatedTransformMakesNoChangesInMap )
				{
					// INIT
					transform t;
					int x1 = 1, y1 = 3, x2 = 5, y2 = 7;

					// ACT
					t.map(x1, y1);
					t.map(x2, y2);

					// ASSERT
					assert_equal(1, x1);
					assert_equal(3, y1);
					assert_equal(5, x2);
					assert_equal(7, y2);
				}


				test( CreatedTransformMakesNoChangesInUnmap )
				{
					// INIT
					transform t;
					int x1 = 7, y1 = 5, x2 = 3, y2 = 1;

					// ACT
					t.unmap(x1, y1);
					t.unmap(x2, y2);

					// ASSERT
					assert_equal(7, x1);
					assert_equal(5, y1);
					assert_equal(3, x2);
					assert_equal(1, y2);
				}


				test( MapPointAccordinglyToOrigin )
				{
					// INIT
					transform t;
					int x = 7, y = 5;

					// ACT
					t.set_origin(19, 3);
					t.map(x, y);

					// ASSERT
					assert_equal(x, -12);
					assert_equal(2, y);

					// ACT
					t.set_origin(-12, 1);
					t.map(x, y);

					// ASSERT
					assert_equal(0, x);
					assert_equal(1, y);
				}


				test( UnmapPointAccordinglyToOrigin )
				{
					// INIT
					transform t;
					int x = 7, y = 5;

					// ACT
					t.set_origin(11, 7);
					t.unmap(x, y);

					// ASSERT
					assert_equal(18, x);
					assert_equal(12, y);

					// ACT
					t.set_origin(-12, 1);
					t.unmap(x, y);

					// ASSERT
					assert_equal(6, x);
					assert_equal(13, y);
				}
			end_test_suite
		}
	}
}
