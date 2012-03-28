#include <wpl/ui/widget.h>
#include <wpl/ui/geometry.h>
#include <wpl/ui/win32/native_view.h>

#include "TestWidgets.h"

namespace std
{
	using tr1::bind;
	using tr1::ref;
   namespace placeholders
   {
      using namespace tr1::placeholders;
   }
}

using namespace Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace
			{
				struct dummy_widget : widget
				{
				};
			}

			[TestClass]
			public ref class WidgetTests
			{
			public:
				[TestMethod]
				void CreatedJunctionProvideAccessToItsWidget()
				{
					// INIT
					shared_ptr<widget> w1(new ut::TestWidget), w2(new ut::TestWidget);

					// ACT
					view j1(w1), j2(w2);

					// ASSERT
					Assert::IsTrue(w1 == j1.widget);
					Assert::IsTrue(w2 == j2.widget);
				}


				[TestMethod]
				void CreatedViewContainsNonNullTransform()
				{
					// INIT
					view v(shared_ptr<widget>(new ut::TestWidget));

					// ACT / ASSERT
					Assert::IsTrue(!!v.transform());
					Assert::IsTrue(v.transform() == v.transform());
				}


				[TestMethod]
				void MovingAViewChangesItsTransformOrigin()
				{
					// INIT
					shared_ptr<widget> w(new ut::TestWidget);
					view v(w);
					int x = 13, y = 17;

					// ACT
					v.move(10, 11, 0, 0);
					v.transform()->map(x, y);

					// ASSERT
					Assert::IsTrue(3 == x);
					Assert::IsTrue(6 == y);

					// ACT
					v.move(3, 6, 1000, 1110);
					v.transform()->map(x, y);

					// ASSERT
					Assert::IsTrue(0 == x);
					Assert::IsTrue(0 == y);
				}


				[TestMethod]
				void GenericWidgetCreatesAView()
				{
					// INIT
					shared_ptr<widget> w(new widget);

					// ACT / ASSERT
					Assert::IsTrue(!!w->create_view(native_root(0)));
				}
			};
		}
	}
}
