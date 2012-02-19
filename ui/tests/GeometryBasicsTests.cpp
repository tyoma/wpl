#include <wpl/ui/geometry.h>

using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			[TestClass]
			public ref class GeometryBasicsTests
			{
			public: 
				[TestMethod]
				void CreatedTransformMakesNoChangesInMap()
				{
					// INIT
					transform t;
					int x1 = 1, y1 = 3, x2 = 5, y2 = 7;

					// ACT
					t.map(x1, y1);
					t.map(x2, y2);

					// ASSERT
					Assert::IsTrue(x1 == 1);
					Assert::IsTrue(y1 == 3);
					Assert::IsTrue(x2 == 5);
					Assert::IsTrue(y2 == 7);
				}


				[TestMethod]
				void CreatedTransformMakesNoChangesInUnmap()
				{
					// INIT
					transform t;
					int x1 = 7, y1 = 5, x2 = 3, y2 = 1;

					// ACT
					t.unmap(x1, y1);
					t.unmap(x2, y2);

					// ASSERT
					Assert::IsTrue(x1 == 7);
					Assert::IsTrue(y1 == 5);
					Assert::IsTrue(x2 == 3);
					Assert::IsTrue(y2 == 1);
				}


				[TestMethod]
				void MapPointAccordinglyToOrigin()
				{
					// INIT
					transform t;
					int x = 7, y = 5;

					// ACT
					t.set_origin(19, 3);
					t.map(x, y);

					// ASSERT
					Assert::IsTrue(x == -12);
					Assert::IsTrue(y == 2);

					// ACT
					t.set_origin(-12, 1);
					t.map(x, y);

					// ASSERT
					Assert::IsTrue(x == 0);
					Assert::IsTrue(y == 1);
				}


				[TestMethod]
				void UnmapPointAccordinglyToOrigin()
				{
					// INIT
					transform t;
					int x = 7, y = 5;

					// ACT
					t.set_origin(11, 7);
					t.unmap(x, y);

					// ASSERT
					Assert::IsTrue(x == 18);
					Assert::IsTrue(y == 12);

					// ACT
					t.set_origin(-12, 1);
					t.unmap(x, y);

					// ASSERT
					Assert::IsTrue(x == 6);
					Assert::IsTrue(y == 13);
				}
			};
		}
	}
}
