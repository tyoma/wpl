#include <wpl/ui/widget.h>
#include <wpl/ui/win32/native_widget.h>

#include "TestWidgets.h"

namespace std
{
	using tr1::bind;
	using tr1::ref;
   namespace placeholders
   {
      using namespace std::tr1::placeholders;
   }
}

using namespace Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace std;
using namespace std::placeholders;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace
			{
				struct myvisitor : widget_visitor
				{
					virtual void generic_widget_visited(widget &w)	{	visitation_log.push_back(make_pair(false, &w));	}
					virtual void native_widget_visited(native_widget &w)	{	visitation_log.push_back(make_pair(true, &w));	}

					vector< pair<bool, void *> > visitation_log;
				};

				struct dummy : widget
				{
				};
			}

			[TestClass]
			public ref class WidgetTests
			{
			public:
				[TestMethod]
				void VisitingDirectChildOfWidgetLeadsToGenericWidget()
				{
					// INIT
					myvisitor v1, v2;
					dummy d1, d2;

					// ACT
					d1.visit(v1);
					d2.visit(v2);
					d1.visit(v2);
					d2.visit(v1);
					d2.visit(v2);
						
					// ASSERT
					Assert::IsTrue(2 == v1.visitation_log.size());
					Assert::IsFalse(v1.visitation_log[0].first);
					Assert::IsTrue(&d1 == v1.visitation_log[0].second);
					Assert::IsFalse(v1.visitation_log[1].first);
					Assert::IsTrue(&d2 == v1.visitation_log[1].second);

					Assert::IsTrue(3 == v2.visitation_log.size());
					Assert::IsFalse(v2.visitation_log[0].first);
					Assert::IsTrue(&d2 == v2.visitation_log[0].second);
					Assert::IsFalse(v2.visitation_log[1].first);
					Assert::IsTrue(&d1 == v2.visitation_log[1].second);
					Assert::IsFalse(v2.visitation_log[2].first);
					Assert::IsTrue(&d2 == v2.visitation_log[2].second);
				}


				[TestMethod]
				void VisitingDirectChildOfNativeWidgetLeadsToNativeWidgetVisitation()
				{
					// INIT
					myvisitor v1, v2;
					ut::TestNativeWidget d1, d2;

					// ACT
					d1.visit(v1);
					d2.visit(v2);
					d1.visit(v2);
					d2.visit(v1);
					d2.visit(v2);
						
					// ASSERT
					Assert::IsTrue(2 == v1.visitation_log.size());
					Assert::IsTrue(v1.visitation_log[0].first);
					Assert::IsTrue(&d1 == v1.visitation_log[0].second);
					Assert::IsTrue(v1.visitation_log[1].first);
					Assert::IsTrue(&d2 == v1.visitation_log[1].second);

					Assert::IsTrue(3 == v2.visitation_log.size());
					Assert::IsTrue(v2.visitation_log[0].first);
					Assert::IsTrue(&d2 == v2.visitation_log[0].second);
					Assert::IsTrue(v2.visitation_log[1].first);
					Assert::IsTrue(&d1 == v2.visitation_log[1].second);
					Assert::IsTrue(v2.visitation_log[2].first);
					Assert::IsTrue(&d2 == v2.visitation_log[2].second);
				}
			};
		}
	}
}
