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

				void get_window_rect(HWND hwnd, RECT &rc)
				{
					HWND hparent = ::GetParent(hwnd);

					::GetWindowRect(hwnd, &rc);
					if (hparent)
						::MapWindowPoints(NULL, hparent, reinterpret_cast<POINT *>(&rc), 2);
				}
			}

			[TestClass]
			public ref class FormTests : ut::WindowTestsBase
			{
				typedef pair<shared_ptr<form>, HWND> form_and_handle;
				
				form_and_handle create_form_with_handle()
				{
					vector<void *> new_windows;
					set<void *> before(ut::enum_thread_windows());
					shared_ptr<form> f(create_form());
					set<void *> after(ut::enum_thread_windows());

					set_difference(after.begin(), after.end(), before.begin(), before.end(), back_inserter(new_windows));
					if (new_windows.size() != 1)
						throw runtime_error("Unexpected amount of windows created!");
					return make_pair(f, reinterpret_cast<HWND>(new_windows[0]));
				}

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
					set<void *> before(ut::enum_thread_windows());

					// ACT
					shared_ptr<form> f1 = create_form();

					// ASSERT
					set<void *> after(ut::enum_thread_windows());

					Assert::IsTrue(1 == ut::added_items(before, after));
					Assert::IsTrue(0 == ut::removed_items(before, after));

					// ACT
					shared_ptr<form> f2 = create_form();
					shared_ptr<form> f3 = create_form();

					// ASSERT
					set<void *> after2(ut::enum_thread_windows());

					Assert::IsTrue(2 == ut::added_items(after, after2));
					Assert::IsTrue(3 == ut::added_items(before, after2));
					Assert::IsTrue(0 == ut::removed_items(before, after2));
				}


				[TestMethod]
				void FormConstructionReturnsNonNullObject()
				{
					// INIT / ACT / ASSERT
					Assert::IsTrue(!!create_form());
				}


				[TestMethod]
				void FormDestructionDestroysItsWindow()
				{
					// INIT
					shared_ptr<form> f1 = create_form();
					shared_ptr<form> f2 = create_form();
					set<void *> before(ut::enum_thread_windows());

					// ACT
					f1 = shared_ptr<form>();
					set<void *> after1(ut::enum_thread_windows());
					f2 = shared_ptr<form>();
					set<void *> after2(ut::enum_thread_windows());

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
				void ResizingFormWindowRaisesRaisesResizeSignal()
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					vector< pair<unsigned int, unsigned int> > resize_log;
					slot_connection c(f.first->resized += bind(&track_resize, ref(resize_log), _1, _2));
					RECT rc;

					// ACT
					::MoveWindow(f.second, 0, 0, 117, 213, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					Assert::IsTrue(1 == resize_log.size());
					Assert::IsTrue(rc.right == (int)resize_log[0].first);
					Assert::IsTrue(rc.bottom == (int)resize_log[0].second);

					// ACT
					::MoveWindow(f.second, 27, 190, 531, 97, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					Assert::IsTrue(2 == resize_log.size());
					Assert::IsTrue(rc.right == (int)resize_log[1].first);
					Assert::IsTrue(rc.bottom == (int)resize_log[1].second);
				}


				[TestMethod]
				void MovingFormDoesNotRaiseResizeSignal()
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					vector< pair<unsigned int, unsigned int> > resize_log;

					::MoveWindow(f.second, 0, 0, 117, 213, TRUE);

					slot_connection c(f.first->resized += bind(&track_resize, ref(resize_log), _1, _2));

					// ACT
					::MoveWindow(f.second, 23, 91, 117, 213, TRUE);
					::MoveWindow(f.second, 53, 91, 117, 213, TRUE);
					::MoveWindow(f.second, 53, 32, 117, 213, TRUE);
					::MoveWindow(f.second, 23, 100, 117, 213, TRUE);

					// ASSERT
					Assert::IsTrue(resize_log.empty());
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
				void AddingNativeWidgetToFormChangesItsParent()
				{
					// INIT
					form_and_handle f[] = {
						create_form_with_handle(),
						create_form_with_handle(),
					};
					shared_ptr<ut::TestNativeWidget> widgets[] = {
						shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
						shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
						shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
					};

					// ACT
					f[0].first->add(widgets[0]);
					f[1].first->add(widgets[1]);

					// ASSERT
					Assert::IsTrue(f[0].second == ::GetParent(widgets[0]->hwnd()));
					Assert::IsTrue(f[1].second == ::GetParent(widgets[1]->hwnd()));
					Assert::IsTrue(NULL == ::GetParent(widgets[2]->hwnd()));

					// ACT
					f[1].first->add(widgets[2]);

					// ASSERT
					Assert::IsTrue(f[1].second == ::GetParent(widgets[2]->hwnd()));
				}


				[TestMethod]
				void AddingContainerWithNativeWidgetsToFormChangesTheirParent()
				{
					// INIT
					form_and_handle f[] = {
						create_form_with_handle(),
						create_form_with_handle(),
					};
					shared_ptr<layout_container> l[] = {
						shared_ptr<layout_container>(new layout_container),
						shared_ptr<layout_container>(new layout_container),
					};
					shared_ptr<ut::TestNativeWidget> widgets[] = {
						shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
						shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
					};

					l[0]->add(widgets[0]);
					l[1]->add(widgets[1]);

					// ACT
					f[0].first->add(l[0]);
					f[1].first->add(l[1]);

					// ASSERT
					Assert::IsTrue(f[0].second == ::GetParent(widgets[0]->hwnd()));
					Assert::IsTrue(f[1].second == ::GetParent(widgets[1]->hwnd()));
				}


				[TestMethod]
				void AddingNullWidgetThrowsArgumentInvalid()
				{
					// INIT
					form_and_handle f(create_form_with_handle());

					// ACT / ASSERT
					ASSERT_THROWS(f.first->add(shared_ptr<widget>()), invalid_argument);
				}


				[TestMethod]
				void NonNativeWidgetsCanBeAdded()
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr<ut::TestWidget> w(new ut::TestWidget);

					// ACT / ASSERT (must not throw)
					f.first->add(w);
				}


				[TestMethod]
				void MovingOfNativeWidgetOnForm()
				{
					// INIT
					shared_ptr<container> f(create_form());
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);
					shared_ptr<container::widget_site> site(f->add(w));
					RECT rc;

					// ACT
					site->move(0, 10, 20, 30);

					// ASSERT
					get_window_rect(w->hwnd(), rc);
					Assert::IsTrue(0 == rc.left);
					Assert::IsTrue(10 == rc.top);
					Assert::IsTrue(20 == rc.right);
					Assert::IsTrue(40 == rc.bottom);

					// ACT
					site->move(13, 17, 29, 47);

					// ASSERT
					get_window_rect(w->hwnd(), rc);
					Assert::IsTrue(13 == rc.left);
					Assert::IsTrue(17 == rc.top);
					Assert::IsTrue(42 == rc.right);
					Assert::IsTrue(64 == rc.bottom);
				}


				[TestMethod]
				void MovingNativeWidgetWithinLayoutContainerOnFormNoOffset()
				{
					// INIT
					shared_ptr<container> f(create_form());
					shared_ptr<layout_container> l1(new layout_container()), l2(new layout_container());
					shared_ptr<ut::TestNativeWidget> w1(new ut::TestNativeWidget), w2(new ut::TestNativeWidget);
					shared_ptr<container::widget_site> site1_l(f->add(l1)), site1_w(l1->add(w1));
					shared_ptr<container::widget_site> site2_w(l2->add(w2)), site2_l(f->add(l2));
					RECT rc;

					// ACT
					site2_l->move(0, 0, 0, 0);
					site1_w->move(0, 10, 20, 30);
					site2_w->move(0, 10, 20, 30);

					// ASSERT
					get_window_rect(w1->hwnd(), rc);
					Assert::IsTrue(0 == rc.left);
					Assert::IsTrue(10 == rc.top);
					Assert::IsTrue(20 == rc.right);
					Assert::IsTrue(40 == rc.bottom);

					get_window_rect(w2->hwnd(), rc);
					Assert::IsTrue(0 == rc.left);
					Assert::IsTrue(10 == rc.top);
					Assert::IsTrue(20 == rc.right);
					Assert::IsTrue(40 == rc.bottom);
				}


				[TestMethod]
				void MovingNativeWidgetWithinLayoutContainerOnFormWithOffset()
				{
					// INIT
					shared_ptr<container> f(create_form());
					shared_ptr<layout_container> l(new layout_container());
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);
					shared_ptr<container::widget_site> site_w(l->add(w)), site_l(f->add(l));
					RECT rc;

					// ACT
					site_l->move(11, 13, 0, 0);
					site_w->move(2, 3, 20, 30);

					// ASSERT
					get_window_rect(w->hwnd(), rc);
					Assert::IsTrue(13 == rc.left);
					Assert::IsTrue(16 == rc.top);
					Assert::IsTrue(33 == rc.right);
					Assert::IsTrue(46 == rc.bottom);

					// ACT
					site_l->move(29, 47, 0, 0);
					site_w->move(4, 5, 20, 30);

					// ASSERT
					get_window_rect(w->hwnd(), rc);
					Assert::IsTrue(33 == rc.left);
					Assert::IsTrue(52 == rc.top);
					Assert::IsTrue(53 == rc.right);
					Assert::IsTrue(82 == rc.bottom);
				}

			};
		}
	}
}
