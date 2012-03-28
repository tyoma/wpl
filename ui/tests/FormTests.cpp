#include <wpl/ui/form.h>
#include <wpl/ui/layout.h>

#include <wpl/ui/win32/containers.h>

#include "TestHelpers.h"
#include "TestWidgets.h"

#include <windows.h>

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
				void track_resize(vector< pair<unsigned int, unsigned int> > &resizes, unsigned int width, unsigned int height)
				{	resizes.push_back(make_pair(width, height));	}
			}

			[TestClass]
			public ref class FormTests : ut::WindowTestsBase
			{
				typedef pair<shared_ptr<form>, HWND> form_and_handle;
				
				form_and_handle create_form_with_handle(shared_ptr<widget> widget)
				{
					vector<HWND> new_windows;
					set<HWND> before(ut::enum_thread_windows());
					shared_ptr<form> f(create_form(widget));
					set<HWND> after(ut::enum_thread_windows());

					set_difference(after.begin(), after.end(), before.begin(), before.end(), back_inserter(new_windows));
					if (new_windows.size() != 1)
						throw runtime_error("Unexpected amount of windows created!");
					return make_pair(f, new_windows[0]);
				}

				form_and_handle create_form_with_handle()
				{	return create_form_with_handle(shared_ptr<widget>(new ut::TestWidget));	}

			public:
				[TestInitialize]
				void create_dummy_window()
				{
					create_window();
				}


				[TestMethod]
				void FormWindowIsCreatedAtFormConstruction()
				{
					// INIT
					set<HWND> before(ut::enum_thread_windows());

					// ACT
					shared_ptr<form> f1 = create_form(shared_ptr<widget>(new ut::TestWidget));

					// ASSERT
					set<HWND> after(ut::enum_thread_windows());

					Assert::IsTrue(1 == ut::added_items(before, after));
					Assert::IsTrue(0 == ut::removed_items(before, after));

					// ACT
					shared_ptr<form> f2 = create_form(shared_ptr<widget>(new ut::TestWidget));
					shared_ptr<form> f3 = create_form(shared_ptr<widget>(new ut::TestWidget));

					// ASSERT
					set<HWND> after2(ut::enum_thread_windows());

					Assert::IsTrue(2 == ut::added_items(after, after2));
					Assert::IsTrue(3 == ut::added_items(before, after2));
					Assert::IsTrue(0 == ut::removed_items(before, after2));
				}


				[TestMethod]
				void FormConstructionReturnsNonNullObject()
				{
					// INIT / ACT / ASSERT
					Assert::IsTrue(!!create_form(shared_ptr<widget>(new ut::TestWidget)));
				}


				[TestMethod]
				void FormDestructionDestroysItsWindow()
				{
					// INIT
					shared_ptr<form> f1 = create_form(shared_ptr<widget>(new ut::TestWidget));
					shared_ptr<form> f2 = create_form(shared_ptr<widget>(new ut::TestWidget));
					set<HWND> before(ut::enum_thread_windows());

					// ACT
					f1 = shared_ptr<form>();
					set<HWND> after1(ut::enum_thread_windows());
					f2 = shared_ptr<form>();
					set<HWND> after2(ut::enum_thread_windows());

					// ASSERT
					Assert::IsTrue(1 == ut::removed_items(before, after1));
					Assert::IsTrue(1 == ut::removed_items(after1, after2));
					Assert::IsTrue(2 == ut::removed_items(before, after2));
					Assert::IsTrue(0 == ut::added_items(before, after2));
				}


				[TestMethod]
				void FormWindowIsHasPopupStyleAndInvisibleAtConstruction()
				{
					// INIT / ACT
					form_and_handle f(create_form_with_handle());

					// ASSERT
					DWORD style = ::GetWindowLong(f.second, GWL_STYLE);

					Assert::IsFalse(!!(WS_VISIBLE & style));
					Assert::IsTrue(!!(WS_THICKFRAME & style));
					Assert::IsTrue(!!(WS_CAPTION & style));
					Assert::IsTrue(!!(WS_CLIPCHILDREN & style));
				}


				[TestMethod]
				void ChangingFormVisibilityAffectsItsWindowVisibility()
				{
					// INIT
					form_and_handle f(create_form_with_handle());

					// ACT
					f.first->set_visible(true);

					// ASSERT
					Assert::IsTrue(!!(WS_VISIBLE & ::GetWindowLong(f.second, GWL_STYLE)));

					// ACT
					f.first->set_visible(false);

					// ASSERT
					Assert::IsFalse(!!(WS_VISIBLE & ::GetWindowLong(f.second, GWL_STYLE)));
				}


				[TestMethod]
				void CreatingFormFromWidgetAcquiresItView()
				{
					// INIT
					shared_ptr<ut::TestWidget> w(new ut::TestWidget);
					
					// ACT
					shared_ptr<form> f1(create_form(w));

					// ASSERT
					Assert::IsTrue(w->views_created.size() == 1);
					Assert::IsFalse(w->views_created[0].expired());

					// ACT
					shared_ptr<form> f2(create_form(w));

					// ASSERT
					Assert::IsTrue(w->views_created.size() == 2);
				}


				[TestMethod]
				void ResizingFormWindowLeadsToContentResize()
				{
					// INIT
					shared_ptr<ut::TestWidget> w(new ut::TestWidget);
					form_and_handle f(create_form_with_handle(w));
					RECT rc;

					// ACT
					::MoveWindow(f.second, 0, 0, 117, 213, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					Assert::IsTrue(1 == w->reposition_log.size());
					Assert::IsTrue(rc.right == (int)w->reposition_log[0].width);
					Assert::IsTrue(rc.bottom == (int)w->reposition_log[0].height);

					// ACT
					::MoveWindow(f.second, 27, 190, 531, 97, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					Assert::IsTrue(2 == w->reposition_log.size());
					Assert::IsTrue(rc.right == (int)w->reposition_log[1].width);
					Assert::IsTrue(rc.bottom == (int)w->reposition_log[1].height);
				}


				[TestMethod]
				void MovingFormDoesNotRaiseResizeSignal()
				{
					// INIT
					shared_ptr<ut::TestWidget> w(new ut::TestWidget);
					form_and_handle f(create_form_with_handle(w));

					::MoveWindow(f.second, 0, 0, 117, 213, TRUE);
					w->reposition_log.clear();

					// ACT
					::MoveWindow(f.second, 23, 91, 117, 213, TRUE);
					::MoveWindow(f.second, 53, 91, 117, 213, TRUE);
					::MoveWindow(f.second, 53, 32, 117, 213, TRUE);
					::MoveWindow(f.second, 23, 100, 117, 213, TRUE);

					// ASSERT
					Assert::IsTrue(w->reposition_log.empty());
				}


				[TestMethod]
				void CreationOfTestWidgetsCreatesWindows()
				{
					// INIT
					shared_ptr<ut::TestNativeWidget> widgets[] = {
						shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
						shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
					};

					// ACT / ASSERT
					Assert::IsTrue(!!::IsWindow(widgets[0]->hwnd()));
					Assert::IsTrue(!!::IsWindow(widgets[1]->hwnd()));
					Assert::IsTrue(widgets[1]->hwnd() != widgets[0]->hwnd());
				}


				[TestMethod]
				void WrappingNativeWidgetToFormChangesItsParent()
				{
					// INIT
					shared_ptr<ut::TestNativeWidget> widgets[] = {
						shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
						shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
					};

					// ACT
					form_and_handle f1(create_form_with_handle(widgets[0]));

					// ASSERT
					Assert::IsTrue(f1.second == ::GetParent(widgets[0]->hwnd()));
					Assert::IsTrue(NULL == ::GetParent(widgets[1]->hwnd()));

					// ACT
					form_and_handle f2(create_form_with_handle(widgets[1]));

					// ASSERT
					Assert::IsTrue(f2.second == ::GetParent(widgets[1]->hwnd()));
				}

/*
				[TestMethod]
				void MovingNativeWidgetWithinLayoutContainerOnFormNoOffset()
				{
					// INIT
					shared_ptr<container> f(create_form());
					shared_ptr<composition> l1(new composition()), l2(new composition());
					shared_ptr<ut::TestNativeWidget> w1(new ut::TestNativeWidget), w2(new ut::TestNativeWidget);
					shared_ptr<view> site1_w(l1->add(w1)), site1_l(f->add(l1));
					shared_ptr<view> site2_w(l2->add(w2)), site2_l(f->add(l2));
					RECT rc;

					// ACT
					site2_l->move(0, 0, 0, 0);
					site1_w->move(0, 10, 20, 30);
					site2_w->move(31, 41, 50, 60);

					// ASSERT
					rc = ut::get_window_rect(w1->hwnd());

					Assert::IsTrue(0 == rc.left);
					Assert::IsTrue(10 == rc.top);
					Assert::IsTrue(20 == rc.right);
					Assert::IsTrue(40 == rc.bottom);

					rc = ut::get_window_rect(w2->hwnd());

					Assert::IsTrue(31 == rc.left);
					Assert::IsTrue(41 == rc.top);
					Assert::IsTrue(81 == rc.right);
					Assert::IsTrue(101 == rc.bottom);
				}


				[TestMethod]
				void MovingNativeWidgetWithinLayoutContainerOnFormWithOffset()
				{
					// INIT
					shared_ptr<container> f(create_form());
					shared_ptr<composition> l(new composition());
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);
					shared_ptr<view> site_w(l->add(w)), site_l(f->add(l));
					RECT rc;

					// ACT
					site_l->move(11, 13, 0, 0);
					site_w->move(2, 3, 20, 30);

					// ASSERT
					rc = ut::get_window_rect(w->hwnd());

					Assert::IsTrue(13 == rc.left);
					Assert::IsTrue(16 == rc.top);
					Assert::IsTrue(33 == rc.right);
					Assert::IsTrue(46 == rc.bottom);

					// ACT
					site_l->move(29, 47, 0, 0);
					site_w->move(4, 5, 20, 30);

					// ASSERT
					rc = ut::get_window_rect(w->hwnd());

					Assert::IsTrue(33 == rc.left);
					Assert::IsTrue(52 == rc.top);
					Assert::IsTrue(53 == rc.right);
					Assert::IsTrue(82 == rc.bottom);
				}
				*/


				[TestMethod]
				void AddingContainerWithNativeWidgetsToFormChangesTheirParent()
				{
					// INIT
					shared_ptr<vstack> l[] = {
						shared_ptr<vstack>(new vstack),
						shared_ptr<vstack>(new vstack),
					};
					shared_ptr<ut::TestNativeWidget> widgets[] = {
						shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
						shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
					};

					l[0]->add(widgets[0], 10);
					l[1]->add_proportional(widgets[1], 0.2);

					// ACT
					form_and_handle f[] = {
						create_form_with_handle(l[0]),
						create_form_with_handle(l[1]),
					};

					// ASSERT
					Assert::IsTrue(f[0].second == ::GetParent(widgets[0]->hwnd()));
					Assert::IsTrue(f[1].second == ::GetParent(widgets[1]->hwnd()));
				}
			};
		}
	}
}
