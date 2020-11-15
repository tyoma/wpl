#include <wpl/win32/cursor_manager.h>
#include <wpl/win32/view_host.h>

#include "helpers-win32.h"
#include "Mockups.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		begin_test_suite( NativeCursorManagementTests )
			unique_ptr<hosting_window> window;

			init( Init )
			{
				window.reset(new hosting_window);
			}


			test( ViewHostPassesMouseCoordinatesWhenAskingForACursor )
			{
				// INIT
				auto v = make_shared< mocks::logging_visual<view> >();

				window->host->set_view(v);
				::MoveWindow(window->hwnd, 17, 19, 100, 100, FALSE);
				::SetCursorPos(103, 191);

				// ACT
				assert_equal(TRUE, ::SendMessage(window->hwnd, WM_SETCURSOR, 0, 0));

				// ASSERT
				assert_equal(1u, v->cursor_request_log.size());
				assert_equal(window->cursor_manager.get(), v->cursor_request_log[0].first);

				POINT pt1 = { v->cursor_request_log[0].second.first, v->cursor_request_log[0].second.second };
				::ClientToScreen(window->hwnd, &pt1);

				assert_equal(103, pt1.x);
				assert_equal(191, pt1.y);

				// INIT
				::MoveWindow(window->hwnd, 71, 100, 100, 100, FALSE);

				// ACT
				assert_equal(TRUE, ::SendMessage(window->hwnd, WM_SETCURSOR, 0, 0));

				// ASSERT
				assert_equal(2u, v->cursor_request_log.size());

				POINT pt2 = { v->cursor_request_log[1].second.first, v->cursor_request_log[1].second.second };
				::ClientToScreen(window->hwnd, &pt2);

				assert_equal(103, pt2.x);
				assert_equal(191, pt2.y);

				// INIT
				::SetCursorPos(1, 2);

				// ACT
				assert_equal(TRUE, ::SendMessage(window->hwnd, WM_SETCURSOR, 0, 0));

				// ASSERT
				assert_equal(3u, v->cursor_request_log.size());

				POINT pt3 = { v->cursor_request_log[2].second.first, v->cursor_request_log[2].second.second };
				::ClientToScreen(window->hwnd, &pt3);

				assert_equal(1, pt3.x);
				assert_equal(2, pt3.y);
			}
		end_test_suite
	}
}