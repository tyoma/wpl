#include <wpl/ui/form.h>
#include <wpl/ui/win32/native_view.h>

#include "Mockups.h"
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

				class child_window : public native_view
				{
				public:
					child_window()
						: _hwnd(::CreateWindow(_T("static"), NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, NULL, 0))
					{	}

					~child_window()
					{
						if (::IsWindow(_hwnd))
							::DestroyWindow(_hwnd);
					}

					virtual HWND get_window()
					{	return _hwnd;	}

				private:
					HWND _hwnd;
				};

				form_and_handle create_form_with_handle()
				{
					window_tracker wt(L"#32770");
					shared_ptr<form> f(form::create());

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

					// ACT
					f.first->set_visible(true);

					// ASSERT
					assert_is_true(!!(WS_VISIBLE & ::GetWindowLong(f.second, GWL_STYLE)));

					// ACT
					f.first->set_visible(false);

					// ASSERT
					assert_is_false(!!(WS_VISIBLE & ::GetWindowLong(f.second, GWL_STYLE)));
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


				test( MovingFormDoesNotRaiseResizeSignal )
				{
					// INIT
					shared_ptr<mocks::logging_visual<view>> v(new mocks::logging_visual<view>());
					form_and_handle f(create_form_with_handle());

					f.first->set_view(v);

					::MoveWindow(f.second, 0, 0, 117, 213, TRUE);
					v->resize_log.clear();

					// ACT
					::MoveWindow(f.second, 23, 91, 117, 213, TRUE);
					::MoveWindow(f.second, 53, 91, 117, 213, TRUE);
					::MoveWindow(f.second, 53, 32, 117, 213, TRUE);
					::MoveWindow(f.second, 23, 100, 117, 213, TRUE);

					// ASSERT
					assert_is_empty(v->resize_log);
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


				test( RedrawingFormSuppliesBitmapOfTheSizeCorrespondingToRedrawAreaToTheContent )
				{
					// INIT
					shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>());
					form_and_handle f(create_form_with_handle());
					RECT rc = { 0, 0, 100, 60 };

					f.first->set_view(v);
					f.first->set_visible(true);

					::MoveWindow(f.second, 0, 0, 1000, 1000, TRUE);
					::ValidateRect(f.second, NULL);

					// ACT
					::RedrawWindow(f.second, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

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
					::RedrawWindow(f.second, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

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
					form_and_handle f(create_form_with_handle());
					RECT rc = { 11, 17, 100, 60 };

					f.first->set_view(v);
					f.first->set_visible(true);

					::MoveWindow(f.second, 0, 0, 1000, 1000, TRUE);
					::ValidateRect(f.second, NULL);

					// ACT
					::RedrawWindow(f.second, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

					// ASSERT
					assert_equal(1u, v->update_area_log.size());
					assert_equal(make_rect(11, 17, 100, 60), v->update_area_log[0]);

					// INIT
					RECT rc2 = { 101, 107, 150, 260 };

					// ACT
					::RedrawWindow(f.second, &rc2, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

					// ASSERT
					assert_equal(2u, v->update_area_log.size());
					assert_equal(make_rect(101, 107, 150, 260), v->update_area_log[1]);
				}


				test( TheSameRasterizerIsSuppliedToDrawingProcedure )
				{
					// INIT
					shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>());
					form_and_handle f(create_form_with_handle());

					f.first->set_view(v);
					f.first->set_visible(true);

					::MoveWindow(f.second, 0, 0, 100, 50, TRUE);

					// ACT
					::RedrawWindow(f.second, NULL, NULL, RDW_UPDATENOW);
					::RedrawWindow(f.second, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

					// ASSERT
					assert_equal(2u, v->rasterizers_log.size());
					assert_not_null(v->rasterizers_log[0]);
					assert_equal(v->rasterizers_log[0], v->rasterizers_log[1]);
				}


				test( RasterizerIsResetOnDraw )
				{
					// INIT
					shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>());
					form_and_handle f(create_form_with_handle());

					f.first->set_view(v);
					f.first->set_visible(true);

					::MoveWindow(f.second, 0, 0, 100, 50, TRUE);

					// ACT / ASSERT (done inside)
					::RedrawWindow(f.second, NULL, NULL, RDW_UPDATENOW);
					::RedrawWindow(f.second, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				}


				test( InvalidateSignalMarksAreaAsInvalid )
				{
					// INIT
					shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>());
					form_and_handle f(create_form_with_handle());
					RECT rc, invalid;

					f.first->set_view(v);
					f.first->set_visible(true);

					::MoveWindow(f.second, 0, 0, 1000, 1000, TRUE);
					::GetClientRect(f.second, &rc);

					// ACT
					v->invalidate(0);

					// ASSERT
					assert_is_true(!!::GetUpdateRect(f.second, &invalid, FALSE));
					assert_equal(rc, invalid);

					// INIT
					agge::rect_i rc2 = { 10, 13, 131, 251 };
					::ValidateRect(f.second, NULL);

					// ACT
					v->invalidate(&rc2);

					// ASSERT
					RECT rc3 = { 10, 13, 131, 251 };

					assert_is_true(!!::GetUpdateRect(f.second, &invalid, FALSE));
					assert_equal(rc3, invalid);

					// INIT
					agge::rect_i rc4 = { 17, 19, 130, 250 };
					::ValidateRect(f.second, NULL);

					// ACT
					v->invalidate(&rc4);

					// ASSERT
					RECT rc5 = { 17, 19, 130, 250 };

					assert_is_true(!!::GetUpdateRect(f.second, &invalid, FALSE));
					assert_equal(rc5, invalid);
				}


				test( MouseEventsAreDispatchedCorrespondingly )
				{
					// INIT
					shared_ptr< mocks::logging_mouse_input<view> > v(new mocks::logging_mouse_input<view>());
					form_and_handle f(create_form_with_handle());

					f.first->set_view(v);

					::MoveWindow(f.second, 0, 0, 100, 50, TRUE);

					// ACT
					::SendMessage(f.second, WM_LBUTTONDOWN, 0, pack_coordinates(-13, 1002));
					::SendMessage(f.second, WM_LBUTTONUP, 0, pack_coordinates(11222, -200));
					::SendMessage(f.second, WM_LBUTTONDOWN, 0, pack_coordinates(10, 10));
					::SendMessage(f.second, WM_LBUTTONDBLCLK, 0, pack_coordinates(12, 11));
					::SendMessage(f.second, WM_LBUTTONDBLCLK, 0, pack_coordinates(10, 13));

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
					::SendMessage(f.second, WM_RBUTTONDOWN, 0, pack_coordinates(-13, 1002));
					::SendMessage(f.second, WM_RBUTTONUP, 0, pack_coordinates(112, -11));
					::SendMessage(f.second, WM_RBUTTONDOWN, 0, pack_coordinates(1, 1));
					::SendMessage(f.second, WM_RBUTTONDBLCLK, 0, pack_coordinates(12, 11));
					::SendMessage(f.second, WM_RBUTTONDBLCLK, 0, pack_coordinates(10, 13));
					::SendMessage(f.second, WM_RBUTTONUP, 0, pack_coordinates(-3, -7));

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
					::SendMessage(f.second, WM_MOUSEMOVE, 0, pack_coordinates(-11, 12));
					::SendMessage(f.second, WM_MOUSEMOVE, 0, pack_coordinates(13, -14));

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
					form_and_handle f(create_form_with_handle());
					shared_ptr<void> h;

					f.first->set_view(v);

					::MoveWindow(f.second, 0, 0, 150, 100, TRUE);
					f.first->set_visible(true);
					::SetFocus(f.second);

					// ACT
					v->capture(h);

					// ASSERT
					assert_not_null(h);
					assert_equal(f.second, ::GetCapture());
				}


				test( SwitchingCaptureNotifiesView )
				{
					// INIT
					shared_ptr< mocks::logging_mouse_input<view> > v1(new mocks::logging_mouse_input<view>());
					shared_ptr< mocks::logging_mouse_input<view> > v2(new mocks::logging_mouse_input<view>());
					form_and_handle f1(create_form_with_handle());
					form_and_handle f2(create_form_with_handle());
					shared_ptr<void> h1, h2;

					f1.first->set_view(v1);
					f2.first->set_view(v2);

					f1.first->set_visible(true);
					f2.first->set_visible(true);
					::SetFocus(f1.second);

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
					form_and_handle f(create_form_with_handle());
					shared_ptr<void> h;

					f.first->set_view(v);
					f.first->set_visible(true);
					::SetFocus(f.second);
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
					form_and_handle f(create_form_with_handle());
					shared_ptr<void> h;

					f.first->set_view(v);
					f.first->set_visible(true);

					// ACT
					::SendMessage(f.second, WM_MOUSEMOVE, 0, pack_coordinates(0, 0));

					// ASSERT
					mocks::mouse_event events[] = {
						mocks::me_enter(),
						mocks::me_move(0, 0, 0),
					};

					assert_equal(events, v->events_log);

					// ACT
					::SendMessage(f.second, WM_MOUSEMOVE, 0, pack_coordinates(7, 17));

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
					form_and_handle f(create_form_with_handle());
					shared_ptr<void> h;

					f.first->set_view(v);
					f.first->set_visible(true);

					::SendMessage(f.second, WM_MOUSEMOVE, 0, pack_coordinates(0, 0));

					// ACT
					::SendMessage(f.second, WM_MOUSELEAVE, 0, 0);

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
					form_and_handle f(create_form_with_handle());
					shared_ptr<void> h;

					f.first->set_view(v);
					f.first->set_visible(true);

					::SendMessage(f.second, WM_MOUSEMOVE, 0, pack_coordinates(0, 0));
					::SendMessage(f.second, WM_MOUSELEAVE, 0, 0);

					// ACT
					::SendMessage(f.second, WM_MOUSEMOVE, 0, pack_coordinates(1, 2));

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


				struct view_providing_nviews : view
				{
					typedef vector< pair<shared_ptr<child_window>, view_location> > response_t;

					view_providing_nviews()
						: container_was_dirty(false)
					{	}

					void resize(unsigned /*cx*/, unsigned /*cy*/, vector<visual::positioned_native_view> &nviews)
					{
						container_was_dirty = container_was_dirty || !nviews.empty();
						for (response_t::const_iterator i = response.begin(); i != response.end(); ++i)
							nviews.push_back(visual::positioned_native_view(*i->first, i->second));
					}

					response_t response;
					bool container_was_dirty;
				};

				test( NativeViewWindowsAreReparented )
				{
					// INIT
					shared_ptr<view_providing_nviews> v(new view_providing_nviews);
					form_and_handle f(create_form_with_handle());

					v->response.push_back(make_pair(shared_ptr<child_window>(new child_window), view_location()));
					v->response.push_back(make_pair(shared_ptr<child_window>(new child_window), view_location()));
					f.first->set_view(v);

					// ACT
					::MoveWindow(f.second, 0, 0, 100, 50, TRUE);

					// ASSERT
					assert_is_false(v->container_was_dirty);
					assert_equal(f.second, ::GetParent(v->response[0].first->get_window()));
					assert_equal(0u, WS_POPUP & ::GetWindowLong(v->response[0].first->get_window(), GWL_STYLE));
					assert_equal(WS_CHILD, WS_CHILD & ::GetWindowLong(v->response[0].first->get_window(), GWL_STYLE));
					assert_equal(f.second, ::GetParent(v->response[1].first->get_window()));
					assert_equal(0u, WS_POPUP & ::GetWindowLong(v->response[1].first->get_window(), GWL_STYLE));
					assert_equal(WS_CHILD, WS_CHILD & ::GetWindowLong(v->response[1].first->get_window(), GWL_STYLE));

					// ACT
					::MoveWindow(f.second, 0, 0, 101, 51, TRUE);

					// ASSERT
					assert_is_false(v->container_was_dirty);
				}


				test( NativeViewWindowsAreResizedAccordinglyToTheirLocations )
				{
					// INIT
					shared_ptr<view_providing_nviews> v(new view_providing_nviews);
					form_and_handle f(create_form_with_handle());
					view_location l1 = { 10, 17, 100, 134 }, l2 = { 100, 1, 91, 200 }, l3 = { 10, 10, 100, 200 };
					shared_ptr<child_window> children[] = {
						shared_ptr<child_window>(new child_window), shared_ptr<child_window>(new child_window),
					};

					v->response.push_back(make_pair(children[0], l1));
					v->response.push_back(make_pair(children[1], l2));
					f.first->set_view(v);

					// ACT
					::MoveWindow(f.second, 0, 0, 100, 50, TRUE);

					// ASSERT
					assert_equal(rect(10, 17, 100, 134), get_window_rect(children[0]->get_window()));
					assert_equal(rect(100, 1, 91, 200), get_window_rect(children[1]->get_window()));

					// INIT
					v->response[0].second = l3;
					v->response.resize(1);

					// ACT
					::MoveWindow(f.second, 0, 0, 101, 51, TRUE);

					// ASSERT
					assert_equal(rect(10, 10, 100, 200), get_window_rect(children[0]->get_window()));
					assert_equal(rect(100, 1, 91, 200), get_window_rect(children[1]->get_window()));
				}


				test( ResettingViewDetachesFromEventsOfThePreviousView )
				{
					// INIT
					shared_ptr<view> v(new view);
					form_and_handle f(create_form_with_handle());
					shared_ptr<void> capture;

					f.first->set_view(v);
					f.first->set_visible(true);
					::MoveWindow(f.second, 0, 0, 500, 400, TRUE);
					::ValidateRect(f.second, NULL);

					// ACT
					f.first->set_view(shared_ptr<view>());

					// ASSERT
					assert_is_true(v.unique());

					// ACT
					v->invalidate(0);
					v->capture(capture);

					// ASSERT
					assert_is_false(!!::GetUpdateRect(f.second, NULL, FALSE));
					assert_equal(HWND(), ::GetCapture());
					assert_null(capture);
				}


				test( ForcingLayoutSignalLeadsToViewResize )
				{
					// INIT
					shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>);
					form_and_handle f(create_form_with_handle());
					RECT rc;

					f.first->set_view(v);
					::MoveWindow(f.second, 0, 0, 500, 400, TRUE);
					::GetClientRect(f.second, &rc);
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
					shared_ptr<view_providing_nviews> v(new view_providing_nviews);
					form_and_handle f(create_form_with_handle());
					view_location l1 = { 10, 17, 100, 134 }, l2 = { 100, 1, 91, 200 };
					shared_ptr<child_window> children[] = {
						shared_ptr<child_window>(new child_window), shared_ptr<child_window>(new child_window),
					};

					v->response.push_back(make_pair(children[0], l1));
					v->response.push_back(make_pair(children[1], l2));
					f.first->set_view(v);
					::MoveWindow(f.second, 0, 0, 100, 100, TRUE);
					::MoveWindow(children[0]->get_window(), 0, 0, 1, 1, TRUE);
					::MoveWindow(children[1]->get_window(), 0, 0, 1, 1, TRUE);

					// ACT
					v->force_layout();

					// ASSERT
					assert_equal(rect(10, 17, 100, 134), get_window_rect(children[0]->get_window()));
					assert_equal(rect(100, 1, 91, 200), get_window_rect(children[1]->get_window()));
				}


				test( SettingViewResizesItToAClient )
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>);
					RECT rc;

					::MoveWindow(f.second, 0, 0, 500, 400, TRUE);
					::GetClientRect(f.second, &rc);

					// ACT
					f.first->set_view(v);

					// ASSERT
					assert_equal(1u, v->resize_log.size());
					assert_equal(rc.right, v->resize_log[0].first);
					assert_equal(rc.bottom, v->resize_log[0].second);

					// INIT
					f.first->set_view(shared_ptr<view>());
					::MoveWindow(f.second, 0, 0, 203, 150, TRUE);
					::GetClientRect(f.second, &rc);

					// ACT
					f.first->set_view(v);

					// ASSERT
					assert_equal(2u, v->resize_log.size());
					assert_equal(rc.right, v->resize_log[1].first);
					assert_equal(rc.bottom, v->resize_log[1].second);
				}
			end_test_suite
		}
	}
}
