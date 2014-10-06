#include <wpl/ui/layout.h>

#include "Mockups.h"
#include "TestHelpers.h"

using namespace std;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace
			{
				bool operator ==(const layout_manager::position &lhs, const layout_manager::position &rhs)
				{	return lhs.left == rhs.left && lhs.top == rhs.top && lhs.width == rhs.width && lhs.height == rhs.height;	}
			}

			[TestClass]
			public ref class StackLayoutTests
			{
			public:
				[TestMethod]
				void LayoutHSingleWidgetAbsolute()
				{
					// INIT
					int sizes1[] = { 13, };
					int sizes2[] = { 171, };
					layout_manager::widget_position p1[_countof(sizes1)];
					layout_manager::widget_position p2[_countof(sizes2)];

					// INIT / ACT
					hstack s1(ut::begin(sizes1), ut::end(sizes1), 0);
					hstack s2(ut::begin(sizes2), ut::end(sizes2), 0);

					// ACT
					s1.layout(10, 15, p1, _countof(p1));
					s2.layout(1000, 551, p2, _countof(p2));

					// ASSERT
					Assert::IsTrue(0 == p1[0].second.left);
					Assert::IsTrue(0 == p1[0].second.top);
					Assert::IsTrue(13 == p1[0].second.width);
					Assert::IsTrue(15 == p1[0].second.height);

					Assert::IsTrue(0 == p2[0].second.left);
					Assert::IsTrue(0 == p2[0].second.top);
					Assert::IsTrue(171 == p2[0].second.width);
					Assert::IsTrue(551 == p2[0].second.height);

					// ACT
					s1.layout(10, 19, p1, _countof(p1));
					s2.layout(1000, 41, p2, _countof(p2));

					// ASSERT
					Assert::IsTrue(0 == p1[0].second.left);
					Assert::IsTrue(0 == p1[0].second.top);
					Assert::IsTrue(13 == p1[0].second.width);
					Assert::IsTrue(19 == p1[0].second.height);

					Assert::IsTrue(0 == p2[0].second.left);
					Assert::IsTrue(0 == p2[0].second.top);
					Assert::IsTrue(171 == p2[0].second.width);
					Assert::IsTrue(41 == p2[0].second.height);
				}


				[TestMethod]
				void LayoutHSeveralWidgetsAbsolute()
				{
					// INIT
					int sizes[] = { 13, 17, 121 };
					layout_manager::widget_position p[_countof(sizes)];

					// INIT / ACT
					hstack s(ut::begin(sizes), ut::end(sizes), 0);

					// ACT
					s.layout(10, 15, p, _countof(p));

					// ASSERT
					layout_manager::position reference1[] = {
						{ 0, 0, 13, 15 },
						{ 13, 0, 17, 15 },
						{ 30, 0, 121, 15 },
					};

					Assert::IsTrue(reference1[0] == p[0].second);
					Assert::IsTrue(reference1[1] == p[1].second);
					Assert::IsTrue(reference1[2] == p[2].second);

					// ACT
					s.layout(10, 19, p, _countof(p));

					// ASSERT
					layout_manager::position reference2[] = {
						{ 0, 0, 13, 19 },
						{ 13, 0, 17, 19 },
						{ 30, 0, 121, 19 },
					};

					Assert::IsTrue(reference2[0] == p[0].second);
					Assert::IsTrue(reference2[1] == p[1].second);
					Assert::IsTrue(reference2[2] == p[2].second);
				}


				[TestMethod]
				void LayoutVSeveralWidgetsAbsolute()
				{
					// INIT
					int sizes[] = { 13, 17, 121 };
					layout_manager::widget_position p[_countof(sizes)];

					// INIT / ACT
					vstack s(ut::begin(sizes), ut::end(sizes), 0);

					// ACT
					s.layout(11, 15, p, _countof(p));

					// ASSERT
					layout_manager::position reference1[] = {
						{ 0, 0, 11, 13 },
						{ 0, 13, 11, 17 },
						{ 0, 30, 11, 121 },
					};

					Assert::IsTrue(reference1[0] == p[0].second);
					Assert::IsTrue(reference1[1] == p[1].second);
					Assert::IsTrue(reference1[2] == p[2].second);

					// ACT
					s.layout(21, 19, p, _countof(p));

					// ASSERT
					layout_manager::position reference2[] = {
						{ 0, 0, 21, 13 },
						{ 0, 13, 21, 17 },
						{ 0, 30, 21, 121 },
					};

					Assert::IsTrue(reference2[0] == p[0].second);
					Assert::IsTrue(reference2[1] == p[1].second);
					Assert::IsTrue(reference2[2] == p[2].second);
				}


				[TestMethod]
				void LayoutVSeveralWidgetsAbsoluteSpaced()
				{
					// INIT
					int sizes[] = { 13, 17, 121, 71 };
					layout_manager::widget_position p[_countof(sizes)];

					// INIT / ACT
					vstack s(ut::begin(sizes), ut::end(sizes), 5);

					// ACT
					s.layout(11, 15, p, _countof(p));

					// ASSERT
					layout_manager::position reference1[] = {
						{ 0, 0, 11, 13 },
						{ 0, 18, 11, 17 },
						{ 0, 40, 11, 121 },
						{ 0, 166, 11, 71 },
					};

					Assert::IsTrue(reference1[0] == p[0].second);
					Assert::IsTrue(reference1[1] == p[1].second);
					Assert::IsTrue(reference1[2] == p[2].second);
					Assert::IsTrue(reference1[3] == p[3].second);
				}


				[TestMethod]
				void LayoutHSingleWidgetRelativelySpaced()
				{
					// INIT
					int sizes1[] = { -10000 /* 100% */ };
					int sizes2[] = { -5750 /* 57.5% */ };
					layout_manager::widget_position p1[_countof(sizes1)];
					layout_manager::widget_position p2[_countof(sizes2)];
					hstack s1(ut::begin(sizes1), ut::end(sizes1), 0);
					hstack s2(ut::begin(sizes2), ut::end(sizes2), 0);

					// ACT
					s1.layout(11, 15, p1, _countof(p1));
					s2.layout(1103, 315, p2, _countof(p2));

					// ASSERT
					layout_manager::position reference[] = {
						{ 0, 0, 11, 15 },
						{ 0, 0, 634, 315 },
					};

					Assert::IsTrue(reference[0] == p1[0].second);
					Assert::IsTrue(reference[1] == p2[0].second);
				}
			};
		}
	}
}
