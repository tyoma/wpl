#include <wpl/ui/layout.h>

#include "Mockups.h"
#include "TestHelpers.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		static bool operator ==(const container::positioned_view &lhs, const container::positioned_view &rhs)
		{	return lhs.left == rhs.left && lhs.top == rhs.top && lhs.width == rhs.width && lhs.height == rhs.height;	}

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
					hstack s1(begin(sizes1), end(sizes1), 0);
					hstack s2(begin(sizes2), end(sizes2), 0);

					// ACT
					s1.layout(10, 15, p1, _countof(p1));
					s2.layout(1000, 551, p2, _countof(p2));

					// ASSERT
					assert_equal(0, p1[0].left);
					assert_equal(0, p1[0].top);
					assert_equal(13, p1[0].width);
					assert_equal(15, p1[0].height);

					assert_equal(0, p2[0].left);
					assert_equal(0, p2[0].top);
					assert_equal(171, p2[0].width);
					assert_equal(551, p2[0].height);

					// ACT
					s1.layout(10, 19, p1, _countof(p1));
					s2.layout(1000, 41, p2, _countof(p2));

					// ASSERT
					assert_equal(0, p1[0].left);
					assert_equal(0, p1[0].top);
					assert_equal(13, p1[0].width);
					assert_equal(19, p1[0].height);

					assert_equal(0, p2[0].left);
					assert_equal(0, p2[0].top);
					assert_equal(171, p2[0].width);
					assert_equal(41, p2[0].height);
				}


				test( LayoutHSeveralWidgetsAbsolute )
				{
					// INIT
					int sizes[] = { 13, 17, 121 };
					container::positioned_view p[_countof(sizes)];

					// INIT / ACT
					hstack s(begin(sizes), end(sizes), 0);

					// ACT
					s.layout(10, 15, p, _countof(p));

					// ASSERT
					container::positioned_view reference1[] = {
						{ 0, 0, 13, 15 },
						{ 13, 0, 17, 15 },
						{ 30, 0, 121, 15 },
					};

					assert_equal(reference1[0], p[0]);
					assert_equal(reference1[1], p[1]);
					assert_equal(reference1[2], p[2]);

					// ACT
					s.layout(10, 19, p, _countof(p));

					// ASSERT
					container::positioned_view reference2[] = {
						{ 0, 0, 13, 19 },
						{ 13, 0, 17, 19 },
						{ 30, 0, 121, 19 },
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
					vstack s(begin(sizes), end(sizes), 0);

					// ACT
					s.layout(11, 15, p, _countof(p));

					// ASSERT
					container::positioned_view reference1[] = {
						{ 0, 0, 11, 13 },
						{ 0, 13, 11, 17 },
						{ 0, 30, 11, 121 },
					};

					assert_equal(reference1[0], p[0]);
					assert_equal(reference1[1], p[1]);
					assert_equal(reference1[2], p[2]);

					// ACT
					s.layout(21, 19, p, _countof(p));

					// ASSERT
					container::positioned_view reference2[] = {
						{ 0, 0, 21, 13 },
						{ 0, 13, 21, 17 },
						{ 0, 30, 21, 121 },
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
					vstack s(begin(sizes), end(sizes), 5);

					// ACT
					s.layout(11, 15, p, _countof(p));

					// ASSERT
					container::positioned_view reference1[] = {
						{ 0, 0, 11, 13 },
						{ 0, 18, 11, 17 },
						{ 0, 40, 11, 121 },
						{ 0, 166, 11, 71 },
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
					hstack s1(begin(sizes1), end(sizes1), 0);
					hstack s2(begin(sizes2), end(sizes2), 0);

					// ACT
					s1.layout(11, 15, p1, _countof(p1));
					s2.layout(1103, 315, p2, _countof(p2));

					// ASSERT
					container::positioned_view reference[] = {
						{ 0, 0, 11, 15 },
						{ 0, 0, 1103, 315 },
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
					vstack s1(begin(sizes1), end(sizes1), 0);
					vstack s2(begin(sizes2), end(sizes2), 0);

					// ACT
					s1.layout(19, 1315, p1, _countof(p1));
					s2.layout(31, 316, p2, _countof(p2));

					// ASSERT
					container::positioned_view reference1[] = {
						{ 0, 0, 19, 370 },
						{ 0, 370, 19, 759 },
						{ 0, 1129, 19, 185 },
					};
					container::positioned_view reference2[] = {
						{ 0, 0, 31, 158 },
						{ 0, 158, 31, 158 },
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
					vstack s1(begin(sizes1), end(sizes1), 3);
					vstack s2(begin(sizes2), end(sizes2), 7);

					// ACT
					s1.layout(19, 1315, p1, _countof(p1));
					s2.layout(31, 316, p2, _countof(p2));

					// ASSERT
					container::positioned_view reference1[] = {
						{ 0, 0, 19, 795 },
						{ 0, 798, 19, 100 },
						{ 0, 901, 19, 413 },
					};
					container::positioned_view reference2[] = {
						{ 0, 0, 31, 202 },
						{ 0, 209, 31, 107 },
					};

					assert_equal(reference1[0], p1[0]);
					assert_equal(reference1[1], p1[1]);
					assert_equal(reference1[2], p1[2]);

					assert_equal(reference2[0], p2[0]);
					assert_equal(reference2[1], p2[1]);
				}

			end_test_suite
		}
	}
}
