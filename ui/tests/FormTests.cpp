#include <wpl/ui/form.h>

#include <wpl/ui/layout.h>

#include "Mockups.h"
#include "TestHelpers.h"

#include <ut/assert.h>
#include <ut/test.h>
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
				window_tracker wt(L"#32770");
				shared_ptr<form> f(form::create());

				wt.checkpoint();

				if (wt.created.size() != 1)
					throw runtime_error("Unexpected amount of windows created!");
				return make_pair(f, wt.created[0]);
			}

			begin_test_suite( FormTests )
				WindowManager windowManager;

				init( Init )
				{
					windowManager.Init();
					windowManager.create_window();
				}

				teardown( Cleanup )
				{
					windowManager.Cleanup();
				}


				test( FormWindowIsCreatedAtFormConstruction )
				{
					// INIT
					window_tracker wt(L"#32770");

					// ACT
					shared_ptr<form> f1 = form::create();

					// ASSERT
					wt.checkpoint();

					assert_equal(1u, wt.created.size());
					assert_is_empty(wt.destroyed);

					// ACT
					shared_ptr<form> f2 = form::create();
					shared_ptr<form> f3 = form::create();

					// ASSERT
					wt.checkpoint();

					assert_equal(3u, wt.created.size());
					assert_is_empty(wt.destroyed);
				}


				test( FormConstructionReturnsNonNullObject )
				{
					// INIT / ACT / ASSERT
					assert_not_null(form::create());
				}


				test( FormDestructionDestroysItsWindow )
				{
					// INIT
					shared_ptr<form> f1 = form::create();
					shared_ptr<form> f2 = form::create();
					window_tracker wt(L"#32770");

					// ACT
					f1 = shared_ptr<form>();

					// ASSERT
					wt.checkpoint();

					assert_is_empty(wt.created);
					assert_equal(1u, wt.destroyed.size());

					// ACT
					f2 = shared_ptr<form>();

					// ASSERT
					wt.checkpoint();

					assert_is_empty(wt.created);
					assert_equal(2u, wt.destroyed.size());
				}


				test( FormWindowIsHasPopupStyleAndInvisibleAtConstruction )
				{
					// INIT / ACT
					form_and_handle f(create_form_with_handle());

					// ASSERT
					DWORD style = ::GetWindowLong(f.second, GWL_STYLE);

					assert_is_false(!!(WS_VISIBLE & style));
					assert_is_true(!!(WS_THICKFRAME & style));
					assert_is_true(!!(WS_CAPTION & style));
					assert_is_true(!!(WS_CLIPCHILDREN & style));
				}


				test( ChangingFormVisibilityAffectsItsWindowVisibility )
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					
					f.first->get_root_container()->layout.reset(new mocks::logging_layout_manager);

					// ACT
					f.first->set_visible(true);

					// ASSERT
					assert_is_true(!!(WS_VISIBLE & ::GetWindowLong(f.second, GWL_STYLE)));

					// ACT
					f.first->set_visible(false);

					// ASSERT
					assert_is_false(!!(WS_VISIBLE & ::GetWindowLong(f.second, GWL_STYLE)));
				}


				test( FormProvidesAValidContainer )
				{
					// INIT
					shared_ptr<form> f = form::create();

					// ACT / ASSERT
					assert_not_null(f->get_root_container());
				}


				test( ResizingFormWindowLeadsToContentResize )
				{
					// INIT
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					form_and_handle f(create_form_with_handle());
					RECT rc;

					f.first->get_root_container()->layout = lm;

					// ACT
					::MoveWindow(f.second, 0, 0, 117, 213, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					assert_equal(1u, lm->reposition_log.size());
					assert_equal(rc.right, (int)lm->reposition_log[0].first);
					assert_equal(rc.bottom, (int)lm->reposition_log[0].second);

					// ACT
					::MoveWindow(f.second, 27, 190, 531, 97, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					assert_equal(2u, lm->reposition_log.size());
					assert_equal(rc.right, (int)lm->reposition_log[1].first);
					assert_equal(rc.bottom, (int)lm->reposition_log[1].second);
				}


				test( MovingFormDoesNotRaiseResizeSignal )
				{
					// INIT
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
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
					assert_is_empty(lm->reposition_log);
				}


				test( ChildrenAreCreatedViaContainer )
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr<container> c = f.first->get_root_container();
					window_tracker wt(L"SysListView32");

					// ACT
					shared_ptr<widget> lv1 = c->create_widget(L"listview", L"1");
					shared_ptr<widget> lv2 = c->create_widget(L"listview", L"2");
					wt.checkpoint();
					
					// ASSERT
					assert_not_null(lv1);
					assert_not_null(lv2);
					assert_not_equal(lv1, lv2);

					assert_equal(2u, wt.created.size());
					assert_equal(0u, wt.destroyed.size());

					assert_equal(f.second, ::GetParent(wt.created[0]));
					assert_equal(f.second, ::GetParent(wt.created[1]));
				}


				test( ChildrenAreHeldByForm )
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr<container> c = f.first->get_root_container();
					vector<HWND> w;
					shared_ptr<widget> lv1 = c->create_widget(L"listview", L"1");
					shared_ptr<widget> lv2 = c->create_widget(L"listview", L"2");
					window_tracker wt;
					weak_ptr<widget> lv1_weak = lv1;
					weak_ptr<widget> lv2_weak = lv2;
					
					// ACT
					lv1 = shared_ptr<widget>();

					// ASSERT
					assert_is_false(lv1_weak.expired());
					
					// ACT
					lv2 = shared_ptr<widget>();
					wt.checkpoint();

					// ASSERT
					assert_is_empty(wt.destroyed);
					assert_is_false(lv2_weak.expired());
				}


				test( ChildrenDestroyedOnFormDestruction )
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr<widget> lv = f.first->get_root_container()->create_widget(L"listview", L"1");
					weak_ptr<widget> lv_weak = lv;

					lv = shared_ptr<widget>();

					// ACT
					f.first = shared_ptr<form>();

					// ASSERT
					assert_is_true(lv_weak.expired());
				}


				test( TwoChildWidgetsAreRepositionedAccordinglyToTheLayout )
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr<container> c = f.first->get_root_container();
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					window_tracker wt(L"SysListView32");
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
					assert_equal(rect(10, 21, 33, 15), get_window_rect(wt.created[0]));
					assert_equal(rect(17, 121, 133, 175), get_window_rect(wt.created[1]));

					// ACT
					layout_manager::position positions2[] = {
						{ 13, 121, 43, 31 },
						{ 71, 21, 113, 105 },
					};
					lm->positions.assign(positions2, positions2 + _countof(positions2));
					::MoveWindow(f.second, 23, 91, 117, 213, TRUE);

					// ASSERT
					assert_equal(rect(13, 121, 43, 31), get_window_rect(wt.created[0]));
					assert_equal(rect(71, 21, 113, 105), get_window_rect(wt.created[1]));
				}


				test( ThreeChildWidgetsAreRepositionedAccordinglyToTheLayout )
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr<container> c = f.first->get_root_container();
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					window_tracker wt(L"SysListView32");
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
					assert_equal(rect(10, 21, 33, 115), get_window_rect(wt.created[0]));
					assert_equal(rect(11, 191, 133, 175), get_window_rect(wt.created[1]));
					assert_equal(rect(16, 131, 103, 185), get_window_rect(wt.created[2]));
				}


				test( SettingCaptionUpdatesWindowText )
				{
					// INIT
					form_and_handle f(create_form_with_handle());

					// ACT
					f.first->set_caption(L"Dialog #1...");

					// ASSERT
					assert_equal(L"Dialog #1...", get_window_text(f.second));

					// ACT
					f.first->set_caption(L"Are you sure?");

					// ASSERT
					assert_equal(L"Are you sure?", get_window_text(f.second));
				}


				test( ClosingWindowRaisesCloseSignal )
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					int close_count = 0;
					slot_connection c = f.first->close += bind(&increment, &close_count);

					// ACT
					::SendMessage(f.second, WM_CLOSE, 0, 0);

					// ASSERT
					assert_equal(1, close_count);

					// ACT (the form still exists)
					::SendMessage(f.second, WM_CLOSE, 0, 0);

					// ASSERT
					assert_equal(2, close_count);
				}
			end_test_suite
		}
	}
}
