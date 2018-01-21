#include <wpl/ui/button.h>

#include <wpl/ui/form.h>

#include "Mockups.h"
#include "TestHelpers.h"

#include <ut/assert.h>
#include <ut/test.h>
#include <windows.h>

using namespace std;

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

			begin_test_suite( ButtonTests )

				WindowManager windowManager;

				init( Init )
				{
					windowManager.Init();
				}

				teardown( Cleanup )
				{
					windowManager.Cleanup();
				}

				test( CreateButton )
				{
					// INIT
					shared_ptr<form> f = form::create();
					window_tracker wt(L"button");

					// ACT
					shared_ptr<button> b = static_pointer_cast<button>(create_widget(wt, *f->get_root_container(), L"button", L"1"));

					// ASSERT
					assert_not_null(b);
					assert_equal(1u, wt.created.size());
				}


				test( ReceiveButtonClickOnMouseDownUp )
				{
					// INIT
					shared_ptr<form> f = form::create();
					window_tracker wt(L"button");
					shared_ptr<button> b = static_pointer_cast<button>(create_widget(wt, *f->get_root_container(), L"button", L"1"));
					int click_count = 0;
					slot_connection c = b->clicked += bind(&increment, &click_count);

					f->get_root_container()->layout.reset(new mocks::fill_layout);

					// ACT
					::SendMessage(wt.created[0], WM_LBUTTONDOWN, 0, 0);

					// ASSERT
					assert_equal(0, click_count);

					// ACT
					::SendMessage(wt.created[0], WM_LBUTTONUP, 0, 0);

					// ASSERT
					assert_equal(1, click_count);
				}


				test( SettingTextUpdatesButtonWindowText )
				{
					// INIT
					shared_ptr<form> f = form::create();
					window_tracker wt(L"button");
					shared_ptr<button> b = static_pointer_cast<button>(create_widget(wt, *f->get_root_container(), L"button", L"1"));

					// ACT
					b->set_text(L"Start doing something!");

					// ASSERT
					assert_equal(L"Start doing something!", get_window_text(wt.created[0]));

					// ACT
					b->set_text(L"Stop that!");

					// ASSERT
					assert_equal(L"Stop that!", get_window_text(wt.created[0]));
				}
			end_test_suite
		}
	}
}
