#include <wpl/mouse_router.h>

#include <tests/common/Mockups.h>
#include <tests/common/mock-router_host.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			const auto nullptr_nv = shared_ptr<native_view>();
			mouse_router::view_and_target view_and_target(shared_ptr<view> v, mouse_input *t)
			{
				mouse_router::view_and_target vt = {	v, t	};
				return vt;
			}
		}

		inline bool operator ==(const mouse_router::view_and_target &lhs, const mouse_router::view_and_target &rhs)
		{	return lhs.over == rhs.over && lhs.events_target == rhs.events_target;	}

		begin_test_suite( MouseRouterTests )
			vector<placed_view> views;
			mocks::mouse_router_host mrhost;

			test( ViewCanBeLocatedByThePoint )
			{
				// INIT
				agge::point<int> pt;
				auto point = [&pt] (int x, int y) -> agge::point<int> & {
					pt.x = x, pt.y = y;
					return pt;
				};
				mouse_router mr(views, mrhost);
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(), make_shared<view>(),	};
				placed_view pv[] = {
					{ v[0], nullptr_nv, { 10, 10, 20, 15 }	},
					{ v[1], nullptr_nv, { 31, 50, 110, 85 }	},
					{ v[2], nullptr_nv, { 141, 50, 200, 85 }	},
				};

				views.assign(begin(pv), end(pv));

				// ACT / ASSERT
				assert_null(mr.from(point(9, 10)));
				assert_null(mr.from(point(10, 9)));
				assert_null(mr.from(point(19, 9)));
				assert_null(mr.from(point(20, 10)));
				assert_null(mr.from(point(20, 14)));
				assert_null(mr.from(point(19, 15)));
				assert_null(mr.from(point(10, 15)));
				assert_null(mr.from(point(9, 14)));

				assert_equal(view_and_target(v[0], v[0].get()), mr.from(point(10, 10)));
				assert_equal(create_point(0, 0), pt);
				assert_equal(view_and_target(v[0], v[0].get()), mr.from(point(19, 10)));
				assert_equal(create_point(9, 0), pt);
				assert_equal(view_and_target(v[0], v[0].get()), mr.from(point(19, 14)));
				assert_equal(create_point(9, 4), pt);

				assert_equal(view_and_target(v[1], v[1].get()), mr.from(point(32, 50)));
				assert_equal(create_point(1, 0), pt);

				assert_equal(view_and_target(v[2], v[2].get()), mr.from(point(141, 51)));
				assert_equal(create_point(0, 1), pt);
			}


			test( ViewSearchIsMadeBackwards )
			{
				// INIT
				agge::point<int> pt;
				auto point = [&pt] (int x, int y) -> agge::point<int> & {
					pt.x = x, pt.y = y;
					return pt;
				};
				mouse_router mr(views, mrhost);
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv[] = {
					{ v[0], nullptr_nv, { 10, 10, 20, 20 }	},
					{ v[1], nullptr_nv, { 15, 15, 30, 30 }	},
				};

				views.assign(begin(pv), end(pv));

				// ACT / ASSERT
				assert_equal(view_and_target(v[1], v[1].get()), mr.from(point(15, 15)));
				assert_equal(create_point(0, 0), pt);
				assert_equal(view_and_target(v[0], v[0].get()), mr.from(point(14, 15)));
				assert_equal(create_point(4, 5), pt);
				assert_equal(view_and_target(v[0], v[0].get()), mr.from(point(15, 14)));
				assert_equal(create_point(5, 4), pt);
			}


			test( MouseEventsAreRedirectedWithOffset )
			{
				// INIT
				mouse_router mr(views, mrhost);
				shared_ptr< mocks::logging_mouse_input<view> > v[] = {
					make_shared< mocks::logging_mouse_input<view> >(),
					make_shared< mocks::logging_mouse_input<view> >(),
				};
				placed_view pv[] = {
					{ v[0], nullptr_nv, { 13, 17, 44, 40 }	},
					{ v[1], nullptr_nv, { 91, 45, 191, 145 }	},
				};

				views.assign(begin(pv), end(pv));

				// ACT
				mr.mouse_move(mouse_input::left, create_point(14, 19));
				mr.mouse_move(mouse_input::right, create_point(40, 37));
				mr.mouse_move(mouse_input::middle, create_point(100, 50));
				mr.mouse_move(mouse_input::left, create_point(150, 140));

				// ASSERT
				mocks::mouse_event reference1[] = {
					mocks::me_enter(),
					mocks::me_move(mouse_input::left, 1, 2),
					mocks::me_move(mouse_input::right, 27, 20),
					mocks::me_leave(),
				};
				mocks::mouse_event reference2[] = {
					mocks::me_enter(),
					mocks::me_move(mouse_input::middle, 9, 5),
					mocks::me_move(mouse_input::left, 59, 95),
				};

				assert_equal(reference1, v[0]->events_log);
				assert_equal(reference2, v[1]->events_log);

				// INIT
				v[0]->events_log.clear();
				v[1]->events_log.clear();

				// ACT
				mr.mouse_click(&mouse_input::mouse_down, mouse_input::left, mouse_input::middle | mouse_input::right, create_point(14, 19));
				mr.mouse_click(&mouse_input::mouse_down, mouse_input::right, mouse_input::middle, create_point(17, 20));
				mr.mouse_click(&mouse_input::mouse_down, mouse_input::left, mouse_input::middle | mouse_input::right, create_point(101, 51));
				mr.mouse_click(&mouse_input::mouse_down, mouse_input::right, mouse_input::middle, create_point(110, 53));

				// ASSERT
				mocks::mouse_event reference3[] = {
					mocks::me_enter(),
					mocks::me_down(mouse_input::left, mouse_input::middle | mouse_input::right, 1, 2),
					mocks::me_down(mouse_input::right, mouse_input::middle, 4, 3),
					mocks::me_leave(),
				};
				mocks::mouse_event reference4[] = {
					mocks::me_leave(),
					mocks::me_enter(),
					mocks::me_down(mouse_input::left, mouse_input::middle | mouse_input::right, 10, 6),
					mocks::me_down(mouse_input::right, mouse_input::middle, 19, 8),
				};

				assert_equal(reference3, v[0]->events_log);
				assert_equal(reference4, v[1]->events_log);

				// INIT
				v[0]->events_log.clear();
				v[1]->events_log.clear();

				// ACT
				mr.mouse_click(&mouse_input::mouse_up, mouse_input::left, mouse_input::middle | mouse_input::right, create_point(14, 19));
				mr.mouse_click(&mouse_input::mouse_up, mouse_input::right, mouse_input::middle, create_point(17, 20));
				mr.mouse_click(&mouse_input::mouse_up, mouse_input::left, mouse_input::middle | mouse_input::right, create_point(101, 51));
				mr.mouse_click(&mouse_input::mouse_up, mouse_input::right, mouse_input::middle, create_point(110, 53));

				// ASSERT
				mocks::mouse_event reference5[] = {
					mocks::me_enter(),
					mocks::me_up(mouse_input::left, mouse_input::middle | mouse_input::right, 1, 2),
					mocks::me_up(mouse_input::right, mouse_input::middle, 4, 3),
					mocks::me_leave(),
				};
				mocks::mouse_event reference6[] = {
					mocks::me_leave(),
					mocks::me_enter(),
					mocks::me_up(mouse_input::left, mouse_input::middle | mouse_input::right, 10, 6),
					mocks::me_up(mouse_input::right, mouse_input::middle, 19, 8),
				};

				assert_equal(reference5, v[0]->events_log);
				assert_equal(reference6, v[1]->events_log);

				// INIT
				v[0]->events_log.clear();
				v[1]->events_log.clear();

				// ACT
				mr.mouse_click(&mouse_input::mouse_double_click, mouse_input::left, mouse_input::middle | mouse_input::right, create_point(14, 19));
				mr.mouse_click(&mouse_input::mouse_double_click, mouse_input::right, mouse_input::middle, create_point(17, 20));
				mr.mouse_click(&mouse_input::mouse_double_click, mouse_input::left, mouse_input::middle | mouse_input::right, create_point(101, 51));
				mr.mouse_click(&mouse_input::mouse_double_click, mouse_input::right, mouse_input::middle, create_point(110, 53));

				// ASSERT
				mocks::mouse_event reference7[] = {
					mocks::me_enter(),
					mocks::me_double_click(mouse_input::left, mouse_input::middle | mouse_input::right, 1, 2),
					mocks::me_double_click(mouse_input::right, mouse_input::middle, 4, 3),
					mocks::me_leave(),
				};
				mocks::mouse_event reference8[] = {
					mocks::me_leave(),
					mocks::me_enter(),
					mocks::me_double_click(mouse_input::left, mouse_input::middle | mouse_input::right, 10, 6),
					mocks::me_double_click(mouse_input::right, mouse_input::middle, 19, 8),
				};

				assert_equal(reference7, v[0]->events_log);
				assert_equal(reference8, v[1]->events_log);

				// INIT
				v[0]->events_log.clear();
				v[1]->events_log.clear();

				// ACT
				mr.mouse_scroll(mouse_input::middle | mouse_input::right, create_point(14, 19), 1, 2);
				mr.mouse_scroll(mouse_input::middle, create_point(17, 20), 0, 4);
				mr.mouse_scroll(mouse_input::middle | mouse_input::right, create_point(101, 51), 7, 0);
				mr.mouse_scroll(keyboard_input::control, create_point(110, 53), 0, 9);

				// ASSERT
				mocks::mouse_event reference9[] = {
					mocks::me_enter(),
					mocks::me_scroll(mouse_input::middle | mouse_input::right, 1, 2, 1, 2),
					mocks::me_scroll(mouse_input::middle, 4, 3, 0, 4),
					mocks::me_leave(),
				};
				mocks::mouse_event reference10[] = {
					mocks::me_leave(),
					mocks::me_enter(),
					mocks::me_scroll(mouse_input::middle | mouse_input::right, 10, 6, 7, 0),
					mocks::me_scroll(keyboard_input::control, 19, 8, 0, 9),
				};

				assert_equal(reference9, v[0]->events_log);
				assert_equal(reference10, v[1]->events_log);
			}


			test( MouseLeaveIsGeneratedWhenMouseLeavesContainer )
			{
				// INIT
				mouse_router mr(views, mrhost);
				shared_ptr< mocks::logging_mouse_input<view> > v[] = {
					make_shared< mocks::logging_mouse_input<view> >(),
					make_shared< mocks::logging_mouse_input<view> >(),
				};
				placed_view pv[] = {
					{ v[0], nullptr_nv, { 13, 17, 44, 40 }	},
					{ v[1], nullptr_nv, { 91, 45, 191, 145 }	},
				};

				views.assign(begin(pv), end(pv));

				mr.mouse_move(0, create_point(14, 19));

				// ACT
				mr.mouse_leave();

				// ASSERT
				mocks::mouse_event reference1[] = { mocks::me_enter(), mocks::me_move(0, 1, 2), mocks::me_leave(), };

				assert_equal(reference1, v[0]->events_log);
				assert_is_empty(v[1]->events_log);

				// INIT
				v[0]->events_log.clear();
				mr.mouse_move(0, create_point(91, 45));

				// ACT
				mr.mouse_leave();

				// ASSERT
				mocks::mouse_event reference2[] = { mocks::me_enter(), mocks::me_move(0, 0, 0), mocks::me_leave(), };

				assert_is_empty(v[0]->events_log);
				assert_equal(reference2, v[1]->events_log);
			}


			test( MouseCaptureFromAChildIsPropagatedUpstream )
			{
				// INIT
				mouse_router mr(views, mrhost);
				shared_ptr<void> capture_source;
				shared_ptr<void> c;
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv[] = {
					{ v[0], nullptr_nv, { 0, 0, 100, 55 }	},
					{ v[1], nullptr_nv, { 70, 30, 150, 100 }	},
				};
				shared_ptr<void> captured;

				views.assign(begin(pv), end(pv));
				mrhost.on_capture_mouse = [&] {	return move(c);	};

				// INIT / ACT
				mr.reload_views();

				// ACT
				c = capture_source = make_shared<int>();
				v[0]->capture(captured, *v[0]);

				// ASSERT
				assert_not_null(captured);
				assert_not_equal(1, capture_source.use_count());

				// ACT
				captured.reset();

				// ASSERT
				assert_equal(1, capture_source.use_count());

				// ACT
				c = capture_source = make_shared<int>();
				v[1]->capture(captured, *v[1]);

				// ASSERT
				assert_not_null(captured);
				assert_not_equal(1, capture_source.use_count());
			}


			test( MouseEnteredViewReceivesCapturedEventsUponRequest )
			{
				// INIT
				mouse_router mr(views, mrhost);
				shared_ptr<void> capture_source;
				shared_ptr<void> c;
				shared_ptr< mocks::logging_mouse_input<view> > v[] = {
					make_shared< mocks::logging_mouse_input<view> >(),
					make_shared< mocks::logging_mouse_input<view> >(),
					make_shared< mocks::logging_mouse_input<view> >(),
				};
				mocks::logging_mouse_input<view> logger1, logger2;
				placed_view pv[] = {
					{ v[0], nullptr_nv, { 0, 0, 100, 55 }	},
					{ v[1], nullptr_nv, { 70, 30, 150, 100 }	},
					{ v[2], nullptr_nv, { 0, 10, 120, 100 }	},
				};
				shared_ptr<void> capture_handle;

				views.assign(begin(pv), end(pv));
				mrhost.on_capture_mouse = [&] {	return make_shared<int>(123);	};
				mr.reload_views();
				mr.mouse_move(0, create_point(10, 3)); // enter v[0]
				v[0]->events_log.clear();

				// ACT (point of all-intersection)
				v[0]->capture(capture_handle, logger1);
				mr.mouse_click(&mouse_input::mouse_down, mouse_input::left, mouse_input::right, create_point(90, 35));

				// ASSERT
				assert_equal(plural + mocks::me_down(mouse_input::left, mouse_input::right, 90, 35), logger1.events_log);
				assert_is_empty(v[0]->events_log);
				assert_is_empty(v[1]->events_log);
				assert_is_empty(v[2]->events_log);

				// ACT
				mr.mouse_move(mouse_input::left | mouse_input::right, create_point(110, 60));
				mr.mouse_move(mouse_input::right, create_point(10, -60));
				mr.mouse_move(mouse_input::right, create_point(20, 6));

				// ASSERT
				assert_equal(plural
					+ mocks::me_down(mouse_input::left, mouse_input::right, 90, 35)
					+ mocks::me_move(mouse_input::left | mouse_input::right, 110, 60)
					+ mocks::me_move(mouse_input::right, 10, -60)
					+ mocks::me_move(mouse_input::right, 20, 6),
					logger1.events_log);
				assert_is_empty(v[0]->events_log);
				assert_is_empty(v[1]->events_log);
				assert_is_empty(v[2]->events_log);

				// INIT
				logger1.events_log.clear();

				// ACT
				capture_handle.reset(); // TODO: generate mouse_leave here, if last position was outside the view

				// ASSERT
				assert_is_empty(logger1.events_log);
				assert_is_empty(v[0]->events_log);
				assert_is_empty(v[1]->events_log);
				assert_is_empty(v[2]->events_log);

				// INIT / ACT (switch works properly after capture)
				mr.mouse_move(0, create_point(120, 30)); // enter v[1]

				// ASSERT
				assert_equal(plural + mocks::me_leave(), v[0]->events_log);
				assert_equal(plural
					+ mocks::me_enter()
					+ mocks::me_move(0, 50, 0),
					v[1]->events_log);
				assert_is_empty(v[2]->events_log);

				// INIT
				v[0]->events_log.clear();
				v[1]->events_log.clear();
				logger1.events_log.clear();

				// ACT
				v[1]->capture(capture_handle, logger2);

				// ASSERT
				assert_is_empty(logger2.events_log);
				assert_is_empty(v[0]->events_log);
				assert_is_empty(v[1]->events_log);
				assert_is_empty(v[2]->events_log);

				// ACT
				mr.mouse_move(mouse_input::left | mouse_input::right, create_point(100, 15)); // v[2]
				mr.mouse_move(mouse_input::right, create_point(121, 30)); // v[1]

				// ASSERT
				assert_is_empty(logger1.events_log);
				assert_equal(plural
					+ mocks::me_move(mouse_input::left | mouse_input::right, 30, -15)
					+ mocks::me_move(mouse_input::right, 51, 0),
					logger2.events_log);
				assert_is_empty(v[0]->events_log);
				assert_is_empty(v[1]->events_log);
				assert_is_empty(v[2]->events_log);
			}


			test( NotificationsFromRemovedViewsAreIgnored )
			{
				// INIT
				mouse_router mr(views, mrhost);
				shared_ptr<void> capture_source;
				shared_ptr<void> c;
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv1[] = {	{ v[0], nullptr_nv, { 0, 0, 100, 55 }	},	};
				placed_view pv2[] = {	{ v[1], nullptr_nv, { 0, 0, 100, 55 }	},	};
				shared_ptr<void> capture_handle;

				views.assign(begin(pv1), end(pv1));
				mrhost.on_capture_mouse = [&] {	return make_shared<int>(123);	};

				mr.reload_views();

				// ACT
				views.assign(begin(pv2), end(pv2));
				mr.reload_views();
				v[0]->capture(capture_handle, *v[0]);

				// ASSERT
				assert_null(capture_handle);

				// ACT
				v[1]->capture(capture_handle, *v[1]);

				// ASSERT
				assert_not_null(capture_handle);
			}


			test( NativeViewsArePermittedOnReload )
			{
				// INIT
				mouse_router mr(views, mrhost);
				shared_ptr<void> capture_source;
				shared_ptr<void> c;
				auto v = make_shared<view>();
				placed_view pv[] = {
					{ v, nullptr_nv, { 0, 0, 100, 55 }	},
					{ nullptr, nullptr_nv /* we don't actually use native view */, { 0, 0, 100, 55 }	},
				};
				shared_ptr<void> capture_handle;

				views.assign(begin(pv), end(pv));
				mrhost.on_capture_mouse = [&] {	return make_shared<int>(123);	};

				// ACT
				mr.reload_views();
				v->capture(capture_handle, *v);

				// ASSERT
				assert_not_null(capture_handle);
			}


			test( FocusRequestSignalIsRaisedOnMouseDownAndDoubleClickingOnAView )
			{
				// INIT
				vector< shared_ptr<keyboard_input> > log;
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(), make_shared<view>(),	};
				placed_view pv[] = {
					{	v[0], nullptr_nv, { 0, 0, 100, 55 }	},
					{	v[1], nullptr_nv, { 70, 30, 150, 100 }	},
					{	v[2], nullptr_nv, { 30, 60, 80, 80 }	},
				};
				mouse_router mr(views, mrhost);

				views.assign(begin(pv), end(pv));
				mrhost.on_request_focus = [&] (shared_ptr<keyboard_input> v) {	log.push_back(v);	};

				// ACT
				mr.mouse_click(&mouse_input::mouse_down, mouse_input::left, 0, create_point(10, 2));

				// ASSERT
				shared_ptr<keyboard_input> reference1[] = { v[0], };

				assert_equal(reference1, log);

				// ACT (these do not cause requests)
				mr.mouse_click(&mouse_input::mouse_up, mouse_input::left, 0, create_point(10, 2));
				mr.mouse_click(&mouse_input::mouse_double_click, mouse_input::right, 0, create_point(10, 2));
				mr.mouse_click(&mouse_input::mouse_up, mouse_input::left, 0, create_point(10, 2));

				// ASSERT
				assert_equal(reference1, log);

				// ACT
				mr.mouse_click(&mouse_input::mouse_down, mouse_input::right, 0, create_point(31, 62));
				mr.mouse_click(&mouse_input::mouse_up, mouse_input::right, 0, create_point(31, 62));

				// ASSERT
				shared_ptr<keyboard_input> reference2[] = { v[0], v[2], };

				assert_equal(reference2, log);

				// ACT
				mr.mouse_click(&mouse_input::mouse_down, mouse_input::middle, 0, create_point(75, 59));
				mr.mouse_click(&mouse_input::mouse_up, mouse_input::middle, 0, create_point(75, 59));

				// ASSERT
				shared_ptr<keyboard_input> reference3[] = { v[0], v[2], v[1], };

				assert_equal(reference3, log);
			}


			test( MouseLeaveIsGeneratedWhenCaptureIsReleasedOutsideTheCapturingView )
			{
				// INIT
				mocks::logging_mouse_input<view> logger;
				shared_ptr< mocks::logging_mouse_input<view> > v = make_shared<mocks::logging_mouse_input<view>>();
				placed_view pv[] = {
					{	v, nullptr_nv, { 77, 34, 150, 100 }	},
				};
				mouse_router mr(views, mrhost);
				shared_ptr<void> handle;

				views.assign(begin(pv), end(pv));
				mr.reload_views();
				mr.mouse_move(0, create_point(77, 34)); // Inside v
				v->events_log.clear();

				// ACT
				v->capture(handle, logger);
				mr.mouse_move(0, create_point(150, 99)); // Goes outside the view that originated the capture.
				handle.reset();

				// ASSERT
				assert_equal(plural + mocks::me_leave(), v->events_log);
			}


			test( MouseEnterIsGeneratedForTheViewWhereCaptureWasReleased )
			{
				// INIT
				mocks::logging_mouse_input<view> logger;
				shared_ptr< mocks::logging_mouse_input<view> > v[] = {
					make_shared<mocks::logging_mouse_input<view>>(),
					make_shared<mocks::logging_mouse_input<view>>(),
				};
				placed_view pv[] = {
					{	v[0], nullptr_nv, { 77, 34, 150, 100 }	},
					{	v[1], nullptr_nv, { 10, 20, 50, 40 }	},
				};
				mouse_router mr(views, mrhost);
				shared_ptr<void> handle;

				views.assign(begin(pv), end(pv));
				mr.reload_views();
				mr.mouse_move(0, create_point(77, 34)); // Inside v
				v[0]->events_log.clear();

				// ACT
				v[0]->capture(handle, logger);
				mr.mouse_move(0, create_point(17, 30)); // Goes outside the view that originated the capture.
				handle.reset();

				// ASSERT
				assert_equal(plural + mocks::me_leave(), v[0]->events_log);
				assert_equal(plural + mocks::me_enter(), v[1]->events_log);
			}
		end_test_suite
	}
}
