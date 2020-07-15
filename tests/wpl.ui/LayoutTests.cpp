#include <wpl/layout.h>

#include "helpers.h"
#include "Mockups.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace
	{
		template <typename IteratorT>
		stack hstack(IteratorT b, IteratorT e, int spacing)
		{
			stack s(spacing, true);

			for (; b != e; ++b)
				s.add(*b);
			return s;
		}

		template <typename IteratorT>
		stack vstack(IteratorT b, IteratorT e, int spacing)
		{
			stack s(spacing, false);

			for (; b != e; ++b)
				s.add(*b);
			return s;
		}
	}

	static bool operator ==(const container::positioned_view &lhs, const container::positioned_view &rhs)
	{	return lhs.location == rhs.location;	}

	namespace tests
	{
		begin_test_suite( StackLayoutTest )

			test( LayoutHSingleWidgetAbsolute )
			{
				// INIT
				int sizes1[] = { 13, };
				int sizes2[] = { 171, };
				container::positioned_view p1[_countof(sizes1)];
				container::positioned_view p2[_countof(sizes2)];

				// INIT / ACT
				stack s1 = hstack(begin(sizes1), end(sizes1), 0);
				stack s2 = hstack(begin(sizes2), end(sizes2), 0);

				// ACT
				s1.layout(10, 15, p1, _countof(p1));
				s2.layout(1000, 551, p2, _countof(p2));

				// ASSERT
				assert_equal(0, p1[0].location.left);
				assert_equal(0, p1[0].location.top);
				assert_equal(13, p1[0].location.width);
				assert_equal(15, p1[0].location.height);

				assert_equal(0, p2[0].location.left);
				assert_equal(0, p2[0].location.top);
				assert_equal(171, p2[0].location.width);
				assert_equal(551, p2[0].location.height);

				// ACT
				s1.layout(10, 19, p1, _countof(p1));
				s2.layout(1000, 41, p2, _countof(p2));

				// ASSERT
				assert_equal(0, p1[0].location.left);
				assert_equal(0, p1[0].location.top);
				assert_equal(13, p1[0].location.width);
				assert_equal(19, p1[0].location.height);

				assert_equal(0, p2[0].location.left);
				assert_equal(0, p2[0].location.top);
				assert_equal(171, p2[0].location.width);
				assert_equal(41, p2[0].location.height);
			}


			test( LayoutHSeveralWidgetsAbsolute )
			{
				// INIT
				int sizes[] = { 13, 17, 121 };
				container::positioned_view p[_countof(sizes)];

				// INIT / ACT
				stack s = hstack(begin(sizes), end(sizes), 0);

				// ACT
				s.layout(10, 15, p, _countof(p));

				// ASSERT
				container::positioned_view reference1[] = {
					{ { 0, 0, 13, 15 }, },
					{ { 13, 0, 17, 15 }, },
					{ { 30, 0, 121, 15 }, },
				};

				assert_equal(reference1[0], p[0]);
				assert_equal(reference1[1], p[1]);
				assert_equal(reference1[2], p[2]);

				// ACT
				s.layout(10, 19, p, _countof(p));

				// ASSERT
				container::positioned_view reference2[] = {
					{ { 0, 0, 13, 19 }, },
					{ { 13, 0, 17, 19 }, },
					{ { 30, 0, 121, 19 }, },
				};

				assert_equal(reference2[0], p[0]);
				assert_equal(reference2[1], p[1]);
				assert_equal(reference2[2], p[2]);
			}


			test( LayoutVSeveralWidgetsAbsolute )
			{
				// INIT
				int sizes[] = { 13, 17, 121 };
				container::positioned_view p[_countof(sizes)];

				// INIT / ACT
				stack s = vstack(begin(sizes), end(sizes), 0);

				// ACT
				s.layout(11, 15, p, _countof(p));

				// ASSERT
				container::positioned_view reference1[] = {
					{ { 0, 0, 11, 13 }, }, 
					{ { 0, 13, 11, 17 }, },
					{ { 0, 30, 11, 121 }, },
				};

				assert_equal(reference1[0], p[0]);
				assert_equal(reference1[1], p[1]);
				assert_equal(reference1[2], p[2]);

				// ACT
				s.layout(21, 19, p, _countof(p));

				// ASSERT
				container::positioned_view reference2[] = {
					{ { 0, 0, 21, 13 }, },
					{ { 0, 13, 21, 17 }, },
					{ { 0, 30, 21, 121 }, },
				};

				assert_equal(reference2[0], p[0]);
				assert_equal(reference2[1], p[1]);
				assert_equal(reference2[2], p[2]);
			}


			test( LayoutVSeveralWidgetsAbsoluteSpaced )
			{
				// INIT
				int sizes[] = { 13, 17, 121, 71 };
				container::positioned_view p[_countof(sizes)];

				// INIT / ACT
				stack s = vstack(begin(sizes), end(sizes), 5);

				// ACT
				s.layout(11, 15, p, _countof(p));

				// ASSERT
				container::positioned_view reference1[] = {
					{ { 0, 0, 11, 13 }, },
					{ { 0, 18, 11, 17 }, },
					{ { 0, 40, 11, 121 }, },
					{ { 0, 166, 11, 71 }, },
				};

				assert_equal(reference1[0], p[0]);
				assert_equal(reference1[1], p[1]);
				assert_equal(reference1[2], p[2]);
				assert_equal(reference1[3], p[3]);
			}


			test( LayoutHSingleWidgetRelativelySpaced )
			{
				// INIT
				int sizes1[] = { -10000 /* 10000 / 10000 */ };
				int sizes2[] = { -5750 /* 5750 / 5750 */ };
				container::positioned_view p1[_countof(sizes1)];
				container::positioned_view p2[_countof(sizes2)];
				stack s1 = hstack(begin(sizes1), end(sizes1), 0);
				stack s2 = hstack(begin(sizes2), end(sizes2), 0);

				// ACT
				s1.layout(11, 15, p1, _countof(p1));
				s2.layout(1103, 315, p2, _countof(p2));

				// ASSERT
				container::positioned_view reference[] = {
					{ { 0, 0, 11, 15 }, },
					{ { 0, 0, 1103, 315 }, },
				};

				assert_equal(reference[0], p1[0]);
				assert_equal(reference[1], p2[0]);
			}


			test( LayoutVSeveralWidgetsRelativelySpaced )
			{
				// INIT
				int sizes1[] = { -10000 /* 10000 / 35500 */, -20500 /* 20000 / 35500 */, -5000 /* 5000 / 35500 */, };
				int sizes2[] = { -5750 /* 5750 / 11500 */, -5750 /* 5750 / 11500 */, };
				container::positioned_view p1[_countof(sizes1)];
				container::positioned_view p2[_countof(sizes2)];
				stack s1 = vstack(begin(sizes1), end(sizes1), 0);
				stack s2 = vstack(begin(sizes2), end(sizes2), 0);

				// ACT
				s1.layout(19, 1315, p1, _countof(p1));
				s2.layout(31, 316, p2, _countof(p2));

				// ASSERT
				container::positioned_view reference1[] = {
					{ { 0, 0, 19, 370 }, },
					{ { 0, 370, 19, 759 }, },
					{ { 0, 1129, 19, 185 }, },
				};
				container::positioned_view reference2[] = {
					{ { 0, 0, 31, 158 }, },
					{ { 0, 158, 31, 158 }, },
				};

				assert_equal(reference1[0], p1[0]);
				assert_equal(reference1[1], p1[1]);
				assert_equal(reference1[2], p1[2]);

				assert_equal(reference2[0], p2[0]);
				assert_equal(reference2[1], p2[1]);
			}


			test( LayoutVSeveralWidgetsRelativelyAndAbsolutelySpacedWithInnerSpacing )
			{
				// INIT
				int sizes1[] = { -10000 /* 10000 / 15200 */, 100, -5200 /* 5200 / 15200 */, };
				int sizes2[] = { -5750 /* 5750 / 5750 */, 107, };
				container::positioned_view p1[_countof(sizes1)];
				container::positioned_view p2[_countof(sizes2)];
				stack s1 = vstack(begin(sizes1), end(sizes1), 3);
				stack s2 = vstack(begin(sizes2), end(sizes2), 7);

				// ACT
				s1.layout(19, 1315, p1, _countof(p1));
				s2.layout(31, 316, p2, _countof(p2));

				// ASSERT
				container::positioned_view reference1[] = {
					{ { 0, 0, 19, 795 }, },
					{ { 0, 798, 19, 100 }, },
					{ { 0, 901, 19, 413 }, },
				};
				container::positioned_view reference2[] = {
					{ { 0, 0, 31, 202 }, },
					{ { 0, 209, 31, 107 }, },
				};

				assert_equal(reference1[0], p1[0]);
				assert_equal(reference1[1], p1[1]);
				assert_equal(reference1[2], p1[2]);

				assert_equal(reference2[0], p2[0]);
				assert_equal(reference2[1], p2[1]);
			}
		end_test_suite


		begin_test_suite( SpacerLayoutTests )
			test( AllViewsAreSpacedByCXAndCY )
			{
				// INIT
				spacer rs1(5, 7);
				layout_manager &s1 = rs1;
				spacer rs2(3, 11);
				layout_manager &s2 = rs2;
				container::positioned_view p1[1];
				container::positioned_view p2[2];

				// ACT
				s1.layout(100, 120, p1, _countof(p1));
				s2.layout(100, 120, p2, _countof(p2));

				// ASSERT
				container::positioned_view reference1[] = {
					{ { 5, 7, 90, 106 }, },
				};
				container::positioned_view reference2[] = {
					{ { 3, 11, 94, 98 }, },
					{ { 3, 11, 94, 98 }, },
				};

				assert_equal(reference1, p1);
				assert_equal(reference2, p2);

				// ACT
				s1.layout(51, 91, p1, _countof(p1));
				s2.layout(51, 91, p2, _countof(p2));

				// ASSERT
				container::positioned_view reference3[] = {
					{ { 5, 7, 41, 77 }, },
				};
				container::positioned_view reference4[] = {
					{ { 3, 11, 45, 69 }, },
					{ { 3, 11, 45, 69 }, },
				};

				assert_equal(reference3, p1);
				assert_equal(reference4, p2);
			}
		end_test_suite
	}
}
