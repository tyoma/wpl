#include <wpl/keyboard_router.h>

#include "mock-native_view.h"

#include <tests/common/mock-router_host.h>
#include <tests/common/Mockups.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			class monitored_key_input : public view
			{
			public:
				function<void (mocks::keyboard_event::event_type type, int key_code, int modifiers)> on_event;

			private:
				virtual void key_down(unsigned code, int modifiers) override
				{	on_event(mocks::keyboard_event::keydown, (int)code, modifiers);	}

				virtual void character(wchar_t symbol, unsigned repeats, int modifiers) override
				{	}

				virtual void key_up(unsigned code, int modifiers) override
				{	on_event(mocks::keyboard_event::keyup, (int)code, modifiers);	}

				virtual void got_focus() override
				{	on_event(mocks::keyboard_event::focusin, 0, 0);	}

				virtual void lost_focus() override
				{	on_event(mocks::keyboard_event::focusout, 0, 0);	}
			};

			void cycle_forward(keyboard_router &router)
			{
				router.key_down(keyboard_input::tab, 0);
				router.key_up(keyboard_input::tab, 0);
			}

			void cycle_backward(keyboard_router& router)
			{
				router.key_down(keyboard_input::tab, keyboard_input::shift);
				router.key_up(keyboard_input::tab, keyboard_input::shift);
			}
		}

		begin_test_suite( KeyboardRouterTests )
			vector<placed_view> views;
			mocks::keyboard_router_host host;


			test( FocusIsSetOnFirstControlIfRouterHasAFocus )
			{
				// INIT
				keyboard_router kr(views, host);
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
				};
				placed_view pv[] = {
					{	v[0], nullptr, {}, 13,	},
				};

				views.assign(begin(pv), end(pv));

				// ACT
				kr.got_focus();
				kr.reload_views();

				// ASSERT
				mocks::keyboard_event reference[] = {
					{	mocks::keyboard_event::focusin, 0, 0	},
				};

				assert_equal(reference, v[0]->events);
			}


			test( FocusIsNotSetOnFirstControlIfRouterHasNoFocus )
			{
				// INIT
				keyboard_router kr(views, host);
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
				};
				placed_view pv[] = {
					{	v[0], nullptr, {}, 13,	},
				};

				views.assign(begin(pv), end(pv));

				// ACT
				kr.reload_views();

				// ASSERT
				assert_is_empty(v[0]->events);
			}


			test( FocusIsNotSetOnFirstControlIfRouterHasLostFocus )
			{
				// INIT
				keyboard_router kr(views, host);
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
				};
				placed_view pv[] = {
					{	v[0], nullptr, {}, 13,	},
				};

				views.assign(begin(pv), end(pv));

				// ACT
				kr.got_focus();
				kr.lost_focus();
				kr.reload_views();

				// ASSERT
				assert_is_empty(v[0]->events);
			}


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
					{	v[0], nullptr, {}, 13,	},
					{	v[1], nullptr, {}, 15,	},
					{	v[2], nullptr, {}, 10,	},
					{	v[3], nullptr, {}, 14,	},
				};

				views.assign(begin(pv), end(pv));
				kr1.got_focus();
				kr2.got_focus();

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
					{	v[0], nullptr, {}, 13,	},
					{	v[1], nullptr, {}, 15,	},
					{	v[2], nullptr, {}, 10,	},
					{	v[3], nullptr, {}, 14,	},
				};

				kr.got_focus();
				views.assign(begin(pv), end(pv));
				kr.reload_views();

				// ACT
				cycle_forward(kr);

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
					{	v[0], nullptr, {}, 13,	},
					{	v[1], nullptr, {}, 15,	},
					{	v[2], nullptr, {}, 10,	},
					{	v[3], nullptr, {}, 14,	},
				};

				kr.got_focus();
				views.assign(begin(pv), end(pv));
				kr.reload_views();

				// ACT
				cycle_backward(kr);

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
					{	v[0], nullptr, {}, 13,	},
					{	v[1], nullptr, {}, 15,	},
					{	v[2], nullptr, {}, 10,	},
				};

				kr.got_focus();
				views.assign(begin(pv), end(pv));
				kr.reload_views();

				// ACT
				kr.set_focus(v[1].get());

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

				// ACT
				kr.set_focus(v[0].get());

				// ASSERT
				assert_equal(reference_in, v[0]->events);
				assert_equal(reference_inout, v[1]->events);
				assert_equal(reference_inout, v[2]->events);

				// ACT / ASSERT
				kr.set_focus(v[3].get());

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
					{	v[0], nullptr, {}, 1,	},
					{	v[1], nullptr, {}, 2,	},
					{	v[2], nullptr, {}, 3,	},
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
				cycle_forward(kr);
				cycle_backward(kr);
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
					{	v[0], nullptr, {}, 0,	},
					{	v[1], nullptr, {}, 2,	},
					{	v[2], nullptr, {}, 0,	},
				};

				views.assign(begin(pv), end(pv));
				kr.reload_views();
				v[1]->events.clear();

				// ACT
				cycle_forward(kr);

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
					{	v, nullptr, {}, 1,	},
				};

				views.assign(begin(pv), end(pv));
				kr.reload_views();

				// ACT
				kr.reload_views();
				v->events.clear();
				cycle_forward(kr);

				// ASSERT
				assert_is_empty(v->events);
			}


			test( NativeViewFocusIsSetOnReload )
			{
				// INIT
				keyboard_router kr(views, host);
				const auto v = make_shared<view>();
				const auto nv = make_shared<native_view>();
				placed_view pv[] = {
					{	nullptr, nv, {}, 2,	},
					{	v, nullptr, {}, 3,	},
				};
				vector<native_view*> nv_log;

				kr.got_focus();
				views.assign(begin(pv), end(pv));
				host.on_set_focus = [&] (native_view *nv) {	nv_log.push_back(nv);	};

				// ACT
				kr.reload_views();

				// ASSERT
				native_view* reference_nv[] = { nv.get(), };

				assert_equal(reference_nv, nv_log);
			}


			test(NativeViewFocusIsNotSetOnReloadForEmptyPlacedView)
			{
				// INIT
				keyboard_router kr(views, host);
				const auto v = make_shared<view>();
				const auto nv = make_shared<native_view>();
				placed_view pv[] = {
					{	nullptr, nullptr, {}, 2,	},
				};

				views.assign(begin(pv), end(pv));
				host.on_set_focus = [] (native_view * /*nv*/) {

					// ASSERT
					assert_is_true(false);
				};

				// ACT
				kr.reload_views();
			}


			test( NothingHappensWhenKeyMessagesAreSentToNativeView )
			{
				// INIT
				keyboard_router kr(views, host);
				const auto nv = make_shared<native_view>();
				placed_view pv[] = {
					{	nullptr, nv, {}, 1,	},
				};

				views.assign(begin(pv), end(pv));
				kr.reload_views();

				// ACT / ASSERT (must not crash)
				kr.key_down(keyboard_input::enter, 0);
				kr.key_up(keyboard_input::left, 0);
				kr.character(L'A', 1, 0);
			}


			test( FocusIsCycledThroughMixedViews )
			{
				// INIT
				vector< tuple<int, mocks::keyboard_event::event_type> > log;
				keyboard_router kr(views, host);
				shared_ptr<monitored_key_input> v[] = {
					make_shared<monitored_key_input>(), make_shared<monitored_key_input>(),
					make_shared<monitored_key_input>(),
				};
				shared_ptr<native_view> nv[] = {
					make_shared<native_view>(), make_shared<native_view>(),
				};
				placed_view pv[] = {
					{	v[0], nullptr, {}, 1,	},
					{	nullptr, nv[0], {}, 2,	},
					{	nullptr, nv[1], {}, 3,	},
					{	v[1], nullptr, {}, 4,	},
					{	v[2], nullptr, {}, 5,	},
				};

				kr.got_focus();
				host.on_set_focus = [&] (native_view *nview) {
					log.push_back(make_tuple(nview ? nview == nv[0].get() ? 2 : 3 : 0, mocks::keyboard_event::focusin));
				};
				v[0]->on_event = [&] (mocks::keyboard_event::event_type e, int, int) {	log.push_back(make_tuple(1, e)); };
				v[1]->on_event = [&] (mocks::keyboard_event::event_type e, int, int) {	log.push_back(make_tuple(4, e)); };
				v[2]->on_event = [&] (mocks::keyboard_event::event_type e, int, int) {	log.push_back(make_tuple(5, e)); };

				views.assign(begin(pv), end(pv));

				// ACT
				kr.reload_views();

				// ASSERT
				assert_equal(plural
					+ make_tuple(1, mocks::keyboard_event::focusin), log);

				// INIT
				log.clear();

				// ACT
				cycle_forward(kr);

				// ASSERT
				assert_equal(plural
					+ make_tuple(2, mocks::keyboard_event::focusin), log);

				// ACT
				kr.lost_focus();

				// ASSERT
				assert_equal(plural
					+ make_tuple(2, mocks::keyboard_event::focusin)
					+ make_tuple(1, mocks::keyboard_event::focusout), log);

				// ACT
				kr.got_native_focus([&] (native_view &v) {	return &v == nv[0].get();	});
				kr.lost_focus(); // does nothing

				// ASSERT
				assert_equal(plural
					+ make_tuple(2, mocks::keyboard_event::focusin)
					+ make_tuple(1, mocks::keyboard_event::focusout), log);

				// ACT
				cycle_forward(kr);
				kr.got_native_focus([&] (native_view &v) {	return &v == nv[1].get();	});

				// ASSERT
				assert_equal(plural
					+ make_tuple(2, mocks::keyboard_event::focusin)
					+ make_tuple(1, mocks::keyboard_event::focusout)
					+ make_tuple(3, mocks::keyboard_event::focusin), log);

				// ACT
				cycle_forward(kr);

				// ASSERT
				assert_equal(plural
					+ make_tuple(2, mocks::keyboard_event::focusin)
					+ make_tuple(1, mocks::keyboard_event::focusout)
					+ make_tuple(3, mocks::keyboard_event::focusin)
					+ make_tuple(0, mocks::keyboard_event::focusin), log);

				// ACT
				kr.got_focus();

				// ASSERT
				assert_equal(plural
					+ make_tuple(2, mocks::keyboard_event::focusin)
					+ make_tuple(1, mocks::keyboard_event::focusout)
					+ make_tuple(3, mocks::keyboard_event::focusin)
					+ make_tuple(0, mocks::keyboard_event::focusin)
					+ make_tuple(4, mocks::keyboard_event::focusin), log);

				// ACT
				cycle_forward(kr);
				cycle_forward(kr);

				// ASSERT
				assert_equal(plural
					+ make_tuple(2, mocks::keyboard_event::focusin)
					+ make_tuple(1, mocks::keyboard_event::focusout)
					+ make_tuple(3, mocks::keyboard_event::focusin)
					+ make_tuple(0, mocks::keyboard_event::focusin)
					+ make_tuple(4, mocks::keyboard_event::focusin)
					+ make_tuple(4, mocks::keyboard_event::focusout)
					+ make_tuple(5, mocks::keyboard_event::focusin)
					+ make_tuple(5, mocks::keyboard_event::focusout)
					+ make_tuple(1, mocks::keyboard_event::focusin), log);
			}
		end_test_suite
	}
}
