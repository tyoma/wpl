#include <wpl/view_host.h>
#include <wpl/win32/controls.h>

#include "helpers-visual.h"
#include "helpers-win32.h"
#include "Mockups.h"
#include "MockupsNative.h"

#include <tchar.h>
#include <ut/assert.h>
#include <ut/test.h>
#include <windows.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			class hosting_window : noncopyable
			{
			public:
				hosting_window()
					: hwnd(::CreateWindow(_T("static"), NULL, WS_POPUP | WS_VISIBLE, 0, 0, 100, 70, NULL, NULL, NULL,
						NULL)), host(wrap_view_host(hwnd))
				{	}

				~hosting_window()
				{
					host.reset();
					::DestroyWindow(hwnd);
				}

			public:
				const HWND hwnd;
				shared_ptr<wpl::view_host> host;
			};
		}

		begin_test_suite( NativeViewHostTests )
			test( ResizingHostWindowLeadsToContentResize )
			{
				// INIT
				shared_ptr<mocks::logging_visual<view>> v(new mocks::logging_visual<view>());
				hosting_window f;
				RECT rc;

				f.host->set_view(v);
				v->resize_log.clear();

				// ACT
				::MoveWindow(f.hwnd, 0, 0, 117, 213, TRUE);

				// ASSERT
				::GetClientRect(f.hwnd, &rc);

				assert_equal(1u, v->resize_log.size());
				assert_equal(make_pair((int)rc.right, (int)rc.bottom), v->resize_log[0]);

				// ACT
				::MoveWindow(f.hwnd, 27, 190, 531, 97, TRUE);

				// ASSERT
				::GetClientRect(f.hwnd, &rc);

				assert_equal(2u, v->resize_log.size());
				assert_equal(make_pair((int)rc.right, (int)rc.bottom), v->resize_log[1]);
			}


			test( MovingHostWindowDoesNotRaiseResizeSignal )
			{
				// INIT
				shared_ptr<mocks::logging_visual<view>> v(new mocks::logging_visual<view>());
				hosting_window f;

				f.host->set_view(v);

				::MoveWindow(f.hwnd, 0, 0, 117, 213, TRUE);
				v->resize_log.clear();

				// ACT
				::MoveWindow(f.hwnd, 23, 91, 117, 213, TRUE);
				::MoveWindow(f.hwnd, 53, 91, 117, 213, TRUE);
				::MoveWindow(f.hwnd, 53, 32, 117, 213, TRUE);
				::MoveWindow(f.hwnd, 23, 100, 117, 213, TRUE);

				// ASSERT
				assert_is_empty(v->resize_log);
			}


			test( RedrawingHostWindowSuppliesBitmapOfTheSizeCorrespondingToRedrawAreaToTheContent )
			{
				// INIT
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>());
				hosting_window f;
				RECT rc = { 0, 0, 100, 60 };

				f.host->set_view(v);

				::MoveWindow(f.hwnd, 0, 0, 1000, 1000, TRUE);
				::ValidateRect(f.hwnd, NULL);

				// ACT
				::RedrawWindow(f.hwnd, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

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
				::RedrawWindow(f.hwnd, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				// ASSERT
				assert_equal(2u, v->surface_size_log.size());
				assert_is_true(115 <= v->surface_size_log[1].first);
				assert_is_true(135 > v->surface_size_log[1].first);
				assert_is_true(71 <= v->surface_size_log[1].second);
				assert_is_true(76 > v->surface_size_log[1].second);
			}


			test( WindowPassedToDrawCorrespondsToInvalidArea )
			{
				// INIT
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>());
				hosting_window f;
				RECT rc = { 11, 17, 100, 60 };

				f.host->set_view(v);

				::MoveWindow(f.hwnd, 0, 0, 1000, 1000, TRUE);
				::ValidateRect(f.hwnd, NULL);

				// ACT
				::RedrawWindow(f.hwnd, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				// ASSERT
				assert_equal(1u, v->update_area_log.size());
				assert_equal(make_rect(11, 17, 100, 60), v->update_area_log[0]);

				// INIT
				RECT rc2 = { 101, 107, 150, 260 };

				// ACT
				::RedrawWindow(f.hwnd, &rc2, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				// ASSERT
				assert_equal(2u, v->update_area_log.size());
				assert_equal(make_rect(101, 107, 150, 260), v->update_area_log[1]);
			}


			test( TheSameRasterizerIsSuppliedToDrawingProcedure )
			{
				// INIT
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>());
				hosting_window f;

				f.host->set_view(v);

				::MoveWindow(f.hwnd, 0, 0, 100, 50, TRUE);

				// ACT
				::RedrawWindow(f.hwnd, NULL, NULL, RDW_UPDATENOW);
				::RedrawWindow(f.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				// ASSERT
				assert_equal(2u, v->rasterizers_log.size());
				assert_not_null(v->rasterizers_log[0]);
				assert_equal(v->rasterizers_log[0], v->rasterizers_log[1]);
			}


			test( RasterizerIsResetOnDraw )
			{
				// INIT
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>());
				hosting_window f;

				f.host->set_view(v);

				::MoveWindow(f.hwnd, 0, 0, 100, 50, TRUE);

				// ACT / ASSERT (done inside)
				::RedrawWindow(f.hwnd, NULL, NULL, RDW_UPDATENOW);
				::RedrawWindow(f.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}


			test( InvalidateSignalMarksAreaAsInvalid )
			{
				// INIT
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>());
				hosting_window f;
				RECT rc, invalid;

				f.host->set_view(v);

				::MoveWindow(f.hwnd, 0, 0, 1000, 1000, TRUE);
				::GetClientRect(f.hwnd, &rc);

				// ACT
				v->invalidate(0);

				// ASSERT
				assert_is_true(!!::GetUpdateRect(f.hwnd, &invalid, FALSE));
				assert_equal(rc, invalid);

				// INIT
				rect_i rc2 = { 10, 13, 131, 251 };
				::ValidateRect(f.hwnd, NULL);

				// ACT
				v->invalidate(&rc2);

				// ASSERT
				RECT rc3 = { 10, 13, 131, 251 };

				assert_is_true(!!::GetUpdateRect(f.hwnd, &invalid, FALSE));
				assert_equal(rc3, invalid);

				// INIT
				rect_i rc4 = { 17, 19, 130, 250 };
				::ValidateRect(f.hwnd, NULL);

				// ACT
				v->invalidate(&rc4);

				// ASSERT
				RECT rc5 = { 17, 19, 130, 250 };

				assert_is_true(!!::GetUpdateRect(f.hwnd, &invalid, FALSE));
				assert_equal(rc5, invalid);
			}


			test( MouseEventsAreDispatchedCorrespondingly )
			{
				// INIT
				shared_ptr< mocks::logging_mouse_input<view> > v(new mocks::logging_mouse_input<view>());
				hosting_window f;

				f.host->set_view(v);

				::MoveWindow(f.hwnd, 0, 0, 100, 50, TRUE);

				// ACT
				::SendMessage(f.hwnd, WM_LBUTTONDOWN, 0, pack_coordinates(-13, 1002));
				::SendMessage(f.hwnd, WM_LBUTTONUP, 0, pack_coordinates(11222, -200));
				::SendMessage(f.hwnd, WM_LBUTTONDOWN, 0, pack_coordinates(10, 10));
				::SendMessage(f.hwnd, WM_LBUTTONDBLCLK, 0, pack_coordinates(12, 11));
				::SendMessage(f.hwnd, WM_LBUTTONDBLCLK, 0, pack_coordinates(10, 13));

				// ASSERT
				mocks::mouse_event events1[] = {
					mocks::me_down(mouse_input::left, 0, -13, 1002),
					mocks::me_up(mouse_input::left, 0, 11222, -200),
					mocks::me_down(mouse_input::left, 0, 10, 10),
					mocks::me_double_click(mouse_input::left, 0, 12, 11),
					mocks::me_double_click(mouse_input::left, 0, 10, 13),
				};

				assert_equal(events1, v->events_log);

				// INIT
				v->events_log.clear();

				// ACT
				::SendMessage(f.hwnd, WM_RBUTTONDOWN, 0, pack_coordinates(-13, 1002));
				::SendMessage(f.hwnd, WM_RBUTTONUP, 0, pack_coordinates(112, -11));
				::SendMessage(f.hwnd, WM_RBUTTONDOWN, 0, pack_coordinates(1, 1));
				::SendMessage(f.hwnd, WM_RBUTTONDBLCLK, 0, pack_coordinates(12, 11));
				::SendMessage(f.hwnd, WM_RBUTTONDBLCLK, 0, pack_coordinates(10, 13));
				::SendMessage(f.hwnd, WM_RBUTTONUP, 0, pack_coordinates(-3, -7));

				// ASSERT
				mocks::mouse_event events2[] = {
					mocks::me_down(mouse_input::right, 0, -13, 1002),
					mocks::me_up(mouse_input::right, 0, 112, -11),
					mocks::me_down(mouse_input::right, 0, 1, 1),
					mocks::me_double_click(mouse_input::right, 0, 12, 11),
					mocks::me_double_click(mouse_input::right, 0, 10, 13),
					mocks::me_up(mouse_input::right, 0, -3, -7),
				};

				assert_equal(events2, v->events_log);

				// INIT
				v->events_log.clear();

				// ACT
				::SendMessage(f.hwnd, WM_MOUSEMOVE, 0, pack_coordinates(-11, 12));
				::SendMessage(f.hwnd, WM_MOUSEMOVE, 0, pack_coordinates(13, -14));

				// ASSERT
				mocks::mouse_event events3[] = {
					mocks::me_enter(),
					mocks::me_move(0, -11, 12),
					mocks::me_move(0, 13, -14),
				};

				assert_equal(events3, v->events_log);
			}


			test( MouseEventsAreDispatchedCorrespondinglyWithModifiers )
			{
				// INIT
				shared_ptr< mocks::logging_mouse_input<view> > v(new mocks::logging_mouse_input<view>());
				hosting_window f;

				f.host->set_view(v);

				::MoveWindow(f.hwnd, 0, 0, 100, 50, TRUE);

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_CONTROL, 0);
				::SendMessage(f.hwnd, WM_LBUTTONDOWN, 0, pack_coordinates(0, 0));
				::SendMessage(f.hwnd, WM_LBUTTONUP, 0, pack_coordinates(0, 0));
				::SendMessage(f.hwnd, WM_LBUTTONDBLCLK, 0, pack_coordinates(0, 0));
				::SendMessage(f.hwnd, WM_RBUTTONDOWN, 0, pack_coordinates(0, 0));
				::SendMessage(f.hwnd, WM_RBUTTONUP, 0, pack_coordinates(0, 0));
				::SendMessage(f.hwnd, WM_RBUTTONDBLCLK, 0, pack_coordinates(0, 0));
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_SHIFT, 0);
				::SendMessage(f.hwnd, WM_LBUTTONDOWN, 0, pack_coordinates(0, 0));
				::SendMessage(f.hwnd, WM_KEYUP, VK_CONTROL, 0);
				::SendMessage(f.hwnd, WM_KEYUP, VK_SHIFT, 0);
				::SendMessage(f.hwnd, WM_LBUTTONUP, 0, pack_coordinates(0, 0));

				// ASSERT
				mocks::mouse_event reference[] = {
					mocks::me_down(mouse_input::left, keyboard_input::control, 0, 0),
					mocks::me_up(mouse_input::left, keyboard_input::control, 0, 0),
					mocks::me_double_click(mouse_input::left, keyboard_input::control, 0, 0),
					mocks::me_down(mouse_input::right, keyboard_input::control, 0, 0),
					mocks::me_up(mouse_input::right, keyboard_input::control, 0, 0),
					mocks::me_double_click(mouse_input::right, keyboard_input::control, 0, 0),
					mocks::me_down(mouse_input::left, keyboard_input::control | keyboard_input::shift, 0, 0),
					mocks::me_up(mouse_input::left, 0, 0, 0),
				};

				assert_equal(reference, v->events_log);
			}


			test( RequestingCaptureSetsHandleAndSetsUnderlyingCapture )
			{
				// INIT
				shared_ptr< mocks::logging_mouse_input<view> > v(new mocks::logging_mouse_input<view>());
				hosting_window f;
				shared_ptr<void> h;

				f.host->set_view(v);

				::MoveWindow(f.hwnd, 0, 0, 150, 100, TRUE);
				::SetFocus(f.hwnd);

				// ACT
				v->capture(h);

				// ASSERT
				assert_not_null(h);
				assert_equal(f.hwnd, ::GetCapture());
			}


			test( SwitchingCaptureNotifiesView )
			{
				// INIT
				shared_ptr< mocks::logging_mouse_input<view> > v1(new mocks::logging_mouse_input<view>());
				shared_ptr< mocks::logging_mouse_input<view> > v2(new mocks::logging_mouse_input<view>());
				hosting_window f1;
				hosting_window f2;
				shared_ptr<void> h1, h2;

				f1.host->set_view(v1);
				f2.host->set_view(v2);

				::SetFocus(f1.hwnd);

				v1->capture(h1);
					
				// ACT
				v2->capture(h2);

				// ASSERT
				assert_equal(1u, v1->capture_lost);
			}


			test( ResettingHandleReleasesTheCapture )
			{
				// INIT
				shared_ptr< mocks::logging_mouse_input<view> > v(new mocks::logging_mouse_input<view>());
				hosting_window f;
				shared_ptr<void> h;

				f.host->set_view(v);
				::SetFocus(f.hwnd);
				v->capture(h);

				// ACT
				h.reset();

				// ASSERT
				assert_null(::GetCapture());
			}


			test( FirstMouseMoveGeneratesMouseEnterPriorTheMouseMove )
			{
				// INIT
				shared_ptr< mocks::logging_mouse_input<view> > v(new mocks::logging_mouse_input<view>());
				hosting_window f;
				shared_ptr<void> h;

				f.host->set_view(v);

				// ACT
				::SendMessage(f.hwnd, WM_MOUSEMOVE, 0, pack_coordinates(0, 0));

				// ASSERT
				mocks::mouse_event events[] = {
					mocks::me_enter(),
					mocks::me_move(0, 0, 0),
				};

				assert_equal(events, v->events_log);

				// ACT
				::SendMessage(f.hwnd, WM_MOUSEMOVE, 0, pack_coordinates(7, 17));

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
				shared_ptr< mocks::logging_mouse_input<view> > v(new mocks::logging_mouse_input<view>());
				hosting_window f;
				shared_ptr<void> h;

				f.host->set_view(v);

				::SendMessage(f.hwnd, WM_MOUSEMOVE, 0, pack_coordinates(0, 0));

				// ACT
				::SendMessage(f.hwnd, WM_MOUSELEAVE, 0, 0);

				// ASSERT
				mocks::mouse_event events[] = {
					mocks::me_enter(),
					mocks::me_move(0, 0, 0),
					mocks::me_leave(),
				};

				assert_equal(events, v->events_log);
			}


			test( MouseEnterCanBeGeneratedAgainAfterMouseLeave )
			{
				// INIT
				shared_ptr< mocks::logging_mouse_input<view> > v(new mocks::logging_mouse_input<view>());
				hosting_window f;
				shared_ptr<void> h;

				f.host->set_view(v);

				::SendMessage(f.hwnd, WM_MOUSEMOVE, 0, pack_coordinates(0, 0));
				::SendMessage(f.hwnd, WM_MOUSELEAVE, 0, 0);

				// ACT
				::SendMessage(f.hwnd, WM_MOUSEMOVE, 0, pack_coordinates(1, 2));

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


			test( KeyboardInputIsRedirectedToTheViewIfNoTabbedControls )
			{
				// INIT
				hosting_window f;
				shared_ptr< mocks::logging_key_input<view> > v(new mocks::logging_key_input<view>);

				f.host->set_view(v);
				::ValidateRect(f.hwnd, NULL);

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);
				::SendMessage(f.hwnd, WM_KEYUP, VK_TAB, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, 'Z', 0);
				::SendMessage(f.hwnd, WM_KEYUP, 'Z', 0);

				// ASSERT
				mocks::keyboard_event reference[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::keydown, 'Z', 0 },
					{ mocks::keyboard_event::keyup, 'Z', 0 },
				};

				assert_equal(reference, v->events);
			}


			test( KeyMessagesGetKeyInputCallbacksCalled )
			{
				// INIT
				shared_ptr< mocks::logging_key_input<view> > v(new mocks::logging_key_input<view>());
				hosting_window f;

				f.host->set_view(v);
				v->request_focus(v);
				v->events.clear();

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_LEFT, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_NEXT, 0);

				// ASSERT
				mocks::keyboard_event reference1[] = {
					{ mocks::keyboard_event::keydown, keyboard_input::left, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::page_down, 0 },
				};

				assert_equal(reference1, v->events);

				// ACT
				::SendMessage(f.hwnd, WM_KEYUP, VK_RIGHT, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_UP, 0);
				::SendMessage(f.hwnd, WM_KEYUP, VK_DOWN, 0);
				::SendMessage(f.hwnd, WM_KEYUP, VK_PRIOR, 0);

				// ASSERT
				mocks::keyboard_event reference2[] = {
					{ mocks::keyboard_event::keydown, keyboard_input::left, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::page_down, 0 },
					{ mocks::keyboard_event::keyup, keyboard_input::right, 0 },
					{ mocks::keyboard_event::keydown, keyboard_input::up, 0 },
					{ mocks::keyboard_event::keyup, keyboard_input::down, 0 },
					{ mocks::keyboard_event::keyup, keyboard_input::page_up, 0 },
				};

				assert_equal(reference2, v->events);
			}


			test( ControlAndShiftAreNotDeliveredAsKeyDownAndKeyUp )
			{
				// INIT
				shared_ptr< mocks::logging_key_input<view> > v(new mocks::logging_key_input<view>());
				hosting_window f;

				f.host->set_view(v);

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_CONTROL, 0);
				::SendMessage(f.hwnd, WM_KEYUP, VK_SHIFT, 0);

				// ASSERT
				assert_is_empty(v->events);
			}


			test( KeyMessageIsAccompaniedWithShiftAndControl )
			{
				// INIT
				shared_ptr< mocks::logging_key_input<view> > v(new mocks::logging_key_input<view>());
				hosting_window f;

				f.host->set_view(v);
				v->request_focus(v);
				v->events.clear();

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_CONTROL, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, 'A', 0);
				::SendMessage(f.hwnd, WM_KEYUP, VK_LEFT, 0);

				// ASSERT
				mocks::keyboard_event reference1[] = {
					{ mocks::keyboard_event::keydown, 'A', keyboard_input::control },
					{ mocks::keyboard_event::keyup, keyboard_input::left, keyboard_input::control },
				};

				assert_equal(reference1, v->events);

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_SHIFT, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, 'B', 0);
				::SendMessage(f.hwnd, WM_KEYUP, 'N', 0);

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
				shared_ptr< mocks::logging_key_input<view> > v(new mocks::logging_key_input<view>());
				hosting_window f;

				f.host->set_view(v);
				v->request_focus(v);
				v->events.clear();

				::SendMessage(f.hwnd, WM_KEYDOWN, VK_CONTROL, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_SHIFT, 0);

				// ACT
				::SendMessage(f.hwnd, WM_KEYUP, VK_CONTROL, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, 'T', 0);
				::SendMessage(f.hwnd, WM_KEYUP, VK_HOME, 0);

				// ASSERT
				mocks::keyboard_event reference1[] = {
					{ mocks::keyboard_event::keydown, 'T', keyboard_input::shift },
					{ mocks::keyboard_event::keyup, keyboard_input::home, keyboard_input::shift },
				};

				assert_equal(reference1, v->events);

				// ACT
				::SendMessage(f.hwnd, WM_KEYUP, VK_SHIFT, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_RETURN, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_END, 0);

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
				shared_ptr<mocks::native_view> v(new mocks::native_view);
				hosting_window f;

				v->response.push_back(make_pair(shared_ptr<mocks::native_view_window>(new mocks::native_view_window), view_location()));
				v->response.push_back(make_pair(shared_ptr<mocks::native_view_window>(new mocks::native_view_window), view_location()));
				f.host->set_view(v);

				// ACT
				::MoveWindow(f.hwnd, 0, 0, 100, 50, TRUE);

				// ASSERT
				assert_is_false(v->container_was_dirty);
				assert_equal(f.hwnd, ::GetParent(v->response[0].first->hwnd()));
				assert_equal(f.hwnd, ::GetParent(v->response[1].first->hwnd()));

				// ACT
				::MoveWindow(f.hwnd, 0, 0, 101, 51, TRUE);

				// ASSERT
				assert_is_false(v->container_was_dirty);
			}


			test( NativeViewWindowsAreResizedAccordinglyToTheirLocations )
			{
				// INIT
				shared_ptr<mocks::native_view> v(new mocks::native_view);
				hosting_window f;
				view_location l1 = { 10, 17, 100, 134 }, l2 = { 100, 1, 91, 200 }, l3 = { 10, 10, 100, 200 };
				shared_ptr<mocks::native_view_window> children[] = {
					shared_ptr<mocks::native_view_window>(new mocks::native_view_window), shared_ptr<mocks::native_view_window>(new mocks::native_view_window),
				};

				v->response.push_back(make_pair(children[0], l1));
				v->response.push_back(make_pair(children[1], l2));
				f.host->set_view(v);

				// ACT
				::MoveWindow(f.hwnd, 0, 0, 100, 50, TRUE);

				// ASSERT
				assert_equal(rect(10, 17, 100, 134), get_window_rect(children[0]->hwnd()));
				assert_equal(rect(100, 1, 91, 200), get_window_rect(children[1]->hwnd()));

				// INIT
				v->response[0].second = l3;
				v->response.resize(1);

				// ACT
				::MoveWindow(f.hwnd, 0, 0, 101, 51, TRUE);

				// ASSERT
				assert_equal(rect(10, 10, 100, 200), get_window_rect(children[0]->hwnd()));
				assert_equal(rect(100, 1, 91, 200), get_window_rect(children[1]->hwnd()));
			}


			test( ResettingViewDetachesFromEventsOfThePreviousView )
			{
				// INIT
				shared_ptr<view> v(new view);
				hosting_window f;
				shared_ptr<void> capture;

				f.host->set_view(v);
				::MoveWindow(f.hwnd, 0, 0, 500, 400, TRUE);
				::ValidateRect(f.hwnd, NULL);

				// ACT
				f.host->set_view(shared_ptr<view>());

				// ASSERT
				assert_is_true(v.unique());

				// ACT
				v->invalidate(0);
				v->capture(capture);

				// ASSERT
				assert_is_false(!!::GetUpdateRect(f.hwnd, NULL, FALSE));
				assert_equal(HWND(), ::GetCapture());
				assert_null(capture);
			}


			test( ForcingLayoutSignalLeadsToViewResize )
			{
				// INIT
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>);
				hosting_window f;
				RECT rc;

				f.host->set_view(v);
				::MoveWindow(f.hwnd, 0, 0, 500, 400, TRUE);
				::GetClientRect(f.hwnd, &rc);
				v->resize_log.clear();

				// ACT
				v->force_layout();

				// ASSERT
				assert_equal(1u, v->resize_log.size());
				assert_equal(rc.right, v->resize_log[0].first);
				assert_equal(rc.bottom, v->resize_log[0].second);
			}


			test( NativeViewWindowsAreResizedAccordinglyToTheirLocationsOnForceLayout )
			{
				// INIT
				shared_ptr<mocks::native_view> v(new mocks::native_view);
				hosting_window f;
				view_location l1 = { 10, 17, 100, 134 }, l2 = { 100, 1, 91, 200 };
				shared_ptr<mocks::native_view_window> children[] = {
					shared_ptr<mocks::native_view_window>(new mocks::native_view_window), shared_ptr<mocks::native_view_window>(new mocks::native_view_window),
				};

				v->response.push_back(make_pair(children[0], l1));
				v->response.push_back(make_pair(children[1], l2));
				f.host->set_view(v);
				::MoveWindow(f.hwnd, 0, 0, 100, 100, TRUE);
				::MoveWindow(children[0]->hwnd(), 0, 0, 1, 1, TRUE);
				::MoveWindow(children[1]->hwnd(), 0, 0, 1, 1, TRUE);

				// ACT
				v->force_layout();

				// ASSERT
				assert_equal(rect(10, 17, 100, 134), get_window_rect(children[0]->hwnd()));
				assert_equal(rect(100, 1, 91, 200), get_window_rect(children[1]->hwnd()));
			}


			test( SettingViewResizesItToAClient )
			{
				// INIT
				hosting_window f;
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>);
				RECT rc;

				::MoveWindow(f.hwnd, 0, 0, 500, 400, TRUE);
				::GetClientRect(f.hwnd, &rc);

				// ACT
				f.host->set_view(v);

				// ASSERT
				assert_equal(1u, v->resize_log.size());
				assert_equal(rc.right, v->resize_log[0].first);
				assert_equal(rc.bottom, v->resize_log[0].second);

				// INIT
				f.host->set_view(shared_ptr<view>());
				::MoveWindow(f.hwnd, 0, 0, 203, 150, TRUE);
				::GetClientRect(f.hwnd, &rc);

				// ACT
				f.host->set_view(v);

				// ASSERT
				assert_equal(2u, v->resize_log.size());
				assert_equal(rc.right, v->resize_log[1].first);
				assert_equal(rc.bottom, v->resize_log[1].second);
			}


			test( TheWholeWindowIsInvalidatedOnSettingBackgroundColor )
			{
				// INIT
				hosting_window f;
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>);

				f.host->set_view(v);
				::ValidateRect(f.hwnd, NULL);

				// ACT
				f.host->set_background_color(color::make(200, 150, 100));

				// ASSERT
				RECT reference1 = { 0, 0, 100, 70 }, invalid;

				assert_is_true(!!::GetUpdateRect(f.hwnd, &invalid, FALSE));
				assert_equal(reference1, invalid);

				// INIT
				::MoveWindow(f.hwnd, 0, 0, 200, 150, FALSE);
				::ValidateRect(f.hwnd, NULL);

				// ACT
				f.host->set_background_color(color::make(100, 100, 100));

				// ASSERT
				RECT reference2 = { 0, 0, 200, 150 };

				assert_is_true(!!::GetUpdateRect(f.hwnd, &invalid, FALSE));
				assert_equal(reference2, invalid);
			}


			test( BackgroundIsFilledWithPresetColor )
			{
				// INIT
				hosting_window f;
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>);

				f.host->set_view(v);
				::ValidateRect(f.hwnd, NULL);

				// ACT
				f.host->set_background_color(color::make(200, 150, 100));
				::RedrawWindow(f.hwnd, NULL, NULL, RDW_UPDATENOW);

				// ASSERT
				pair<gcontext::pixel_type, bool> reference1[] = {
					make_pair(make_pixel(color::make(200, 150, 100, 255)), true),
				};

				assert_equal(reference1, v->background_color);

				// ACT
				f.host->set_background_color(color::make(100, 0, 200));
				::RedrawWindow(f.hwnd, NULL, NULL, RDW_UPDATENOW);

				// ASSERT
				pair<gcontext::pixel_type, bool> reference2[] = {
					make_pair(make_pixel(color::make(200, 150, 100, 255)), true),
					make_pair(make_pixel(color::make(100, 0, 200, 255)), true),
				};

				assert_equal(reference2, v->background_color);
			}


			test( KeyboardInputIsRedirectedToTheFirstControlInOrderOnTab )
			{
				// INIT
				hosting_window f;
				shared_ptr<container> c(new container);
				shared_ptr< mocks::logging_key_input<view> > v1(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v2(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v3(new mocks::logging_key_input<view>);
				shared_ptr<stack> s(new stack(0, true));

				c->set_layout(s);
				c->add_view(v1, 5), s->add(10);
				c->add_view(v2, 2), s->add(10);
				c->add_view(v3, 1000), s->add(10);
				f.host->set_view(c);
				::ValidateRect(f.hwnd, NULL);

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);

				// ASSERT
				mocks::keyboard_event reference1[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
				};

				assert_is_empty(v1->events);
				assert_equal(reference1, v2->events);
				assert_is_empty(v3->events);

				// INIT
				v2->events.clear();

				// ACT
				::SendMessage(f.hwnd, WM_KEYUP, VK_TAB, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, 'Z', 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_SHIFT, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, 'A', 0);
				::SendMessage(f.hwnd, WM_KEYUP, 'Z', 0);

				// ASSERT
				mocks::keyboard_event reference2[] = {
					{ mocks::keyboard_event::keydown, 'Z', 0 },
					{ mocks::keyboard_event::keydown, 'A', keyboard_input::shift },
					{ mocks::keyboard_event::keyup, 'Z', keyboard_input::shift },
				};

				assert_is_empty(v1->events);
				assert_equal(reference2, v2->events);
				assert_is_empty(v3->events);
			}


			test( FocusIsCycledAccordinglyToTabOrder )
			{
				// INIT
				hosting_window f;
				shared_ptr<container> c(new container);
				shared_ptr< mocks::logging_key_input<view> > v1(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v2(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v3(new mocks::logging_key_input<view>);
				shared_ptr<stack> s(new stack(0, true));

				c->set_layout(s);
				c->add_view(v1, 5), s->add(10);
				c->add_view(v2, 2), s->add(10);
				c->add_view(v3, 1000), s->add(10);
				f.host->set_view(c);
				::ValidateRect(f.hwnd, NULL);

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);
				::SendMessage(f.hwnd, WM_KEYUP, VK_TAB, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);

				// ASSERT
				mocks::keyboard_event reference_in[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
				};
				mocks::keyboard_event reference_inout[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
				};

				assert_equal(reference_in, v1->events);
				assert_equal(reference_inout, v2->events);
				assert_is_empty(v3->events);

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);

				// ASSERT
				assert_equal(reference_inout, v1->events);
				assert_equal(reference_inout, v2->events);
				assert_equal(reference_in, v3->events);

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);

				// ASSERT
				mocks::keyboard_event reference_inoutin[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
					{ mocks::keyboard_event::focusin, 0, 0 },
				};

				assert_equal(reference_inout, v1->events);
				assert_equal(reference_inoutin, v2->events);
				assert_equal(reference_inout, v3->events);
			}


			test( FocusIsCycledBackwardsAccordinglyToTabOrder )
			{
				// INIT
				hosting_window f;
				shared_ptr<container> c(new container);
				shared_ptr< mocks::logging_key_input<view> > v1(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v2(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v3(new mocks::logging_key_input<view>);
				shared_ptr<stack> s(new stack(0, true));

				c->set_layout(s);
				c->add_view(v1, 3), s->add(10);
				c->add_view(v2, 2), s->add(10);
				c->add_view(v3, 1), s->add(10);
				f.host->set_view(c);
				::ValidateRect(f.hwnd, NULL);

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_SHIFT, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);

				// ASSERT
				mocks::keyboard_event reference_in[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
				};

				assert_equal(reference_in, v1->events);
				assert_is_empty(v2->events);
				assert_is_empty(v3->events);

				// ACT
				::SendMessage(f.hwnd, WM_KEYUP, VK_TAB, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);

				// ASSERT
				mocks::keyboard_event reference_inout[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
				};

				assert_equal(reference_inout, v1->events);
				assert_equal(reference_inout, v2->events);
				assert_equal(reference_in, v3->events);
			}


			test( FocusIsSetToTheControlSpecifiedByRequest )
			{
				// INIT
				hosting_window f;
				shared_ptr<container> c(new container);
				shared_ptr< mocks::logging_key_input<view> > v1(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v2(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v3(new mocks::logging_key_input<view>);
				shared_ptr<stack> s(new stack(0, true));

				c->set_layout(s);
				c->add_view(v1, 3), s->add(10);
				c->add_view(v2, 2), s->add(10);
				c->add_view(v3, 1), s->add(10);
				f.host->set_view(c);
				::ValidateRect(f.hwnd, NULL);

				// ACT
				c->request_focus(v2);

				// ASSERT
				mocks::keyboard_event reference_in[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
				};

				assert_is_empty(v1->events);
				assert_equal(reference_in, v2->events);
				assert_is_empty(v3->events);

				// ACT
				c->request_focus(v1);

				// ASSERT
				mocks::keyboard_event reference_inout[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
				};

				assert_equal(reference_in, v1->events);
				assert_equal(reference_inout, v2->events);
				assert_is_empty(v3->events);

				// ACT
				c->request_focus(v3);

				// ASSERT
				assert_equal(reference_inout, v1->events);
				assert_equal(reference_inout, v2->events);
				assert_equal(reference_in, v3->events);
			}


			test( FocusMovesAccordinglyToTabOrderAfterFocusRequest )
			{
				// INIT
				hosting_window f;
				shared_ptr<container> c(new container);
				shared_ptr< mocks::logging_key_input<view> > v1(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v2(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v3(new mocks::logging_key_input<view>);
				shared_ptr<stack> s(new stack(0, true));

				c->set_layout(s);
				c->add_view(v1, 3), s->add(10);
				c->add_view(v2, 2), s->add(10);
				c->add_view(v3, 1), s->add(10);
				f.host->set_view(c);
				::ValidateRect(f.hwnd, NULL);

				c->request_focus(v2);

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);

				// ASSERT
				mocks::keyboard_event reference_in[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
				};
				mocks::keyboard_event reference_inout[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
				};

				assert_equal(reference_in, v1->events);
				assert_equal(reference_inout, v2->events);
				assert_is_empty(v3->events);

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);

				// ASSERT
				assert_equal(reference_inout, v1->events);
				assert_equal(reference_inout, v2->events);
				assert_equal(reference_in, v3->events);

				// INIT
				c->request_focus(v3);
				v1->events.clear();
				v2->events.clear();
				v3->events.clear();

				// ACT
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_SHIFT, 0);
				::SendMessage(f.hwnd, WM_KEYDOWN, VK_TAB, 0);

				// ASSERT
				mocks::keyboard_event reference_out[] = {
					{ mocks::keyboard_event::focusout, 0, 0 },
				};

				assert_equal(reference_in, v1->events);
				assert_is_empty(v2->events);
				assert_equal(reference_out, v3->events);
			}


			test( FocusDoesNotMoveIfSetToTheSameControl )
			{
				// INIT
				hosting_window f;
				shared_ptr<container> c(new container);
				shared_ptr< mocks::logging_key_input<view> > v1(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v2(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v3(new mocks::logging_key_input<view>);
				shared_ptr<stack> s(new stack(0, true));

				c->set_layout(s);
				c->add_view(v1, 3), s->add(10);
				c->add_view(v2, 2), s->add(10);
				c->add_view(v3, 1), s->add(10);
				f.host->set_view(c);
				::ValidateRect(f.hwnd, NULL);

				c->request_focus(v2);
				v2->events.clear();

				// ACT
				c->request_focus(v2);

				// ASSERT
				assert_is_empty(v1->events);
				assert_is_empty(v2->events);
				assert_is_empty(v3->events);
			}


			test( FocusDoesNotMoveIfUnregisteredControlRequestsForFocus )
			{
				// INIT
				hosting_window f;
				shared_ptr<container> c(new container);
				shared_ptr< mocks::logging_key_input<view> > v1(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v2(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v3(new mocks::logging_key_input<view>);
				shared_ptr<stack> s(new stack(0, true));

				c->set_layout(s);
				c->add_view(v1, 3), s->add(10);
				c->add_view(v2, 2), s->add(10);
				f.host->set_view(c);
				::ValidateRect(f.hwnd, NULL);

				c->request_focus(v2);
				v2->events.clear();

				// ACT
				c->request_focus(v3);

				// ASSERT
				assert_is_empty(v1->events);
				assert_is_empty(v2->events);
				assert_is_empty(v3->events);
			}


			test( WindowObtainsFocusOnFocusRequest )
			{
				// INIT
				hosting_window f1;
				hosting_window f2;
				shared_ptr<container> c1(new container);
				shared_ptr<container> c2(new container);
				shared_ptr< mocks::logging_key_input<view> > v1(new mocks::logging_key_input<view>);
				shared_ptr< mocks::logging_key_input<view> > v2(new mocks::logging_key_input<view>);
				shared_ptr<stack> s1(new stack(0, true));
				shared_ptr<stack> s2(new stack(0, true));

				c1->set_layout(s1);
				c1->add_view(v1, 3); s1->add(10);
				c2->set_layout(s2);
				c2->add_view(v2, 2); s2->add(10);
				f1.host->set_view(c1);
				f2.host->set_view(c2);
				::ValidateRect(f1.hwnd, NULL);
				::ValidateRect(f2.hwnd, NULL);

				// ACT
				c1->request_focus(v1);

				// ASSERT
				assert_equal(f1.hwnd, GetFocus());

				// ACT
				c2->request_focus(v2);

				// ASSERT
				assert_equal(f2.hwnd, GetFocus());

				// ACT (no change on requesting invalid child)
				c1->request_focus(v2);

				// ASSERT
				assert_equal(f2.hwnd, GetFocus());
			}
		end_test_suite
	}
}
