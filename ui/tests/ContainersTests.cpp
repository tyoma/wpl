#include <wpl/ui/form.h>

#include <wpl/ui/win32/containers.h>

#include "TestHelpers.h"
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
			[TestClass]
			public ref class ContainersTests
			{
			public:
				[TestMethod]
				void GettingChildrenFromEmptyContainerLeadsToEmptyList()
				{
					// INIT
					shared_ptr<container> container(new container);
					vector< shared_ptr<view> > children(1);	// must be emptied

					// ACT
					container->get_children(children);

					// ASSERT
					Assert::IsTrue(children.empty());
				}


				[TestMethod]
				void AddedChildrenAreReturnedInAdditionOrder()
				{
					// INIT
					vector< shared_ptr<view> > c1(1), c2;
					shared_ptr<container> containers[] = {
						shared_ptr<container>(new container),
						shared_ptr<container>(new container),
					};
					shared_ptr<widget> widgets[] = {
						shared_ptr<widget>(new ut::TestWidget()),
						shared_ptr<widget>(new ut::TestNativeWidget()),
						shared_ptr<widget>(new ut::TestWidget()),
						shared_ptr<widget>(new ut::TestNativeWidget()),
						shared_ptr<widget>(new ut::TestNativeWidget()),
					};

					containers[0]->add(widgets[0]);
					containers[0]->add(widgets[1]);
					containers[0]->add(widgets[2]);
					containers[1]->add(widgets[3]);

					// ACT
					containers[0]->get_children(c1);
					containers[1]->get_children(c2);

					// ASSERT
					Assert::IsTrue(3 == c1.size());
					Assert::IsTrue(1 == c2.size());
					Assert::IsTrue(widgets[0] == c1[0]->widget);
					Assert::IsTrue(widgets[1] == c1[1]->widget);
					Assert::IsTrue(widgets[2] == c1[2]->widget);
					Assert::IsTrue(widgets[3] == c2[0]->widget);

					// INIT
					containers[1]->add(widgets[4]);

					// ACT
					containers[1]->get_children(c2);

					// ASSERT
					Assert::IsTrue(2 == c2.size());
					Assert::IsTrue(widgets[3] == c2[0]->widget);
					Assert::IsTrue(widgets[4] == c2[1]->widget);
				}


				[TestMethod]
				void SiteIsReturnedWhenAddingNativeWidgetToForm()
				{
					// INIT
					shared_ptr<container> container(new container);
					shared_ptr<widget> widget(new ut::TestNativeWidget());

					// ACT / ASSERT
					Assert::IsTrue(!!container->add(widget));
				}


				[TestMethod]
				void SiteIsReturnedWhenAddingGenericWidgetToForm()
				{
					// INIT
					shared_ptr<container> container(create_form());
					shared_ptr<widget> widget(new ut::TestWidget());

					// ACT / ASSERT
					Assert::IsTrue(!!container->add(widget));
				}


				[TestMethod]
				void ViewsOfWidgetsAddedToContainerAreSameToTheChildrenViews()
				{
					// INIT
					vector< shared_ptr<view> > c1, c2;
					shared_ptr<container> containers[] = {
						shared_ptr<container>(new container),
						shared_ptr<container>(new container),
					};
					shared_ptr<widget> widgets[] = {
						shared_ptr<widget>(new ut::TestWidget()),
						shared_ptr<widget>(new ut::TestNativeWidget()),
						shared_ptr<widget>(new ut::TestWidget()),
						shared_ptr<widget>(new ut::TestNativeWidget()),
						shared_ptr<widget>(new ut::TestNativeWidget()),
					};

					// ACT
					shared_ptr<view> added_views[] = {
						containers[0]->add(widgets[0]),
						containers[0]->add(widgets[1]),
						containers[0]->add(widgets[2]),
						containers[1]->add(widgets[3]),
						containers[1]->add(widgets[4]),
					};
					containers[0]->get_children(c1);
					containers[1]->get_children(c2);

					// ASSERT
					Assert::IsTrue(added_views[0] == c1[0]);
					Assert::IsTrue(added_views[1] == c1[1]);
					Assert::IsTrue(added_views[2] == c1[2]);
					Assert::IsTrue(added_views[3] == c2[0]);
					Assert::IsTrue(added_views[4] == c2[1]);
				}


				[TestMethod]
				void CustomViewIsCreatedIfSupported()
				{
					// INIT
					shared_ptr<container> container(new container);
					shared_ptr<widget> widget(new ut::TestNativeWidget());

					// ACT
					shared_ptr<view> added_view = container->add(widget);

					// ASSERT
					ut::ViewVisitationChecker v;

					added_view->visit(v);

					Assert::IsTrue(1 == v.visitation_log.size());
					Assert::IsTrue(v.visitation_log[0].first);
					Assert::IsTrue(&*added_view == v.visitation_log[0].second);
				}


				[TestMethod]
				void AddingNullWidgetThrowsArgumentInvalid()
				{
					// INIT
					shared_ptr<container> container(new container);

					// ACT / ASSERT
					ASSERT_THROWS(container->add(shared_ptr<widget>()), invalid_argument);
				}
			};
		}
	}
}
