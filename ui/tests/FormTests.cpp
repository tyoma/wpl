#include <wpl/ui/form.h>

#include <wpl/ui/layout.h>

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
				class logging_layout_manager : public layout_manager
				{
				public:
					mutable vector< pair<size_t, size_t> > reposition_log;

				private:
					virtual void layout(size_t width, size_t height, widget_position * /*widgets*/, size_t /*count*/) const
					{
						reposition_log.push_back(make_pair(width, height));
					}
				};
			}

			[TestClass]
			public ref class FormTests : ut::WindowTestsBase
			{
				typedef pair<shared_ptr<form>, HWND> form_and_handle;
				
				form_and_handle create_form_with_handle()
				{
					vector<HWND> new_windows;
					set<HWND> before(ut::enum_thread_windows());
					shared_ptr<form> f(form::create());
					set<HWND> after(ut::enum_thread_windows());

					set_difference(after.begin(), after.end(), before.begin(), before.end(), back_inserter(new_windows));
					if (new_windows.size() != 1)
						throw runtime_error("Unexpected amount of windows created!");
					return make_pair(f, new_windows[0]);
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
					set<HWND> before(ut::enum_thread_windows());

					// ACT
					shared_ptr<form> f1 = form::create();

					// ASSERT
					set<HWND> after(ut::enum_thread_windows());

					Assert::IsTrue(1 == ut::added_items(before, after));
					Assert::IsTrue(0 == ut::removed_items(before, after));

					// ACT
					shared_ptr<form> f2 = form::create();
					shared_ptr<form> f3 = form::create();

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
					Assert::IsTrue(!!form::create());
				}


				[TestMethod]
				void FormDestructionDestroysItsWindow()
				{
					// INIT
					shared_ptr<form> f1 = form::create();
					shared_ptr<form> f2 = form::create();
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
				void FormProvidesAValidContainer()
				{
					// INIT
					shared_ptr<form> f = form::create();

					// ACT / ASSERT
					Assert::IsTrue(!!f->get_root_container());
				}


				[TestMethod]
				void ResizingFormWindowLeadsToContentResize()
				{
					// INIT
					shared_ptr<logging_layout_manager> lm(new logging_layout_manager);
					form_and_handle f(create_form_with_handle());
					RECT rc;

					f.first->get_root_container()->layout = lm;

					// ACT
					::MoveWindow(f.second, 0, 0, 117, 213, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					Assert::IsTrue(1 == lm->reposition_log.size());
					Assert::IsTrue(rc.right == (int)lm->reposition_log[0].first);
					Assert::IsTrue(rc.bottom == (int)lm->reposition_log[0].second);

					// ACT
					::MoveWindow(f.second, 27, 190, 531, 97, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					Assert::IsTrue(2 == lm->reposition_log.size());
					Assert::IsTrue(rc.right == (int)lm->reposition_log[1].first);
					Assert::IsTrue(rc.bottom == (int)lm->reposition_log[1].second);
				}


				[TestMethod]
				void MovingFormDoesNotRaiseResizeSignal()
				{
					// INIT
					shared_ptr<logging_layout_manager> lm(new logging_layout_manager);
					form_and_handle f(create_form_with_handle());

					f.first->get_root_container()->layout = lm;

					::MoveWindow(f.second, 0, 0, 117, 213, TRUE);
					lm->reposition_log.clear();

					// ACT
					::MoveWindow(f.second, 23, 91, 117, 213, TRUE);
					::MoveWindow(f.second, 53, 91, 117, 213, TRUE);
					::MoveWindow(f.second, 53, 32, 117, 213, TRUE);
					::MoveWindow(f.second, 23, 100, 117, 213, TRUE);

					// ASSERT
					Assert::IsTrue(lm->reposition_log.empty());
				}


				[TestMethod]
				void ChildrenAreCreatedViaContainer()
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr<container> c = f.first->get_root_container();
					vector<HWND> w1, w2;

					// ACT
					set<HWND> p1(ut::enum_thread_windows(_T("SysListView32")));
					shared_ptr<widget> lv1 = c->create_widget(L"listview", L"1");
					set<HWND> p2(ut::enum_thread_windows(_T("SysListView32")));
					shared_ptr<widget> lv2 = c->create_widget(L"listview", L"2");
					set<HWND> p3(ut::enum_thread_windows(_T("SysListView32")));
					
					// ASSERT
					set_difference(p2.begin(), p2.end(), p1.begin(), p1.end(), back_inserter(w1));
					set_difference(p3.begin(), p3.end(), p2.begin(), p2.end(), back_inserter(w2));

					Assert::IsTrue(!!lv1);
					Assert::IsTrue(!!lv2);
					Assert::IsTrue(lv1 != lv2);

					Assert::IsTrue(1u == w1.size());
					Assert::IsTrue(1u == w2.size());

					Assert::IsTrue(f.second == ::GetParent(w1[0]));
					Assert::IsTrue(f.second == ::GetParent(w2[0]));
				}


				[TestMethod]
				void ChildrenAreHeldByForm()
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr<container> c = f.first->get_root_container();
					vector<HWND> w;
					shared_ptr<widget> lv1 = c->create_widget(L"listview", L"1");
					shared_ptr<widget> lv2 = c->create_widget(L"listview", L"2");
					set<HWND> p1(ut::enum_thread_windows(_T("SysListView32")));
					weak_ptr<widget> lv1_weak = lv1;
					weak_ptr<widget> lv2_weak = lv2;
					
					// ACT
					lv1 = shared_ptr<widget>();

					// ASSERT
					Assert::IsFalse(lv1_weak.expired());
					
					// ACT
					lv2 = shared_ptr<widget>();

					// ASSERT
					set<HWND> p2(ut::enum_thread_windows(_T("SysListView32")));

					set_difference(p2.begin(), p2.end(), p1.begin(), p1.end(), back_inserter(w));

					Assert::IsTrue(w.empty());
					Assert::IsFalse(lv2_weak.expired());
				}


				[TestMethod]
				void ChildrenDestroyedOnFormDestruction()
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr<widget> lv = f.first->get_root_container()->create_widget(L"listview", L"1");
					weak_ptr<widget> lv_weak = lv;

					lv = shared_ptr<widget>();

					// ACT
					f.first = shared_ptr<form>();

					// ASSERT
					Assert::IsTrue(lv_weak.expired());
				}


				//[TestMethod]
				//void CreationOfTestWidgetsCreatesWindows()
				//{
				//	// INIT
				//	shared_ptr<ut::TestNativeWidget> widgets[] = {
				//		shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
				//		shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
				//	};

				//	// ACT / ASSERT
				//	Assert::IsTrue(!!::IsWindow(widgets[0]->hwnd()));
				//	Assert::IsTrue(!!::IsWindow(widgets[1]->hwnd()));
				//	Assert::IsTrue(widgets[1]->hwnd() != widgets[0]->hwnd());
				//}


				//[TestMethod]
				//void WrappingNativeWidgetToFormChangesItsParent()
				//{
				//	// INIT
				//	shared_ptr<ut::TestNativeWidget> widgets[] = {
				//		shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
				//		shared_ptr<ut::TestNativeWidget>(new ut::TestNativeWidget()),
				//	};

				//	// ACT
				//	form_and_handle f1(create_form_with_handle(widgets[0]));

				//	// ASSERT
				//	Assert::IsTrue(f1.second == ::GetParent(widgets[0]->hwnd()));
				//	Assert::IsTrue(NULL == ::GetParent(widgets[1]->hwnd()));

				//	// ACT
				//	form_and_handle f2(create_form_with_handle(widgets[1]));

				//	// ASSERT
				//	Assert::IsTrue(f2.second == ::GetParent(widgets[1]->hwnd()));
				//}
			};
		}
	}
}
