#include <wpl/ui/form.h>
#include <wpl/ui/win32/native_view.h>
#include <wpl/ui/win32/form.h>

#include "Mockups.h"
#include "MockupsNative.h"
#include "TestHelpers.h"

#include <tchar.h>
#include <ut/assert.h>
#include <ut/test.h>
#include <windows.h>

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
				typedef pair<shared_ptr<form>, HWND> form_and_handle;

				form_and_handle create_form_with_handle(HWND howner = 0)
				{
					window_tracker wt(L"#32770");
					shared_ptr<form> f(create_form(howner));

					wt.checkpoint();

					if (wt.created.size() != 1)
						throw runtime_error("Unexpected amount of windows created!");
					return make_pair(f, wt.created[0]);
				}

				shared_ptr<void> create_window(HWND hparent)
				{
					return shared_ptr<void>(::CreateWindow(_T("static"), NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hparent,
						NULL, NULL, 0), &::DestroyWindow);
				}

				void increment(int *counter)
				{	++*counter;	}
			}

			begin_test_suite( FormTests )
				WindowManager windowManager;

				init( Init )
				{
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
					shared_ptr<form> f1 = create_form();

					// ASSERT
					wt.checkpoint();

					assert_equal(1u, wt.created.size());
					assert_is_empty(wt.destroyed);

					// ACT
					shared_ptr<form> f2 = create_form();
					shared_ptr<form> f3 = create_form();

					// ASSERT
					wt.checkpoint();

					assert_equal(3u, wt.created.size());
					assert_is_empty(wt.destroyed);
				}


				test( FormConstructionReturnsNonNullObject )
				{
					// INIT / ACT / ASSERT
					assert_not_null(create_form());
				}


				test( FormDestructionDestroysItsWindow )
				{
					// INIT
					shared_ptr<form> f1 = create_form();
					shared_ptr<form> f2 = create_form();
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


				test( FormWindowHasPopupStyleAndInvisibleAtConstruction )
				{
					// INIT / ACT
					form_and_handle f(create_form_with_handle());

					// ASSERT
					assert_is_true(has_style(f.second, WS_CAPTION | WS_THICKFRAME | WS_CLIPCHILDREN));
					assert_is_true(has_no_style(f.second, WS_VISIBLE));
				}


				test( ChangingFormVisibilityAffectsItsWindowVisibility )
				{
					// INIT
					form_and_handle f(create_form_with_handle());

					// ACT
					f.first->set_visible(true);

					// ASSERT
					assert_is_true(has_style(f.second, WS_VISIBLE));

					// ACT
					f.first->set_visible(false);

					// ASSERT
					assert_is_true(has_no_style(f.second, WS_VISIBLE));
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


				test( ResizingFormWindowLeadsToContentResize )
				{
					// INIT
					shared_ptr<mocks::logging_visual<view>> v(new mocks::logging_visual<view>());
					form_and_handle f(create_form_with_handle());
					RECT rc;

					f.first->set_view(v);
					v->resize_log.clear();

					// ACT
					::MoveWindow(f.second, 0, 0, 117, 213, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					assert_equal(1u, v->resize_log.size());
					assert_equal(make_pair((int)rc.right, (int)rc.bottom), v->resize_log[0]);

					// ACT
					::MoveWindow(f.second, 27, 190, 531, 97, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					assert_equal(2u, v->resize_log.size());
					assert_equal(make_pair((int)rc.right, (int)rc.bottom), v->resize_log[1]);
				}


				test( FormIsOwnedIfOwnerIsSpecified )
				{
					// INIT
					form_and_handle owner1(create_form_with_handle()), owner2(create_form_with_handle());

					// ACT
					form_and_handle owned1(create_form_with_handle(owner1.second));
					form_and_handle owned2(create_form_with_handle(owner2.second));

					// ASSERT
					assert_is_true(has_no_style(owned1.second, WS_CHILD));
					assert_equal(owner1.second, ::GetWindow(owned1.second, GW_OWNER));
					assert_is_true(has_no_style(owned2.second, WS_CHILD));
					assert_equal(owner2.second, ::GetWindow(owned2.second, GW_OWNER));
				}

			end_test_suite
		}
	}
}
