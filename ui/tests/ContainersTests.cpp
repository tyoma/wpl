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


				[TestMethod, Ignore]
				void CompositionIsWidget()
				{
					// INIT / ACT / ASSERT
					shared_ptr<widget> lc(new composition());
				}


				[TestMethod, Ignore]
				void CompositionIsContainer()
				{
					// INIT / ACT / ASSERT
					shared_ptr<container> lc(new composition());
				}


				[TestMethod]
				void CompositionCanBeNested()
				{
					// INIT
					shared_ptr<composition> containers[] = {
						shared_ptr<composition>(new composition()),
						shared_ptr<composition>(new composition()),
						shared_ptr<composition>(new composition()),
						shared_ptr<composition>(new composition()),
						shared_ptr<composition>(new composition()),
					};
					container::children_list c[5];

					// ACT
					containers[0]->add(containers[1]);
					containers[0]->add(containers[2]);
					containers[2]->add(containers[3]);
					containers[4]->add(containers[0]);

					containers[0]->get_children(c[0]);
					containers[1]->get_children(c[1]);
					containers[2]->get_children(c[2]);
					containers[3]->get_children(c[3]);
					containers[4]->get_children(c[4]);

					// ASSERT
					Assert::IsTrue(2 == c[0].size());
					Assert::IsTrue(0 == c[1].size());
					Assert::IsTrue(1 == c[2].size());
					Assert::IsTrue(0 == c[3].size());
					Assert::IsTrue(1 == c[4].size());

					Assert::IsTrue(containers[1] == c[0][0]->widget);
					Assert::IsTrue(containers[2] == c[0][1]->widget);
					Assert::IsTrue(containers[0] == c[4][0]->widget);
				}


				[TestMethod]
				void CompositionIsVisitedAsContainer()
				{
					// INIT
					ut::NodesVisitationChecker v;
					shared_ptr<composition> l(new composition());
					void *proper_container = (container *)l.get();

					// ACT
					l->visit(v);
					((container *)l.get())->visit(v);
					((widget *)l.get())->visit(v);
					((node *)(container *)l.get())->visit(v);
					((node *)(widget *)l.get())->visit(v);

					// ASSERT
					Assert::IsTrue(5 == v.visitation_log.size());
					Assert::IsTrue(v.visitation_log[0].first);
					Assert::IsTrue(proper_container == v.visitation_log[0].second);
					Assert::IsTrue(v.visitation_log[1].first);
					Assert::IsTrue(proper_container == v.visitation_log[1].second);
					Assert::IsTrue(v.visitation_log[2].first);
					Assert::IsTrue(proper_container == v.visitation_log[2].second);
					Assert::IsTrue(v.visitation_log[3].first);
					Assert::IsTrue(proper_container == v.visitation_log[3].second);
					Assert::IsTrue(v.visitation_log[4].first);
					Assert::IsTrue(proper_container == v.visitation_log[4].second);
				}
			};
		}
	}
}
