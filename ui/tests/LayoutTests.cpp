#include <wpl/ui/layout.h>

#include "TestWidgets.h"

using namespace std;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace wpl
{
	namespace ui
	{
/*
		struct root
		{
		};

		struct view
		{
			struct visitor;
		};

		struct native_view : view
		{
		};

		struct widget
		{
			virtual shared_ptr<view> create_view(shared_ptr<root> root) = 0;
		};

		struct vlayout : widget
		{
			virtual shared_ptr<view> create_view(shared_ptr<root> root);
		};
		*/
		namespace tests
		{
			[TestClass]
			public ref class LayoutTests
			{
			public:
				[TestMethod, Ignore]
				void VStackIsWidget()
				{
					// INIT
					shared_ptr<vstack> vs;

					// ACT / ASSERT (must compile)
					shared_ptr<widget> w(vs);
				}


				[TestMethod]
				void AddingAWidgetDoesNotCreateItsView()
				{
					// INIT
					shared_ptr<vstack> s(new vstack);
					shared_ptr<ut::TestWidget> w(new ut::TestWidget);

					// ACT
					s->add(w, 146);
					s->add_proportional(w, 0.17);

					// ASSERT
					Assert::IsTrue(0 == w->views_created.size());
				}


				[TestMethod]
				void CreatingLayoutViewDoesCreateAViewForAnAddedWidget()
				{
					// INIT
					shared_ptr<vstack> s(new vstack);
					shared_ptr<ut::TestWidget> w[] = {
						shared_ptr<ut::TestWidget>(new ut::TestWidget),
						shared_ptr<ut::TestWidget>(new ut::TestWidget),
					};

					// ACT
					s->add(w[0], 117);

					s->create_view();

					// ASSERT
					Assert::IsTrue(1 == w[0]->views_created.size());
					Assert::IsTrue(0 == w[1]->views_created.size());

					// ACT
					s->add_proportional(w[1], 0.45);

					s->create_view();

					// ASSERT
					Assert::IsTrue(2 == w[0]->views_created.size());
					Assert::IsTrue(1 == w[1]->views_created.size());
				}


				[TestMethod]
				void CompositeViewHoldsChildren()
				{
					// INIT
					shared_ptr<vstack> s(new vstack);
					shared_ptr<ut::TestWidget> w[] = {
						shared_ptr<ut::TestWidget>(new ut::TestWidget),
						shared_ptr<ut::TestWidget>(new ut::TestWidget),
					};

					s->add(w[0], 117);

					// ACT
					shared_ptr<view> v1(s->create_view());

					// ASSERT
					Assert::IsTrue(1 == w[0]->views_created.size());
					Assert::IsFalse(w[0]->views_created[0].expired());
					Assert::IsTrue(0 == w[1]->views_created.size());

					// INIT
					s->add_proportional(w[1], 0.45);

					// ACT
					shared_ptr<view> v2(s->create_view());

					// ASSERT
					Assert::IsTrue(2 == w[0]->views_created.size());
					Assert::IsFalse(w[0]->views_created[0].expired());
					Assert::IsFalse(w[0]->views_created[1].expired());
					Assert::IsTrue(1 == w[1]->views_created.size());
					Assert::IsFalse(w[1]->views_created[0].expired());
				}


				[TestMethod]
				void CompositeViewReleasesChildrenOnDestroy()
				{
					// INIT
					shared_ptr<vstack> s(new vstack);
					shared_ptr<ut::TestWidget> w(new ut::TestWidget);

					s->add(w, 117);

					// ACT
					s->create_view();
					shared_ptr<view> v(s->create_view());

					// ASSERT
					Assert::IsTrue(w->views_created[0].expired());
					Assert::IsFalse(w->views_created[1].expired());
				}


				[TestMethod]
				void ResizingSingle100PercentChildMakesItFittingCompletely()
				{
					// INIT
					shared_ptr<vstack> s(new vstack);
					shared_ptr<ut::TestWidget> w(new ut::TestWidget);

					s->add_proportional(w, 1);

					shared_ptr<view> v(s->create_view());

					// ACT
					v->move(2, 3, 5, 7);

					// ASSERT
					Assert::IsTrue(1 == w->reposition_log.size());
					Assert::IsTrue(2 == w->reposition_log[0].left);
					Assert::IsTrue(3 == w->reposition_log[0].top);
					Assert::IsTrue(5 == w->reposition_log[0].width);
					Assert::IsTrue(7 == w->reposition_log[0].height);

					// ACT
					v->move(13224, 17322, 2332, 7222);

					// ASSERT
					Assert::IsTrue(2 == w->reposition_log.size());
					Assert::IsTrue(13224 == w->reposition_log[1].left);
					Assert::IsTrue(17322 == w->reposition_log[1].top);
					Assert::IsTrue(2332 == w->reposition_log[1].width);
					Assert::IsTrue(7222 == w->reposition_log[1].height);
				}


				[TestMethod]
				void ResizingSingleAbsoluteChildPreservesItsHeight()
				{
					// INIT
					shared_ptr<vstack> s(new vstack);
					shared_ptr<ut::TestWidget> w(new ut::TestWidget);

					s->add(w, 47);

					shared_ptr<view> v(s->create_view());

					// ACT
					v->move(2, 3, 5, 170);

					// ASSERT
					Assert::IsTrue(1 == w->reposition_log.size());
					Assert::IsTrue(2 == w->reposition_log[0].left);
					Assert::IsTrue(3 == w->reposition_log[0].top);
					Assert::IsTrue(5 == w->reposition_log[0].width);
					Assert::IsTrue(47 == w->reposition_log[0].height);

					// ACT
					v->move(11224, 1322, 2732, 722);

					// ASSERT
					Assert::IsTrue(2 == w->reposition_log.size());
					Assert::IsTrue(11224 == w->reposition_log[1].left);
					Assert::IsTrue(1322 == w->reposition_log[1].top);
					Assert::IsTrue(2732 == w->reposition_log[1].width);
					Assert::IsTrue(47 == w->reposition_log[1].height);
				}
			};
		}
	}
}
