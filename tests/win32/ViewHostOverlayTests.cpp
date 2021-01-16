#include <wpl/win32/view_host.h>

#include "helpers-win32.h"
#include "MockupsNative.h"

#include <tests/common/mock-control.h>
#include <tests/common/Mockups.h>

#include <ut/assert.h>
#include <ut/test.h>
#include <wpl/stylesheet_db.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			const auto nullptr_nv = shared_ptr<native_view>();
		}

		begin_test_suite( ViewHostOverlayTests )

			form_context context;
			window_manager wm;
			shared_ptr<mocks::cursor_manager> cursor_manager;
			window_tracker tracker;

			HWND create_window(bool visible = false, int width = 100, int height = 100)
			{
				auto hwnd = wm.create_window(L"static", NULL, WS_POPUP | (visible ? WS_VISIBLE : 0), 0);

				::MoveWindow(hwnd, 10, 20, width, height, FALSE);
				::ValidateRect(hwnd, NULL);
				return hwnd;
			}

			init( Init )
			{
				context.backbuffer = make_shared<gcontext::surface_type>(1, 1, 0);
				context.renderer = make_shared<gcontext::renderer_type>(1);
				context.text_engine = create_text_engine();
				context.stylesheet_ = make_shared<stylesheet_db>();
				context.cursor_manager_ = cursor_manager = make_shared<mocks::cursor_manager>();
				cursor_manager->cursors[cursor_manager::arrow].reset(new cursor(16, 16, 0, 0));
			}


			ignored_test( OverlayWindowIsNotCreatedIfNoOverlayViewsInLayout )
			{
				// INIT
				const auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv[] = {	{	v, nullptr_nv, create_rect(0, 0, 1000, 1000), false	}	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				win32::view_host vh(hwnd, context);

				tracker.checkpoint();

				// ACT
				vh.set_root(ctl);

				// ASSERT
				tracker.checkpoint();

				assert_is_empty(tracker.created);

				// INIT
				ctl->views = mkvector(pv);

				// ACT
				ctl->layout_changed(true);

				// ASSERT
				tracker.checkpoint();

				assert_is_empty(tracker.created);
			}


			ignored_test( OverlayWindowGetsCreatedIfOverlayViewIsMetInLayoutOnSettingRoot )
			{
				// INIT
				const auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv[] = {	{	v, nullptr_nv, create_rect(0, 0, 71, 37), true	}	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				win32::view_host vh(hwnd, context);

				ctl->views = mkvector(pv);
				tracker.checkpoint();

				// ACT
				vh.set_root(ctl);

				// ASSERT
				tracker.checkpoint();

				assert_equal(1u, tracker.find_created(L"#32770").size());
			}


			ignored_test( OverlayWindowSizeIsAUnionOfOverlayViewRects )
			{
				// INIT
				const auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv1[] = {	{	v, nullptr_nv, create_rect(2, 100, 71, 137), true	}	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				win32::view_host vh(hwnd, context);

				::MoveWindow(hwnd, 17, 121, 100, 100, TRUE);
				vh.set_root(ctl);
				ctl->views = mkvector(pv1);
				tracker.checkpoint();

				// ACT
				ctl->layout_changed(true);

				// ASSERT
				tracker.checkpoint();
				const auto hoverlay = tracker.find_created(L"#32770")[0];

				assert_equal(create_rect(19, 221, 88, 258), get_window_rect(hoverlay));

				// INIT
				::MoveWindow(hwnd, 5, 11, 100, 100, TRUE);

				// INIT
				placed_view pv2[] = {
					{	v, nullptr_nv, create_rect(30, 19, 40, 41), true	},
					{	v, nullptr_nv, create_rect(32, 9, 20, 80), true	},
				};

				ctl->views = mkvector(pv2);

				// ACT
				ctl->layout_changed(true);

				// ASSERT
				tracker.checkpoint();
				assert_is_empty(tracker.created);
				assert_is_empty(tracker.destroyed);

				assert_equal(create_rect(30 + 5, 9 + 11, 40 + 5, 80 + 11), get_window_rect(hoverlay));
			}
		end_test_suite
	}
}
