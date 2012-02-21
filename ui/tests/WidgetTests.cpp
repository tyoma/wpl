#include <wpl/ui/widget.h>
#include <wpl/ui/geometry.h>

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

				struct dummy_container : container
				{
					virtual shared_ptr<view> add(shared_ptr<widget> /*widget*/)	{	throw 0;	}
					virtual void get_children(children_list &/*children*/) const	{	}
				};
			}

			[TestClass]
			public ref class WidgetTests
			{
			public:
				[TestMethod]
				void VisitingGenericViewLeadsToGenericView()
				{
					// INIT
					ut::ViewVisitationChecker v1, v2;
					shared_ptr<widget> w(new ut::TestWidget);
					view view1(w), view2(w);

					// ACT
					view1.visit(v1);
					view2.visit(v2);
					view1.visit(v2);
					view2.visit(v1);
					view2.visit(v2);
						
					// ASSERT
					Assert::IsTrue(2 == v1.visitation_log.size());
					Assert::IsFalse(v1.visitation_log[0].first);
					Assert::IsTrue(&view1 == v1.visitation_log[0].second);
					Assert::IsFalse(v1.visitation_log[1].first);
					Assert::IsTrue(&view2 == v1.visitation_log[1].second);

					Assert::IsTrue(3 == v2.visitation_log.size());
					Assert::IsFalse(v2.visitation_log[0].first);
					Assert::IsTrue(&view2 == v2.visitation_log[0].second);
					Assert::IsFalse(v2.visitation_log[1].first);
					Assert::IsTrue(&view1 == v2.visitation_log[1].second);
					Assert::IsFalse(v2.visitation_log[2].first);
					Assert::IsTrue(&view2 == v2.visitation_log[2].second);
				}


				[TestMethod]
				void NodeVisitingDirectChildOfWidgetLeadsToWidget()
				{
					// INIT
					ut::NodesVisitationChecker v;
					dummy_widget d1, d2;

					// ACT
					d1.visit(v);
					d2.visit(v);
					d1.visit(v);
						
					// ASSERT
					Assert::IsTrue(3 == v.visitation_log.size());
					Assert::IsFalse(v.visitation_log[0].first);
					Assert::IsTrue(&d1 == v.visitation_log[0].second);
					Assert::IsFalse(v.visitation_log[1].first);
					Assert::IsTrue(&d2 == v.visitation_log[1].second);
					Assert::IsFalse(v.visitation_log[2].first);
					Assert::IsTrue(&d1 == v.visitation_log[2].second);
				}


				[TestMethod]
				void NodeVisitingDirectChildOfContainerLeadsToContainer()
				{
					// INIT
					ut::NodesVisitationChecker v;
					dummy_container d1, d2;

					// ACT
					d1.visit(v);
					d1.visit(v);
					d2.visit(v);
						
					// ASSERT
					Assert::IsTrue(3 == v.visitation_log.size());
					Assert::IsTrue(v.visitation_log[0].first);
					Assert::IsTrue(&d1 == v.visitation_log[0].second);
					Assert::IsTrue(v.visitation_log[1].first);
					Assert::IsTrue(&d1 == v.visitation_log[1].second);
					Assert::IsTrue(v.visitation_log[2].first);
					Assert::IsTrue(&d2 == v.visitation_log[2].second);
				}


				[TestMethod]
				void VisitingViewOfATestNativeWidgetLeadsToNativeViewVisitation()
				{
					// INIT
					ut::ViewVisitationChecker v1, v2;
					shared_ptr<widget> nw1(new ut::TestNativeWidget), nw2(new ut::TestNativeWidget);
					shared_ptr<view> view1(nw1->create_view()), view2(nw2->create_view());

					// ACT
					view1->visit(v1);
					view2->visit(v2);
					view1->visit(v2);
					view2->visit(v1);
					view2->visit(v2);
						
					// ASSERT
					Assert::IsTrue(2 == v1.visitation_log.size());
					Assert::IsTrue(v1.visitation_log[0].first);
					Assert::IsTrue(&*view1 == v1.visitation_log[0].second);
					Assert::IsTrue(v1.visitation_log[1].first);
					Assert::IsTrue(&*view2 == v1.visitation_log[1].second);

					Assert::IsTrue(3 == v2.visitation_log.size());
					Assert::IsTrue(v2.visitation_log[0].first);
					Assert::IsTrue(&*view2 == v2.visitation_log[0].second);
					Assert::IsTrue(v2.visitation_log[1].first);
					Assert::IsTrue(&*view1 == v2.visitation_log[1].second);
					Assert::IsTrue(v2.visitation_log[2].first);
					Assert::IsTrue(&*view2 == v2.visitation_log[2].second);
				}


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
					Assert::IsTrue(!!w->create_view());
				}


				[TestMethod]
				void GenericWidgetCreatesGenericView()
				{
					// INIT
					shared_ptr<widget> w(new widget);
					ut::ViewVisitationChecker v;
					
					// ACT
					w->create_view()->visit(v);

					// ASSERT
					Assert::IsTrue(1 == v.visitation_log.size());
					Assert::IsFalse(v.visitation_log[0].first);
				}
			};
		}
	}
}
