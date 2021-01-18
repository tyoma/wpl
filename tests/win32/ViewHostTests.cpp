#include <wpl/win32/view_host.h>

#include "helpers-win32.h"
#include "MockupsNative.h"

#include <tests/common/helpers-visual.h>
#include <tests/common/mock-control.h>
#include <tests/common/Mockups.h>

#include <ut/assert.h>
#include <ut/test.h>
#include <wpl/stylesheet_db.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			rect_i get_client_rect(HWND hwnd)
			{
				RECT rc;

				::GetClientRect(hwnd, &rc);
				return create_rect<int>(rc.left, rc.top, rc.right, rc.bottom);
			}
		}

		begin_test_suite( ViewHostTests )

			form_context context;
			window_manager wm;
			shared_ptr<mocks::cursor_manager> cursor_manager;

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


			test( ResizingHostWindowLeadsToContentResize )
			{
				// INIT
				auto ctl = make_shared<mocks::control>();
				auto hwnd = create_window();
				win32::view_host vh(hwnd, context);

				// INIT / ACT
				vh.set_root(ctl);
				ctl->size_log.clear();

				// ACT
				::MoveWindow(hwnd, 0, 0, 117, 213, TRUE);

				// ASSERT
				assert_equal(1u, ctl->size_log.size());
				assert_equal(get_client_size(hwnd), ctl->size_log.back());

				// ACT
				::MoveWindow(hwnd, 27, 190, 531, 97, TRUE);

				// ASSERT
				assert_equal(2u, ctl->size_log.size());
				assert_equal(get_client_size(hwnd), ctl->size_log.back());
			}


			test( MovingHostWindowDoesNotRaiseResizeSignal )
			{
				// INIT
				auto ctl = make_shared<mocks::control>();
				auto hwnd = create_window();
				win32::view_host vh(hwnd, context);

				vh.set_root(ctl);
				::MoveWindow(hwnd, 0, 0, 117, 213, TRUE);
				ctl->size_log.clear();

				// ACT
				::MoveWindow(hwnd, 23, 91, 117, 213, TRUE);
				::MoveWindow(hwnd, 53, 91, 117, 213, TRUE);
				::MoveWindow(hwnd, 53, 32, 117, 213, TRUE);
				::MoveWindow(hwnd, 23, 100, 117, 213, TRUE);

				// ASSERT
				assert_is_empty(ctl->size_log);
			}


			test( RedrawingHostWindowSuppliesBitmapOfTheSizeCorrespondingToRedrawAreaToTheContent )
			{
				// INIT
				const auto v = make_shared< mocks::logging_visual<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 1000, 1000)	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 1000, 1000);
				win32::view_host vh(hwnd, context);
				RECT rc = { 0, 0, 100, 60 };

				ctl->views.push_back(pv);

				vh.set_root(ctl);
				context.backbuffer->resize(1, 1);
				::ValidateRect(hwnd, NULL);

				// ACT
				::RedrawWindow(hwnd, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				// ASSERT
				assert_equal(1u, v->surface_size_log.size());
				assert_is_true(100 <= v->surface_size_log[0].first);
				assert_is_true(115 > v->surface_size_log[0].first);
				assert_is_true(60 <= v->surface_size_log[0].second);
				assert_is_true(71 > v->surface_size_log[0].second);

				// INIT
				rc.left = 20;
				rc.right = 135;
				rc.top = 5;
				rc.bottom = 76;

				// ACT
				::RedrawWindow(hwnd, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				// ASSERT
				assert_equal(2u, v->text_engines_log.size());
				assert_equal(context.text_engine.get(), v->text_engines_log[0]);
				assert_equal(context.text_engine.get(), v->text_engines_log[1]);

				assert_equal(2u, v->surface_size_log.size());
				assert_is_true(115 <= v->surface_size_log[1].first);
				assert_is_true(135 > v->surface_size_log[1].first);
				assert_is_true(71 <= v->surface_size_log[1].second);
				assert_is_true(76 > v->surface_size_log[1].second);

				assert_equal(115, static_cast<int>(context.backbuffer->width()));
				assert_equal(71, static_cast<int>(context.backbuffer->height()));
			}


			test( WindowPassedToDrawCorrespondsToInvalidArea )
			{
				// INIT
				const auto v = make_shared< mocks::logging_visual<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 1000, 1000)	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 1000, 1000);
				win32::view_host vh(hwnd, context);
				RECT rc = { 11, 17, 100, 60 };

				ctl->views.push_back(pv);

				vh.set_root(ctl);
				v->transcending = true;
				::ValidateRect(hwnd, NULL);

				// ACT
				::RedrawWindow(hwnd, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				// ASSERT
				assert_equal(1u, v->update_area_log.size());
				assert_equal(create_rect(11, 17, 100, 60), v->update_area_log[0]);

				// INIT
				RECT rc2 = { 101, 107, 150, 260 };

				// ACT
				::RedrawWindow(hwnd, &rc2, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				// ASSERT
				assert_equal(2u, v->update_area_log.size());
				assert_equal(create_rect(101, 107, 150, 260), v->update_area_log[1]);
			}


			test( TheSameRasterizerIsSuppliedToDrawingProcedure )
			{
				// INIT
				const auto v = make_shared< mocks::logging_visual<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 100)	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				ctl->views.push_back(pv);

				vh.set_root(ctl);

				// ACT
				::RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				::RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				// ASSERT
				assert_equal(2u, v->rasterizers_log.size());
				assert_not_null(v->rasterizers_log[0]);
				assert_equal(v->rasterizers_log[0], v->rasterizers_log[1]);
			}


			test( RasterizerIsResetOnDraw )
			{
				// INIT
				const auto v = make_shared< mocks::logging_visual<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 100)	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				ctl->views.push_back(pv);

				vh.set_root(ctl);

				// ACT / ASSERT (done inside)
				::RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				::RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}


			test( WholeClientIsInvalidatedOnResizeAndRootSetting )
			{
				// INIT
				const auto hwnd = create_window(true, 40, 40);
				win32::view_host vh(hwnd, context);
				const auto ctl = make_shared<mocks::control>();

				::ShowWindow(hwnd, SW_SHOW);
				::ValidateRect(hwnd, NULL);

				// ACT
				vh.set_root(ctl);

				// ASSERT
				assert_equal(get_client_rect(hwnd), get_update_rect(hwnd));

				// ACT
				::MoveWindow(hwnd, 10, 10, 200, 40, FALSE);

				// ASSERT
				assert_equal(get_client_rect(hwnd), get_update_rect(hwnd));

				// ACT
				::MoveWindow(hwnd, 10, 10, 250, 100, TRUE);

				// ASSERT
				assert_equal(get_client_rect(hwnd), get_update_rect(hwnd));
			}


			test( RootLayoutChangesAreIgnoredOnRootReset )
			{
				// INIT
				const auto hwnd = create_window(true, 40, 40);
				win32::view_host vh(hwnd, context);
				const auto ctl = make_shared<mocks::control>();

				::ShowWindow(hwnd, SW_SHOW);
				vh.set_root(ctl);
				::ValidateRect(hwnd, NULL);

				// ACT
				vh.set_root(nullptr);

				// ASSERT
				assert_equal(get_client_rect(hwnd), get_update_rect(hwnd));

				// INIT
				::ValidateRect(hwnd, NULL);

				// ACT
				ctl->layout_changed(false);
				ctl->layout_changed(true);

				// ASSERT
				assert_is_false(!!::GetUpdateRect(hwnd, NULL, FALSE));
			}


			test( ForcingLayoutWithoutHierarchyChangeSignalLeadsToLayoutRequestInvalidatesWindow )
			{
				// INIT
				const shared_ptr<view> v[] = {
					make_shared<view>(), make_shared<view>(), make_shared<view>(),
				};
				const placed_view pv[] = {
					{ v[0], nullptr, create_rect(1, 7, 30, 25), 1 },
					{ v[1], nullptr, create_rect(1, 7, 100, 100), 2 },
					{ v[2], nullptr, create_rect(50, 40, 75, 100), 3 },
				};
				auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				ctl->views.assign(begin(pv), end(pv));
				vh.set_root(ctl);
				::ValidateRect(hwnd, NULL);
				ctl->size_log.clear();

				// ACT
				ctl->layout_changed(false);

				// ASSERT
				box<int> reference1[] = {	get_client_size(hwnd),	};

				assert_equal(reference1, ctl->size_log);
				assert_equal(get_client_rect(hwnd), get_update_rect(hwnd));
			}


			test( HostReactsOnInvalidateRequestsFromViews )
			{
				// INIT
				const shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv[] = {
					{	v[0], nullptr, create_rect(1, 7, 100, 100),	},
					{	v[1], nullptr, create_rect(50, 50, 110, 105),	},
				};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 1000, 1000);
				win32::view_host vh(hwnd, context);

				ctl->views = mkvector(pv);
				vh.set_root(ctl);
				::ValidateRect(hwnd, NULL);

				// ACT
				v[0]->invalidate(nullptr);

				// ASSERT
				assert_equal(create_rect(1, 7, 100, 100), get_update_rect(hwnd));

				// INIT
				::ValidateRect(hwnd, NULL);

				// ACT
				v[1]->invalidate(nullptr);

				// ASSERT
				assert_equal(create_rect(50, 50, 110, 105), get_update_rect(hwnd));
			}


			test( NewViewsAreNotAttachedWhenNoHierarchyChangesAreReportedOnLayout )
			{
				// INIT
				const auto v = make_shared< mocks::logging_key_input<view> >();
				placed_view pv = {	v, nullptr, create_rect(10, 15, 40, 50), 1	};
				const auto hwnd = create_window(true);
				win32::view_host vh(hwnd, context);
				const auto ctl = make_shared<mocks::control>();
				shared_ptr<void> capture_handle;

				vh.set_root(ctl);
				ctl->views.push_back(pv);

				// ACT
				ctl->layout_changed(false);
				::ValidateRect(hwnd, NULL);
				v->capture(capture_handle);
				v->invalidate(nullptr);

				// ASSERT
				assert_is_empty(v->events);
				assert_null(capture_handle);
				assert_is_false(!!::GetUpdateRect(hwnd, NULL, FALSE));
			}


			test( NewViewsAreAttachedWhenHierarchyChangeIsReportedOnLayout )
			{
				// INIT
				const auto v = make_shared< mocks::logging_key_input<view> >();
				placed_view pv = {	v, nullptr, create_rect(10, 15, 40, 50), 1	};
				const auto hwnd = create_window(true);
				win32::view_host vh(hwnd, context);
				const auto ctl = make_shared<mocks::control>();
				shared_ptr<void> capture_handle;

				vh.set_root(ctl);
				ctl->views.push_back(pv);

				// ACT
				ctl->layout_changed(true);

				// ASSERT
				mocks::keyboard_event reference[] = {
					{	mocks::keyboard_event::focusin, 0, 0	},
				};

				assert_equal(reference, v->events);

				// INIT
				::ValidateRect(hwnd, NULL);

				// ACT
				v->capture(capture_handle);
				v->invalidate(nullptr);

				// ASSERT
				assert_not_null(capture_handle);
				assert_equal(create_rect(10, 15, 40, 50), get_update_rect(hwnd));
			}


			test( MouseEventsAreDispatchedCorrespondingly )
			{
				// INIT
				const auto v = make_shared< mocks::logging_mouse_input<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 100)	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				::MoveWindow(hwnd, 0, 0, 100, 100, TRUE);

				ctl->views.push_back(pv);
				vh.set_root(ctl);

				// ACT
				::SendMessage(hwnd, WM_LBUTTONDOWN, 0, pack_coordinates(13, 92));
				::SendMessage(hwnd, WM_LBUTTONUP, 0, pack_coordinates(11, 90));
				::SendMessage(hwnd, WM_LBUTTONDOWN, 0, pack_coordinates(10, 10));
				::SendMessage(hwnd, WM_LBUTTONDBLCLK, 0, pack_coordinates(12, 11));
				::SendMessage(hwnd, WM_LBUTTONDBLCLK, 0, pack_coordinates(10, 13));
				::SendMessage(hwnd, WM_MOUSEWHEEL, pack_wheel(23), pack_screen_coordinates(hwnd, 29, 17));
				::SendMessage(hwnd, WM_MOUSEWHEEL, pack_wheel(-3), pack_screen_coordinates(hwnd, 80, 70));

				// ASSERT
				mocks::mouse_event events1[] = {
					mocks::me_enter(),
					mocks::me_down(mouse_input::left, 0, 13, 92),
					mocks::me_up(mouse_input::left, 0, 11, 90),
					mocks::me_down(mouse_input::left, 0, 10, 10),
					mocks::me_double_click(mouse_input::left, 0, 12, 11),
					mocks::me_double_click(mouse_input::left, 0, 10, 13),
					mocks::me_scroll(0, 29, 17, 0, 23),
					mocks::me_scroll(0, 80, 70, 0, -3),
				};

				assert_equal(events1, v->events_log);

				// INIT
				v->events_log.clear();

				// ACT
				::SendMessage(hwnd, WM_RBUTTONDOWN, 0, pack_coordinates(13, 92));
				::SendMessage(hwnd, WM_RBUTTONUP, 0, pack_coordinates(11, 90));
				::SendMessage(hwnd, WM_RBUTTONDOWN, 0, pack_coordinates(1, 1));
				::SendMessage(hwnd, WM_RBUTTONDBLCLK, 0, pack_coordinates(12, 11));
				::SendMessage(hwnd, WM_RBUTTONDBLCLK, 0, pack_coordinates(10, 13));
				::SendMessage(hwnd, WM_RBUTTONUP, 0, pack_coordinates(3, 7));
				::SendMessage(hwnd, WM_MOUSEHWHEEL, pack_wheel(7), pack_screen_coordinates(hwnd, 1, 2));
				::SendMessage(hwnd, WM_MOUSEHWHEEL, pack_wheel(-11), pack_screen_coordinates(hwnd, 71, 17));
				::SendMessage(hwnd, WM_MOUSEWHEEL, pack_wheel(1), pack_screen_coordinates(hwnd, 5, 9));

				// ASSERT
				mocks::mouse_event events2[] = {
					mocks::me_down(mouse_input::right, 0, 13, 92),
					mocks::me_up(mouse_input::right, 0, 11, 90),
					mocks::me_down(mouse_input::right, 0, 1, 1),
					mocks::me_double_click(mouse_input::right, 0, 12, 11),
					mocks::me_double_click(mouse_input::right, 0, 10, 13),
					mocks::me_up(mouse_input::right, 0, 3, 7),
					mocks::me_scroll(0, 1, 2, 7, 0),
					mocks::me_scroll(0, 71, 17, -11, 0),
					mocks::me_scroll(0, 5, 9, 0, 1),
				};

				assert_equal(events2, v->events_log);

				// INIT
				v->events_log.clear();

				// ACT
				::SendMessage(hwnd, WM_MOUSEMOVE, 0, pack_coordinates(91, 12));
				::SendMessage(hwnd, WM_MOUSEMOVE, 0, pack_coordinates(13, 41));

				// ASSERT
				mocks::mouse_event events3[] = {
					mocks::me_move(0, 91, 12),
					mocks::me_move(0, 13, 41),
				};

				assert_equal(events3, v->events_log);
			}


			test( ScrollEventsAreTranslatedToWindowClient )
			{
				// INIT
				const auto v = make_shared< mocks::logging_mouse_input<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 50)	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				ctl->views.push_back(pv);
				vh.set_root(ctl);

				::MoveWindow(hwnd, 17, 151, 100, 60, TRUE);

				// ACT
				::SendMessage(hwnd, WM_MOUSEWHEEL, pack_wheel(1), pack_screen_coordinates(hwnd, 83, 10));
				::SendMessage(hwnd, WM_MOUSEHWHEEL, pack_wheel(-2), pack_screen_coordinates(hwnd, 50, 37));

				// ASSERT
				mocks::mouse_event events1[] = {
					mocks::me_enter(),
					mocks::me_scroll(0, 83, 10, 0, 1),
					mocks::me_scroll(0, 50, 37, -2, 0),
				};

				assert_equal(events1, v->events_log);

				// ACT
				::MoveWindow(hwnd, 10, 20, 100, 50, TRUE);
				v->events_log.clear();

				// ACT
				::SendMessage(hwnd, WM_MOUSEHWHEEL, pack_wheel(1), pack_screen_coordinates(hwnd, 17, 18));
				::SendMessage(hwnd, WM_MOUSEWHEEL, pack_wheel(-2), pack_screen_coordinates(hwnd, 50, 40));

				// ASSERT
				mocks::mouse_event events2[] = {
					mocks::me_scroll(0, 17, 18, 1, 0),
					mocks::me_scroll(0, 50, 40, 0, -2),
				};

				assert_equal(events2, v->events_log);
			}


			test( MouseEventsAreDispatchedCorrespondinglyWithModifiers )
			{
				// INIT
				const auto v = make_shared< mocks::logging_mouse_input<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 50)	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				ctl->views.push_back(pv);
				vh.set_root(ctl);

				::MoveWindow(hwnd, 0, 0, 150, 100, TRUE);

				// ACT
				::SendMessage(hwnd, WM_LBUTTONDOWN, MK_CONTROL, pack_coordinates(0, 0));
				::SendMessage(hwnd, WM_LBUTTONUP, MK_CONTROL | MK_RBUTTON, pack_coordinates(0, 0));
				::SendMessage(hwnd, WM_LBUTTONDBLCLK, MK_CONTROL | MK_MBUTTON, pack_coordinates(0, 0));
				::SendMessage(hwnd, WM_RBUTTONDOWN, MK_CONTROL | MK_LBUTTON, pack_coordinates(0, 0));
				::SendMessage(hwnd, WM_RBUTTONUP, MK_CONTROL, pack_coordinates(0, 0));
				::SendMessage(hwnd, WM_RBUTTONDBLCLK, MK_CONTROL, pack_coordinates(0, 0));
				::SendMessage(hwnd, WM_LBUTTONDOWN, MK_CONTROL | MK_SHIFT, pack_coordinates(0, 0));
				::SendMessage(hwnd, WM_LBUTTONUP, 0, pack_coordinates(0, 0));
				::SendMessage(hwnd, WM_MOUSEMOVE, MK_SHIFT, pack_coordinates(10, 1));
				::SendMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, pack_coordinates(10, 11));
				::SendMessage(hwnd, WM_MOUSEHWHEEL, pack_wheel(-11) | MK_LBUTTON, pack_screen_coordinates(hwnd, 71, 17));
				::SendMessage(hwnd, WM_MOUSEWHEEL, pack_wheel(1) | MK_CONTROL, pack_screen_coordinates(hwnd, 5, 9));

				// ASSERT
				mocks::mouse_event reference[] = {
					mocks::me_enter(),
					mocks::me_down(mouse_input::left, keyboard_input::control, 0, 0),
					mocks::me_up(mouse_input::left, keyboard_input::control | mouse_input::right, 0, 0),
					mocks::me_double_click(mouse_input::left, keyboard_input::control | mouse_input::middle, 0, 0),
					mocks::me_down(mouse_input::right, keyboard_input::control | mouse_input::left, 0, 0),
					mocks::me_up(mouse_input::right, keyboard_input::control, 0, 0),
					mocks::me_double_click(mouse_input::right, keyboard_input::control, 0, 0),
					mocks::me_down(mouse_input::left, keyboard_input::control | keyboard_input::shift, 0, 0),
					mocks::me_up(mouse_input::left, 0, 0, 0),
					mocks::me_move(keyboard_input::shift, 10, 1),
					mocks::me_move(mouse_input::left, 10, 11),
					mocks::me_scroll(mouse_input::left, 71, 17, -11, 0),
					mocks::me_scroll(keyboard_input::control, 5, 9, 0, 1),
				};

				assert_equal(reference, v->events_log);
			}


			test( RequestingCaptureSetsHandleAndSetsUnderlyingCapture )
			{
				// INIT
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);
				mouse_router_host &mrh = vh;
				shared_ptr<void> h;

				::MoveWindow(hwnd, 0, 0, 150, 100, TRUE);
				::SetFocus(hwnd);

				// ACT
				h = mrh.capture_mouse();

				// ASSERT
				assert_not_null(h);
				assert_equal(hwnd, ::GetCapture());
			}


			test( ReleasingRelinquishedCaptureDoesNotAffectNewCapture )
			{
				// INIT
				const auto hwnd1 = create_window(true, 100, 100);
				const auto hwnd2 = create_window(true, 100, 100);
				win32::view_host vh1(hwnd1, context);
				win32::view_host vh2(hwnd2, context);

				::SetFocus(hwnd1);
				auto h1 = vh1.capture_mouse();

				// ACT
				auto h2 = vh2.capture_mouse();
				h1.reset();

				// ASSERT
				assert_equal(hwnd2, ::GetCapture());
			}


			test( ResettingHandleReleasesTheCapture )
			{
				// INIT
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);
				mouse_router_host &mrh = vh;
				shared_ptr<void> h;

				::MoveWindow(hwnd, 0, 0, 150, 100, TRUE);
				::SetFocus(hwnd);
				h = mrh.capture_mouse();

				// ACT
				h.reset();

				// ASSERT
				assert_null(::GetCapture());
			}


			test( HostReactsOnCaptureRequestsFromViews )
			{
				// INIT
				const auto v = make_shared<view>();
				placed_view pv = { v, nullptr, create_rect(0, 0, 100, 100), };
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);
				shared_ptr<void> h;

				ctl->views.push_back(pv);
				vh.set_root(ctl);

				// ACT
				v->capture(h);

				// ASSERT
				assert_equal(hwnd, ::GetCapture());
			}


			test( FirstMouseMoveGeneratesMouseEnterPriorTheMouseMove )
			{
				// INIT
				const auto v = make_shared< mocks::logging_mouse_input<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 50)	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				ctl->views.push_back(pv);
				vh.set_root(ctl);

				// ACT
				::SendMessage(hwnd, WM_MOUSEMOVE, 0, pack_coordinates(0, 0));

				// ASSERT
				mocks::mouse_event events[] = {
					mocks::me_enter(),
					mocks::me_move(0, 0, 0),
				};

				assert_equal(events, v->events_log);

				// ACT
				::SendMessage(hwnd, WM_MOUSEMOVE, 0, pack_coordinates(7, 17));

				// ASSERT
				mocks::mouse_event events2[] = {
					mocks::me_enter(),
					mocks::me_move(0, 0, 0),
					mocks::me_move(0, 7, 17),
				};

				assert_equal(events2, v->events_log);
			}


			test( MouseLeaveEventCausesNotification )
			{
				// INIT
				const auto v = make_shared< mocks::logging_mouse_input<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 50)	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				ctl->views.push_back(pv);
				vh.set_root(ctl);

				::SendMessage(hwnd, WM_MOUSEMOVE, 0, pack_coordinates(1, 20));

				// ACT
				::SendMessage(hwnd, WM_MOUSELEAVE, 0, 0);

				// ASSERT
				mocks::mouse_event events[] = {
					mocks::me_enter(),
					mocks::me_move(0, 1, 20),
					mocks::me_leave(),
				};

				assert_equal(events, v->events_log);
			}


			test( MouseEnterCanBeGeneratedAgainAfterMouseLeave )
			{
				// INIT
				const auto v = make_shared< mocks::logging_mouse_input<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 50)	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				ctl->views.push_back(pv);
				vh.set_root(ctl);

				::SendMessage(hwnd, WM_MOUSEMOVE, 0, pack_coordinates(0, 0));
				::SendMessage(hwnd, WM_MOUSELEAVE, 0, 0);

				// ACT
				::SendMessage(hwnd, WM_MOUSEMOVE, 0, pack_coordinates(1, 2));

				// ASSERT
				mocks::mouse_event events[] = {
					mocks::me_enter(),
					mocks::me_move(0, 0, 0),
					mocks::me_leave(),
					mocks::me_enter(),
					mocks::me_move(0, 1, 2),
				};

				assert_equal(events, v->events_log);
			}


			test( KeyMessagesGetKeyInputCallbacksCalled )
			{
				// INIT
				const auto v = make_shared< mocks::logging_key_input<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 50), 1	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				win32::view_host vh(hwnd, context);

				ctl->views.push_back(pv);
				vh.set_root(ctl);

				// ACT
				::SendMessage(hwnd, WM_KEYDOWN, VK_LEFT, 0);
				::SendMessage(hwnd, WM_KEYDOWN, VK_NEXT, 0);

				// ASSERT
				mocks::keyboard_event reference1[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::left, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::page_down, 0 },
				};

				assert_equal(reference1, v->events);

				// ACT
				::SendMessage(hwnd, WM_KEYUP, VK_RIGHT, 0);
				::SendMessage(hwnd, WM_KEYDOWN, VK_UP, 0);
				::SendMessage(hwnd, WM_KEYUP, VK_DOWN, 0);
				::SendMessage(hwnd, WM_KEYUP, VK_PRIOR, 0);
				::SendMessage(hwnd, WM_KEYDOWN, VK_HOME, 0);
				::SendMessage(hwnd, WM_KEYDOWN, VK_END, 0);
				::SendMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
				::SendMessage(hwnd, WM_KEYDOWN, 'A', 0);
				::SendMessage(hwnd, WM_KEYDOWN, 'a', 0);
				::SendMessage(hwnd, WM_KEYUP, 'a', 0);

				// ASSERT
				mocks::keyboard_event reference2[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::left, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::page_down, 0 },
					{ mocks::keyboard_event::keyup, keyboard_input::right, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::up, 0 },
					{ mocks::keyboard_event::keyup, keyboard_input::down, 0 },
					{ mocks::keyboard_event::keyup, keyboard_input::page_up, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::home, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::end, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::enter, 0 },
					{ mocks::keyboard_event::keydown, 'A', 0 },
					{ mocks::keyboard_event::keydown, 'a', 0 },
					{ mocks::keyboard_event::keyup, 'a', 0 },
				};

				assert_equal(reference2, v->events);
			}


			test( ControlAndShiftAreNotDeliveredAsKeyDownAndKeyUp )
			{
				// INIT
				const auto v = make_shared< mocks::logging_key_input<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 50), 1	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				win32::view_host vh(hwnd, context);

				ctl->views.push_back(pv);
				vh.set_root(ctl);
				v->events.clear();

				// ACT
				::SendMessage(hwnd, WM_KEYDOWN, VK_CONTROL, 0);
				::SendMessage(hwnd, WM_KEYUP, VK_SHIFT, 0);

				// ASSERT
				assert_is_empty(v->events);
			}


			test( KeyMessageIsAccompaniedWithShiftAndControl )
			{
				// INIT
				const auto v = make_shared< mocks::logging_key_input<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 50), 1	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				win32::view_host vh(hwnd, context);

				ctl->views.push_back(pv);
				vh.set_root(ctl);
				v->events.clear();

				// ACT
				::SendMessage(hwnd, WM_KEYDOWN, VK_CONTROL, 0);
				::SendMessage(hwnd, WM_KEYDOWN, 'A', 0);
				::SendMessage(hwnd, WM_KEYUP, VK_LEFT, 0);

				// ASSERT
				mocks::keyboard_event reference1[] = {
					{ mocks::keyboard_event::keydown, 'A', keyboard_input::control },
					{ mocks::keyboard_event::keyup, keyboard_input::left, keyboard_input::control },
				};

				assert_equal(reference1, v->events);

				// ACT
				::SendMessage(hwnd, WM_KEYDOWN, VK_SHIFT, 0);
				::SendMessage(hwnd, WM_KEYDOWN, 'B', 0);
				::SendMessage(hwnd, WM_KEYUP, 'N', 0);

				// ASSERT
				mocks::keyboard_event reference2[] = {
					{ mocks::keyboard_event::keydown, 'A', keyboard_input::control },
					{ mocks::keyboard_event::keyup, keyboard_input::left, keyboard_input::control },
					{ mocks::keyboard_event::keydown, 'B', keyboard_input::control | keyboard_input::shift },
					{ mocks::keyboard_event::keyup, 'N', keyboard_input::control | keyboard_input::shift },
				};

				assert_equal(reference2, v->events);
			}


			test( KeyMessageIsNotAccompaniedWithShiftAndControlAfterShiftAndControlAreReleased )
			{
				// INIT
				const auto v = make_shared< mocks::logging_key_input<view> >();
				const placed_view pv = {	v, nullptr, create_rect(0, 0, 100, 50), 1	};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true);
				win32::view_host vh(hwnd, context);

				ctl->views.push_back(pv);
				vh.set_root(ctl);
				v->events.clear();

				::SendMessage(hwnd, WM_KEYDOWN, VK_CONTROL, 0);
				::SendMessage(hwnd, WM_KEYDOWN, VK_SHIFT, 0);

				// ACT
				::SendMessage(hwnd, WM_KEYUP, VK_CONTROL, 0);
				::SendMessage(hwnd, WM_KEYDOWN, 'T', 0);
				::SendMessage(hwnd, WM_KEYUP, VK_HOME, 0);

				// ASSERT
				mocks::keyboard_event reference1[] = {
					{ mocks::keyboard_event::keydown, 'T', keyboard_input::shift },
					{ mocks::keyboard_event::keyup, keyboard_input::home, keyboard_input::shift },
				};

				assert_equal(reference1, v->events);

				// ACT
				::SendMessage(hwnd, WM_KEYUP, VK_SHIFT, 0);
				::SendMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
				::SendMessage(hwnd, WM_KEYDOWN, VK_END, 0);

				// ASSERT
				mocks::keyboard_event reference2[] = {
					{ mocks::keyboard_event::keydown, 'T', keyboard_input::shift },
					{ mocks::keyboard_event::keyup, keyboard_input::home, keyboard_input::shift },
					{ mocks::keyboard_event::keydown, keyboard_input::enter, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::end, 0 },
				};

				assert_equal(reference2, v->events);
			}


			test( NativeViewWindowsAreReparented )
			{
				// INIT
				const placed_view pv[] = {
					{	nullptr, make_shared<mocks::native_view_window>(), create_rect(0, 0, 20, 50)	},
					{	nullptr, make_shared<mocks::native_view_window>(), create_rect(15, 30, 70, 50)	},
				};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd1 = create_window(true, 100, 100);
				win32::view_host vh1(hwnd1, context);

				ctl->views.assign(begin(pv), end(pv));

				// ACT
				vh1.set_root(ctl);

				// ASSERT
				assert_equal(hwnd1, ::GetParent(pv[0].native->get_window()));
				assert_equal(hwnd1, ::GetParent(pv[1].native->get_window()));

				// INIT
				const auto hwnd2 = create_window(true, 100, 100);
				win32::view_host vh2(hwnd2, context);

				// ACT
				vh2.set_root(ctl);

				// ASSERT
				assert_equal(hwnd2, ::GetParent(pv[0].native->get_window()));
				assert_equal(hwnd2, ::GetParent(pv[1].native->get_window()));
			}


			test( NativeViewWindowsAreResizedAccordinglyToTheirLocations )
			{
				// INIT
				placed_view pv[] = {
					{	nullptr, make_shared<mocks::native_view_window>(), create_rect(10, 17, 110, 151)	},
					{	nullptr, make_shared<mocks::native_view_window>(), create_rect(100, 1, 191, 201)	},
					{	nullptr, make_shared<mocks::native_view_window>(), create_rect(10, 10, 110, 210)	},
				};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				ctl->views.assign(begin(pv), end(pv));

				// ACT
				vh.set_root(ctl);

				// ASSERT
				assert_equal(rect(10, 17, 100, 134), get_window_rect(pv[0].native->get_window()));
				assert_equal(rect(100, 1, 91, 200), get_window_rect(pv[1].native->get_window()));
				assert_equal(rect(10, 10, 100, 200), get_window_rect(pv[2].native->get_window()));

				// INIT
				ctl->views[0].location = create_rect(0, 0, 100, 30);

				// ACT
				::MoveWindow(hwnd, 0, 0, 200, 200, FALSE);

				// ASSERT
				assert_equal(rect(0, 0, 100, 30), get_window_rect(pv[0].native->get_window()));
			}


			test( SettingViewResizesItToAClient )
			{
				// INIT
				auto ctl = make_shared<mocks::control>();
				auto hwnd = create_window();
				win32::view_host vh(hwnd, context);

				// ACT
				vh.set_root(ctl);

				// ASSERT
				assert_equal(1u, ctl->size_log.size());
				assert_equal(get_client_size(hwnd), ctl->size_log.back());

				// ACT
				vh.set_root(nullptr);
				::MoveWindow(hwnd, 0, 0, 203, 150, TRUE);
				vh.set_root(ctl);

				// ASSERT
				assert_equal(2u, ctl->size_log.size());
				assert_equal(get_client_size(hwnd), ctl->size_log.back());
			}


			test( FocusMovesAccordinglyToTabOrderAfterFocusRequest )
			{
				// INIT
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
				};
				const placed_view pv[] = {
					{	v[0], nullptr, create_rect(10, 17, 110, 151), 2	},
					{	v[1], nullptr, create_rect(100, 1, 191, 201), 1	},
					{	v[2], nullptr, create_rect(10, 10, 110, 210), 3	},
				};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				ctl->views.assign(begin(pv), end(pv));
				vh.set_root(ctl);

				// ACT
				::SendMessage(hwnd, WM_KEYDOWN, VK_TAB, 0);

				// ASSERT
				mocks::keyboard_event reference_in[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
				};
				mocks::keyboard_event reference_inout[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
				};

				assert_equal(reference_in, v[0]->events);
				assert_equal(reference_inout, v[1]->events);
				assert_is_empty(v[2]->events);

				// ACT
				::SendMessage(hwnd, WM_KEYDOWN, VK_SHIFT, 0);
				::SendMessage(hwnd, WM_KEYDOWN, VK_TAB, 0);
				::SendMessage(hwnd, WM_KEYUP, VK_SHIFT, 0);

				// ASSERT
				mocks::keyboard_event reference_inoutin[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
					{ mocks::keyboard_event::focusin, 0, 0 },
				};

				assert_equal(reference_inout, v[0]->events);
				assert_equal(reference_inoutin, v[1]->events);
				assert_is_empty(v[2]->events);
			}


			test( FocusIsSetToTheControlSpecifiedByRequest )
			{
				// INIT
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
				};
				const placed_view pv[] = {
					{	v[0], nullptr, create_rect(10, 17, 110, 151), 2	},
					{	v[1], nullptr, create_rect(100, 1, 191, 201), 1	},
					{	v[2], nullptr, create_rect(10, 10, 110, 210), 3	},
				};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);
				mouse_router_host &mrhost = vh;

				ctl->views.assign(begin(pv), end(pv));
				vh.set_root(ctl);

				// ACT
				mrhost.request_focus(v[0]);

				// ASSERT
				mocks::keyboard_event reference_inout[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
				};
				mocks::keyboard_event reference_in[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
				};

				assert_equal(reference_in, v[0]->events);
				assert_equal(reference_inout, v[1]->events);
				assert_is_empty(v[2]->events);

				// ACT
				mrhost.request_focus(v[1]);

				// ASSERT
				mocks::keyboard_event reference_inoutin[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
					{ mocks::keyboard_event::focusin, 0, 0 },
				};

				assert_equal(reference_inout, v[0]->events);
				assert_equal(reference_inoutin, v[1]->events);
				assert_is_empty(v[2]->events);
			}


			test( WindowObtainsFocusOnFocusRequest )
			{
				// INIT
				const auto v = make_shared<view>();
				const placed_view pv[] = {
					{	v, nullptr, create_rect(100, 1, 191, 201), 1	},
					{	nullptr, make_shared<mocks::native_view_window>(), create_rect(1, 1, 191, 201), 2	},
				};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);
				mouse_router_host &mrhost = vh;

				ctl->views.assign(begin(pv), end(pv));
				vh.set_root(ctl);
				::SetFocus(pv[1].native->get_window());

				// ACT
				mrhost.request_focus(v);

				// ASSERT
				assert_equal(hwnd, ::GetFocus());
			}


			test( FocusedViewIsNotifiedOfFocusLostOnWindowFocusLost )
			{
				// INIT
				const auto v = make_shared< mocks::logging_key_input<view> >();
				const placed_view pv[] = {
					{	v, nullptr, create_rect(100, 1, 191, 201), 1	},
					{	nullptr, make_shared<mocks::native_view_window>(), create_rect(1, 1, 191, 201), 2	},
				};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);
//				mouse_router_host &mrhost = vh;

				ctl->views.assign(begin(pv), end(pv));
				vh.set_root(ctl);

				v->events.clear();

				// ACT
				::SetFocus(pv[1].native->get_window());

				// ASSERT
				mocks::keyboard_event reference[] = {
					{ mocks::keyboard_event::focusout, 0, 0 },
				};

				assert_equal(reference, v->events);
			}


			test( WindowDoesNotObtainFocusOnFocusRequestForANonKeyInputView )
			{
				// INIT
				const auto v = make_shared<view>();
				const placed_view pv[] = {
					{	nullptr, make_shared<mocks::native_view_window>(), create_rect(1, 1, 191, 201)	},
				};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);
				mouse_router_host &mrhost = vh;

				ctl->views.assign(begin(pv), end(pv));
				vh.set_root(ctl);
				::SetFocus(pv[0].native->get_window());

				// ACT
				mrhost.request_focus(v);

				// ASSERT
				assert_equal(pv[0].native->get_window(), ::GetFocus());
			}


			test( SetCursorMessageIsStopped )
			{
				// INIT
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				// ACT / ASSERT
				assert_equal(TRUE, ::SendMessage(hwnd, WM_SETCURSOR, reinterpret_cast<WPARAM>(hwnd),
					MAKELONG(HTCLIENT, 0)));
			}


			test( SetCursorMessageForAChildIsNotStopped )
			{
				// INIT
				const auto v = make_shared<view>();
				const placed_view pv[] = {
					{	nullptr, make_shared<mocks::native_view_window>(), create_rect(1, 1, 191, 201)	},
				};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				ctl->views.assign(begin(pv), end(pv));
				vh.set_root(ctl);

				// ACT / ASSERT
				assert_equal(FALSE, ::SendMessage(hwnd, WM_SETCURSOR, reinterpret_cast<WPARAM>(pv[0].native->get_window()),
					MAKELONG(HTCLIENT, 0)));
			}


			test( CursorIsSetToArrowOnMouseEnter )
			{
				// INIT
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);

				// INIT / ASSERT
				assert_equal(0u, cursor_manager->stack_level);

				// ACT
				::SendMessage(hwnd, WM_MOUSEMOVE, 0, pack_coordinates(10, 11));

				// ASSERT
				assert_equal(1u, cursor_manager->stack_level);
				assert_equal(1u, cursor_manager->attempts);
				assert_equal(cursor_manager->cursors[cursor_manager::arrow], cursor_manager->recently_set);

				// ACT
				::SendMessage(hwnd, WM_MOUSEMOVE, 0, pack_coordinates(20, 11));

				// ASSERT
				assert_equal(1u, cursor_manager->stack_level);
				assert_equal(1u, cursor_manager->attempts);
				assert_equal(cursor_manager->cursors[cursor_manager::arrow], cursor_manager->recently_set);

				// ACT
				::SendMessage(hwnd, WM_MOUSELEAVE, 0, 0);

				// ASSERT
				assert_equal(0u, cursor_manager->stack_level);
				assert_equal(1u, cursor_manager->attempts);
			}


			test( SettingFocusToANativeViewFocusesItsWindow )
			{
				// INIT
				const auto v = make_shared<view>();
				const auto nv = make_shared<mocks::native_view_window>();
				const placed_view pv[] = {
					{	v, nullptr, {}, 1	},
					{	nullptr, nv, create_rect(10, 17, 110, 151), 2	},
				};
				const auto ctl = make_shared<mocks::control>();
				const auto hwnd = create_window(true, 100, 100);
				win32::view_host vh(hwnd, context);
				keyboard_router_host &krhost = vh;

				ctl->views.assign(begin(pv), end(pv));
				vh.set_root(ctl);

				// INIT / ASSERT
				assert_not_equal(nv->get_window(), ::GetFocus());

				// ACT
				krhost.set_focus(*nv);

				// INIT / ASSERT
				assert_equal(nv->get_window(), ::GetFocus());
			}

		end_test_suite
	}
}
