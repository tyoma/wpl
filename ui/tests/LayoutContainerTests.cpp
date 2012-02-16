#include <wpl/ui/layout.h>

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
			public ref class LayoutContainerTests
			{
			public:
				[TestMethod, Ignore]
				void LayoutContainerIsWidget()
				{
					// INIT / ACT / ASSERT
					shared_ptr<widget> lc(new layout_container());
				}


				[TestMethod, Ignore]
				void LayoutContainerIsContainer()
				{
					// INIT / ACT / ASSERT
					shared_ptr<container> lc(new layout_container());
				}


				[TestMethod]
				void LayoutContainerCanBeNested()
				{
					// INIT
					shared_ptr<layout_container> containers[] = {
						shared_ptr<layout_container>(new layout_container()),
						shared_ptr<layout_container>(new layout_container()),
						shared_ptr<layout_container>(new layout_container()),
						shared_ptr<layout_container>(new layout_container()),
						shared_ptr<layout_container>(new layout_container()),
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

					Assert::IsTrue(containers[1] == c[0][0]);
					Assert::IsTrue(containers[2] == c[0][1]);
					Assert::IsTrue(containers[0] == c[4][0]);
				}


				[TestMethod]
				void LayoutContainerIsVisitedAsContainer()
				{
					// INIT
					ut::NodesVisitationChecker v;
					shared_ptr<layout_container> l(new layout_container());
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
