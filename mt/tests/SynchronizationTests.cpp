#include <synchronization.h>

using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace wpl
{
	namespace mt
	{
		namespace tests
		{
			[TestClass]
			public ref class SynchronizationTests
			{
			public:
				[TestMethod]
				void EventFlagCreateRaised()
				{
					// INIT
					event_flag e(true, false);

					// ACT / ASSERT
					Assert::IsTrue(waitable::satisfied == e.wait(0));
					Assert::IsTrue(waitable::satisfied == e.wait(10000));
					Assert::IsTrue(waitable::satisfied == e.wait(waitable::infinite));
				}


				[TestMethod]
				void EventFlagCreateLowered()
				{
					// INIT
					event_flag e(false, false);

					// ACT / ASSERT
					Assert::IsTrue(waitable::timeout == e.wait(0));
					Assert::IsTrue(waitable::timeout == e.wait(200));
				}


				[TestMethod]
				void EventFlagCreateAutoResettable()
				{
					// INIT
					event_flag e(true, true);

					e.wait(100);

					// ACT / ASSERT
					Assert::IsTrue(waitable::timeout == e.wait(100));
				}


				[TestMethod]
				void RaisingEventFlag()
				{
					// INIT
					event_flag e(false, false);

					// ACT
					e.raise();

					// ACT / ASSERT
					Assert::IsTrue(waitable::satisfied == e.wait());
				}


				[TestMethod]
				void LoweringEventFlag()
				{
					// INIT
					event_flag e(true, false);

					// ACT
					e.lower();

					// ACT / ASSERT
					Assert::IsTrue(waitable::timeout == e.wait(0));
				}
			};
		}
	}
}
