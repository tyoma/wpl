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

			pair<shared_ptr<win32::view_host>, HWND /*hoverlay*/> create_view_host(HWND hwnd, const form_context &context_)
			{
				tracker.checkpoint(), tracker.created.clear();
				const auto vh = make_shared<win32::view_host>(hwnd, context_);
				tracker.checkpoint();
				return make_pair(vh, tracker.created[0]);
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


			test( OverlayWindowGetsCreatedOnViewHostConstruction )
			{
				// INIT
				const auto hwnd = create_window(true);

				tracker.checkpoint(), tracker.created.clear();

				// ACT
				unique_ptr<win32::view_host> vh(new win32::view_host(hwnd, context));

				// ASSERT
				tracker.checkpoint();

				assert_equal(1u, tracker.find_created(L"static").size());
				const auto hoverlay = tracker.created[0];
				assert_is_false(!!::IsWindowVisible(hoverlay));
				assert_equal(hwnd, ::GetWindow(hoverlay, GW_OWNER));
				assert_equal(WS_POPUP, (WS_POPUP | WS_OVERLAPPEDWINDOW | WS_CHILD | WS_BORDER)
					& ::GetWindowLongPtr(hoverlay, GWL_STYLE));

				// ACT
				vh.reset();

				// ASSERT
				tracker.checkpoint();
				assert_equal(1u, tracker.destroyed.size());
				assert_equal(hoverlay, tracker.destroyed[0]);
			}


			test( OverlayWindowIsNotVisibleIfNoOverlayViewsInLayout )
			{
				// INIT
				const auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv[] = {	{	v, nullptr_nv, create_rect(0, 0, 1000, 1000), 0, false	}	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				const auto vh = create_view_host(hwnd, context);

				// ACT
				vh.first->set_root(ctl);

				// ASSERT
				assert_is_false(!!::IsWindowVisible(vh.second));

				// INIT
				ctl->views = mkvector(pv);

				// ACT
				ctl->layout_changed(true);

				// ASSERT
				assert_is_false(!!::IsWindowVisible(vh.second));
			}


			test( OverlayWindowBecomesVisibleIfOverlayViewIsMetInLayoutOnSettingRoot )
			{
				// INIT
				const auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv[] = {	{	v, nullptr_nv, create_rect(0, 0, 71, 37), 0, true	}	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				const auto vh = create_view_host(hwnd, context);

				ctl->views = mkvector(pv);

				// ACT
				vh.first->set_root(ctl);

				// ASSERT
				assert_is_true(!!::IsWindowVisible(vh.second));
			}


			test( OverlayWindowGetsHiddenWhenOverlayViewsDisappear )
			{
				// INIT
				const auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv[] = {	{	v, nullptr_nv, create_rect(0, 0, 71, 37), 0, true	}	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				const auto vh = create_view_host(hwnd, context);

				ctl->views = mkvector(pv);
				vh.first->set_root(ctl);

				// ACT
				pv[0].overlay = false;
				ctl->views = mkvector(pv);
				ctl->layout_changed(true);

				// ASSERT
				assert_is_false(!!::IsWindowVisible(vh.second));
			}


			test( OverlayWindowSizeIsAUnionOfOverlayViewRects )
			{
				// INIT
				const auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv1[] = {	{	v, nullptr_nv, create_rect(2, 100, 71, 137), 0, true	}	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				const auto vh = create_view_host(hwnd, context);

				::MoveWindow(hwnd, 17, 121, 100, 100, TRUE);
				vh.first->set_root(ctl);
				ctl->views = mkvector(pv1);

				// ACT
				ctl->layout_changed(true);

				// ASSERT
				assert_equal(create_rect(19, 221, 88, 258), get_window_rect(vh.second));

				// INIT
				::MoveWindow(hwnd, 5, 11, 100, 100, TRUE);

				// INIT
				placed_view pv2[] = {
					{	v, nullptr_nv, create_rect(30, 19, 40, 41), 0, true	},
					{	v, nullptr_nv, create_rect(32, 9, 20, 80), 0, true	},
				};

				ctl->views = mkvector(pv2);

				// ACT
				ctl->layout_changed(true);

				// ASSERT
				assert_equal(create_rect(30 + 5, 9 + 11, 40 + 5, 80 + 11), get_window_rect(vh.second));
			}


			test( InvalidatedAreaIsOffsetForOverlayWindow )
			{
				// INIT
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv1[] = {
					{	v[0], nullptr_nv, create_rect(21, 41, 35, 60), 0, true	},
					{	v[1], nullptr_nv, create_rect(25, 30, 71, 137), 0, true	},
				};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				const auto vh = create_view_host(hwnd, context);

				ctl->views = mkvector(pv1);
				vh.first->set_root(ctl);
				::ValidateRect(hwnd, nullptr);
				::ValidateRect(vh.second, nullptr);

				// ACT
				v[0]->invalidate(nullptr);

				// ASSERT
				assert_is_false(!!::GetUpdateRect(hwnd, NULL, FALSE));
				assert_equal(create_rect(0, 11, 14, 30), get_update_rect(vh.second));

				// INIT
				::ValidateRect(vh.second, nullptr);

				// ACT
				v[1]->invalidate(nullptr);

				// ASSERT
				assert_is_false(!!::GetUpdateRect(hwnd, NULL, FALSE));
				assert_equal(create_rect(4, 0, 50, 107), get_update_rect(vh.second));

				// INIT
				placed_view pv2[] = {
					{	v[0], nullptr_nv, create_rect(0, 0, 10, 10), 0, true	},
				};

				ctl->views = mkvector(pv2);
				ctl->layout_changed(true);
				::ValidateRect(hwnd, nullptr);
				::ValidateRect(vh.second, nullptr);

				// ACT
				v[0]->invalidate(nullptr);

				// ASSERT
				assert_is_false(!!::GetUpdateRect(hwnd, NULL, FALSE));
				assert_equal(create_rect(0, 0, 10, 10), get_update_rect(vh.second));
			}
		end_test_suite
	}
}
