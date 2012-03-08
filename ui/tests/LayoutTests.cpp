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
					s->add(w);

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
						shared_ptr<ut::TestWidget>(new ut::TestWidget),
					};

					// ACT
					s->add(w[0], 117);
					s->add_proportional(w[1], 0.45);
					s->add(w[2]);

					// ASSERT
					Assert::IsTrue(1 == w[0]->views_created.size());
					Assert::IsTrue(1 == w[1]->views_created.size());
					Assert::IsTrue(1 == w[2]->views_created.size());
				}
			};
		}
	}
}
