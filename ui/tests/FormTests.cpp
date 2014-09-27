#include <wpl/ui/form.h>

#include <wpl/ui/layout.h>

#include "Mockups.h"
#include "TestHelpers.h"

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
				void increment(int *counter)
				{	++*counter;	}
			}

			typedef pair<shared_ptr<form>, HWND> form_and_handle;

			form_and_handle create_form_with_handle()
			{
				ut::window_tracker wt(L"#32770");
				shared_ptr<form> f(form::create());

				wt.checkpoint();

				if (wt.created.size() != 1)
					throw runtime_error("Unexpected amount of windows created!");
				return make_pair(f, wt.created[0]);
			}

			[TestClass]
			public ref class FormTests : ut::WindowTestsBase
			{
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
					ut::window_tracker wt(L"#32770");

					// ACT
					shared_ptr<form> f1 = form::create();

					// ASSERT
					wt.checkpoint();

					Assert::IsTrue(1 == wt.created.size());
					Assert::IsTrue(0 == wt.destroyed.size());

					// ACT
					shared_ptr<form> f2 = form::create();
					shared_ptr<form> f3 = form::create();

					// ASSERT
					wt.checkpoint();

					Assert::IsTrue(3 == wt.created.size());
					Assert::IsTrue(0 == wt.destroyed.size());
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
					ut::window_tracker wt(L"#32770");

					// ACT
					f1 = shared_ptr<form>();

					// ASSERT
					wt.checkpoint();

					Assert::IsTrue(0 == wt.created.size());
					Assert::IsTrue(1 == wt.destroyed.size());

					// ACT
					f2 = shared_ptr<form>();

					// ASSERT
					wt.checkpoint();

					Assert::IsTrue(0 == wt.created.size());
					Assert::IsTrue(2 == wt.destroyed.size());
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
					shared_ptr<ut::logging_layout_manager> lm(new ut::logging_layout_manager);
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
					shared_ptr<ut::logging_layout_manager> lm(new ut::logging_layout_manager);
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
					ut::window_tracker wt(L"SysListView32");

					// ACT
					shared_ptr<widget> lv1 = c->create_widget(L"listview", L"1");
					shared_ptr<widget> lv2 = c->create_widget(L"listview", L"2");
					wt.checkpoint();
					
					// ASSERT
					Assert::IsTrue(!!lv1);
					Assert::IsTrue(!!lv2);
					Assert::IsTrue(lv1 != lv2);

					Assert::IsTrue(2u == wt.created.size());
					Assert::IsTrue(0u == wt.destroyed.size());

					Assert::IsTrue(f.second == ::GetParent(wt.created[0]));
					Assert::IsTrue(f.second == ::GetParent(wt.created[1]));
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
					ut::window_tracker wt;
					weak_ptr<widget> lv1_weak = lv1;
					weak_ptr<widget> lv2_weak = lv2;
					
					// ACT
					lv1 = shared_ptr<widget>();

					// ASSERT
					Assert::IsFalse(lv1_weak.expired());
					
					// ACT
					lv2 = shared_ptr<widget>();
					wt.checkpoint();

					// ASSERT
					Assert::IsTrue(wt.destroyed.empty());
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


				[TestMethod]
				void TwoChildWidgetsAreRepositionedAccordinglyToTheLayout()
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr<container> c = f.first->get_root_container();
					shared_ptr<ut::logging_layout_manager> lm(new ut::logging_layout_manager);
					ut::window_tracker wt(L"SysListView32");
					shared_ptr<widget> lv1 = c->create_widget(L"listview", L"1");
					wt.checkpoint();
					shared_ptr<widget> lv2 = c->create_widget(L"listview", L"2");
					wt.checkpoint();
					f.first->get_root_container()->layout = lm;

					// ACT
					layout_manager::position positions1[] = {
						{ 10, 21, 33, 15 },
						{ 17, 121, 133, 175 },
					};
					lm->positions.assign(positions1, positions1 + _countof(positions1));
					::MoveWindow(f.second, 23, 91, 167, 213, TRUE);

					// ASSERT
					Assert::IsTrue(ut::rect(10, 21, 33, 15) == ut::get_window_rect(wt.created[0]));
					Assert::IsTrue(ut::rect(17, 121, 133, 175) == ut::get_window_rect(wt.created[1]));

					// ACT
					layout_manager::position positions2[] = {
						{ 13, 121, 43, 31 },
						{ 71, 21, 113, 105 },
					};
					lm->positions.assign(positions2, positions2 + _countof(positions2));
					::MoveWindow(f.second, 23, 91, 117, 213, TRUE);

					// ASSERT
					Assert::IsTrue(ut::rect(13, 121, 43, 31) == ut::get_window_rect(wt.created[0]));
					Assert::IsTrue(ut::rect(71, 21, 113, 105) == ut::get_window_rect(wt.created[1]));
				}


				[TestMethod]
				void ThreeChildWidgetsAreRepositionedAccordinglyToTheLayout()
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr<container> c = f.first->get_root_container();
					shared_ptr<ut::logging_layout_manager> lm(new ut::logging_layout_manager);
					ut::window_tracker wt(L"SysListView32");
					shared_ptr<widget> lv1 = c->create_widget(L"listview", L"1");
					wt.checkpoint();
					shared_ptr<widget> lv2 = c->create_widget(L"listview", L"2");
					wt.checkpoint();
					shared_ptr<widget> lv3 = c->create_widget(L"listview", L"3");
					wt.checkpoint();
					f.first->get_root_container()->layout = lm;

					// ACT
					layout_manager::position positions[] = {
						{ 10, 21, 33, 115 },
						{ 11, 191, 133, 175 },
						{ 16, 131, 103, 185 },
					};
					lm->positions.assign(positions, positions + _countof(positions));
					::MoveWindow(f.second, 23, 91, 117, 213, TRUE);

					// ASSERT
					Assert::IsTrue(ut::rect(10, 21, 33, 115) == ut::get_window_rect(wt.created[0]));
					Assert::IsTrue(ut::rect(11, 191, 133, 175) == ut::get_window_rect(wt.created[1]));
					Assert::IsTrue(ut::rect(16, 131, 103, 185) == ut::get_window_rect(wt.created[2]));
				}


				[TestMethod]
				void SettingCaptionUpdatesWindowText()
				{
					// INIT
					form_and_handle f(create_form_with_handle());

					// ACT
					f.first->set_caption(L"Dialog #1...");

					// ASSERT
					Assert::IsTrue(L"Dialog #1..." == ut::get_window_text(f.second));

					// ACT
					f.first->set_caption(L"Are you sure?");

					// ASSERT
					Assert::IsTrue(L"Are you sure?" == ut::get_window_text(f.second));
				}


				[TestMethod]
				void ClosingWindowRaisesCloseSignal()
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					int close_count = 0;
					slot_connection c = f.first->close += bind(&increment, &close_count);

					// ACT
					::SendMessage(f.second, WM_CLOSE, 0, 0);

					// ASSERT
					Assert::IsTrue(1 == close_count);

					// ACT (the form still exists)
					::SendMessage(f.second, WM_CLOSE, 0, 0);

					// ASSERT
					Assert::IsTrue(2 == close_count);
				}
			};
		}
	}
}
