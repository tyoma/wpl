#include <wpl/ui/view_host.h>
#include <wpl/ui/win32/controls.h>

#include "helpers.h"
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
	namespace ui
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
					shared_ptr<wpl::ui::view_host> host;
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
			end_test_suite
		}
	}
}
