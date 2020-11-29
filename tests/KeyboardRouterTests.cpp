#include <wpl/keyboard_router.h>

#include "mock-router_host.h"
#include "Mockups.h"

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
		}

		begin_test_suite( KeyboardRouterTests )
			vector<placed_view> views;
			mocks::keyboard_router_host host;

			test( KeyboardInputIsRedirectedToTheInputWithLowestTabOrderOnReload )
			{
				// INIT
				keyboard_router kr1(views, host);
				keyboard_router kr2(views, host);
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
				};
				placed_view pv[] = {
					{	v[0], nullptr_nv, {}, 13,	},
					{	v[1], nullptr_nv, {}, 15,	},
					{	v[2], nullptr_nv, {}, 10,	},
					{	v[3], nullptr_nv, {}, 14,	},
				};

				views.assign(begin(pv), end(pv));

				// ACT
				kr1.reload_views();
				kr1.key_down(1, 17);
				kr1.key_down(19, 103);
				kr1.key_up(900, 191);
				kr1.key_up(11, 23);

				// ASSERT
				mocks::keyboard_event reference1[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::keydown, 1, 17 },
					{ mocks::keyboard_event::keydown, 19, 103 },
					{ mocks::keyboard_event::keyup, 900, 191 },
					{ mocks::keyboard_event::keyup, 11, 23 },
				};

				assert_is_empty(v[0]->events);
				assert_is_empty(v[1]->events);
				assert_equal(reference1, v[2]->events);
				assert_is_empty(v[3]->events);

				// INIT
				views[1].tab_order = 3;
				v[2]->events.clear();

				// ACT
				kr2.reload_views();
				kr2.key_down(10, 170);
				kr2.key_up(110, 230);

				// ASSERT
				mocks::keyboard_event reference2[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::keydown, 10, 170 },
					{ mocks::keyboard_event::keyup, 110, 230 },
				};

				assert_is_empty(v[0]->events);
				assert_equal(reference2, v[1]->events);
				assert_is_empty(v[2]->events);
				assert_is_empty(v[3]->events);
			}


			test( FocusIsCycledAccordinglyToTabOrder )
			{
				// INIT
				keyboard_router kr(views, host);
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
				};
				placed_view pv[] = {
					{	v[0], nullptr_nv, {}, 13,	},
					{	v[1], nullptr_nv, {}, 15,	},
					{	v[2], nullptr_nv, {}, 10,	},
					{	v[3], nullptr_nv, {}, 14,	},
				};

				views.assign(begin(pv), end(pv));
				kr.reload_views();

				// ACT
				kr.key_down(keyboard_input::tab, 0);
				kr.key_up(keyboard_input::tab, 0);

				// ASSERT
				mocks::keyboard_event reference_in[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
				};
				mocks::keyboard_event reference_inout[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
				};

				assert_equal(reference_in, v[0]->events);
				assert_is_empty(v[1]->events);
				assert_equal(reference_inout, v[2]->events);
				assert_is_empty(v[3]->events);

				// ACT
				kr.key_down(keyboard_input::tab, 0);

				// ASSERT
				assert_equal(reference_inout, v[0]->events);
				assert_is_empty(v[1]->events);
				assert_equal(reference_inout, v[2]->events);
				assert_equal(reference_in, v[3]->events);

				// ACT
				kr.key_down(keyboard_input::tab, 0);
				kr.key_down(keyboard_input::tab, 0);

				// ASSERT
				mocks::keyboard_event reference_inoutin[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
					{ mocks::keyboard_event::focusin, 0, 0 },
				};

				assert_equal(reference_inout, v[0]->events);
				assert_equal(reference_inout, v[1]->events);
				assert_equal(reference_inoutin, v[2]->events);
				assert_equal(reference_inout, v[3]->events);
			}


			test( FocusIsCycledBackwardsAccordinglyToTabOrder )
			{
				// INIT
				keyboard_router kr(views, host);
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
				};
				placed_view pv[] = {
					{	v[0], nullptr_nv, {}, 13,	},
					{	v[1], nullptr_nv, {}, 15,	},
					{	v[2], nullptr_nv, {}, 10,	},
					{	v[3], nullptr_nv, {}, 14,	},
				};

				views.assign(begin(pv), end(pv));
				kr.reload_views();

				// ACT
				kr.key_down(keyboard_input::tab, keyboard_input::shift);
				kr.key_up(keyboard_input::tab, keyboard_input::shift);

				// ASSERT
				mocks::keyboard_event reference_in[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
				};
				mocks::keyboard_event reference_inout[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
				};

				assert_is_empty(v[0]->events);
				assert_equal(reference_in, v[1]->events);
				assert_equal(reference_inout, v[2]->events);
				assert_is_empty(v[3]->events);

				// ACT
				kr.key_down(keyboard_input::tab, keyboard_input::shift);

				// ASSERT
				assert_is_empty(v[0]->events);
				assert_equal(reference_inout, v[1]->events);
				assert_equal(reference_inout, v[2]->events);
				assert_equal(reference_in, v[3]->events);

				// ACT
				kr.key_down(keyboard_input::tab, keyboard_input::shift);
				kr.key_down(keyboard_input::tab, keyboard_input::shift);

				// ASSERT
				mocks::keyboard_event reference_inoutin[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
					{ mocks::keyboard_event::focusin, 0, 0 },
				};

				assert_equal(reference_inout, v[0]->events);
				assert_equal(reference_inout, v[1]->events);
				assert_equal(reference_inoutin, v[2]->events);
				assert_equal(reference_inout, v[3]->events);
			}


			test( FocusIsSetToTheControlSpecifiedByRequest )
			{
				// INIT
				keyboard_router kr(views, host);
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
				};
				placed_view pv[] = {
					{	v[0], nullptr_nv, {}, 13,	},
					{	v[1], nullptr_nv, {}, 15,	},
					{	v[2], nullptr_nv, {}, 10,	},
				};

				views.assign(begin(pv), end(pv));
				kr.reload_views();

				// ACT / ASSERT
				assert_is_true(kr.set_focus(v[1].get()));

				// ASSERT
				mocks::keyboard_event reference_inout[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
				};
				mocks::keyboard_event reference_in[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
				};

				assert_is_empty(v[0]->events);
				assert_equal(reference_in, v[1]->events);
				assert_equal(reference_inout, v[2]->events);

				// ACT / ASSERT
				assert_is_true(kr.set_focus(v[0].get()));

				// ASSERT
				assert_equal(reference_in, v[0]->events);
				assert_equal(reference_inout, v[1]->events);
				assert_equal(reference_inout, v[2]->events);

				// ACT / ASSERT
				assert_is_false(kr.set_focus(v[3].get()));

				// ASSERT
				assert_equal(reference_in, v[0]->events);
				assert_equal(reference_inout, v[1]->events);
				assert_equal(reference_inout, v[2]->events);
			}


			test( FocusDoesNotMoveIfSetToTheSameControl )
			{
				// INIT
				keyboard_router kr(views, host);
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
				};
				placed_view pv[] = {
					{	v[0], nullptr_nv, {}, 1,	},
					{	v[1], nullptr_nv, {}, 2,	},
					{	v[2], nullptr_nv, {}, 3,	},
				};

				views.assign(begin(pv), end(pv));
				kr.reload_views();
				kr.set_focus(v[2].get());
				v[0]->events.clear();
				v[2]->events.clear();

				// ACT
				kr.set_focus(v[2].get());

				// ASSERT
				assert_is_empty(v[0]->events);
				assert_is_empty(v[1]->events);
				assert_is_empty(v[2]->events);
			}


			test( NoFocusIsSetOrCycledForNoViews )
			{
				// INIT
				keyboard_router kr(views, host);

				// ACT / ASSERT (no crash)
				kr.reload_views();
				kr.key_down(keyboard_input::tab, 0);
				kr.key_up(keyboard_input::tab, 0);
				kr.key_down(keyboard_input::tab, keyboard_input::shift);
				kr.key_up(keyboard_input::tab, keyboard_input::shift);
				kr.key_down('a', 0);
				kr.key_up('a', 0);
			}


			test( NonTabStoppableControlsAreNotCycled )
			{
				// INIT
				keyboard_router kr(views, host);
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
				};
				placed_view pv[] = {
					{	v[0], nullptr_nv, {}, 0,	},
					{	v[1], nullptr_nv, {}, 2,	},
					{	v[2], nullptr_nv, {}, 0,	},
				};

				views.assign(begin(pv), end(pv));
				kr.reload_views();
				v[1]->events.clear();

				// ACT
				kr.key_down(keyboard_input::tab, 0);

				// ASSERT
				assert_is_empty(v[0]->events);
				assert_is_empty(v[1]->events);
				assert_is_empty(v[2]->events);
			}


			test( ContentsIsEmptiedOnReload )
			{
				// INIT
				keyboard_router kr(views, host);
				const auto v = make_shared< mocks::logging_key_input<view> >();
				placed_view pv[] = {
					{	v, nullptr_nv, {}, 1,	},
				};

				views.assign(begin(pv), end(pv));
				kr.reload_views();

				// ACT
				kr.reload_views();
				v->events.clear();
				kr.key_down(keyboard_input::tab, 0);

				// ASSERT
				assert_is_empty(v->events);
			}
		end_test_suite
	}
}
