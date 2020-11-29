#include <wpl/keyboard_router.h>

#include "mock-router_host.h"
#include "Mockups.h"
#include "MockupsNative.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		begin_test_suite( KeyboardRouterNativeTests )
			vector<placed_view> views;
			mocks::keyboard_router_host host;

			test( NativeViewFocusIsSetOnReload )
			{
				// INIT
				keyboard_router kr(views, host);
				const auto v = make_shared<view>();
				const auto nv = make_shared<mocks::native_view_window>();
				placed_view pv[] = {
					{	nullptr, nv, {}, 2,	},
					{	v, nullptr, {}, 3,	},
				};
				vector<native_view *> nv_log;

				views.assign(begin(pv), end(pv));
				host.on_set_focus = [&] (native_view &nv) {	nv_log.push_back(&nv);	};

				// ACT
				kr.reload_views();

				// ASSERT
				native_view *reference_nv[] = {	nv.get(),	};

				assert_equal(reference_nv, nv_log);
			}


			test( NativeViewFocusIsNotSetOnReloadForEmptyPlacedView )
			{
				// INIT
				keyboard_router kr(views, host);
				const auto v = make_shared<view>();
				const auto nv = make_shared<mocks::native_view_window>();
				placed_view pv[] = {
					{	nullptr, nullptr, {}, 2,	},
				};

				views.assign(begin(pv), end(pv));
				host.on_set_focus = [] (native_view &/*nv*/) {

				// ASSERT
					assert_is_true(false);
				};

				// ACT
				kr.reload_views();
			}


			test( NativeViewIsPassedToHostForFocus )
			{
				// INIT
				keyboard_router kr(views, host);
				shared_ptr< mocks::logging_key_input<view> > v[] = {
					make_shared< mocks::logging_key_input<view> >(),
					make_shared< mocks::logging_key_input<view> >(),
				};
				const auto nv = make_shared<mocks::native_view_window>();
				placed_view pv[] = {
					{	v[0], nullptr, {}, 1,	},
					{	nullptr, nv, {}, 2,	},
					{	v[1], nullptr, {}, 3,	},
				};
				vector<native_view *> nv_log;

				views.assign(begin(pv), end(pv));
				host.on_set_focus = [&] (native_view &nv) {	nv_log.push_back(&nv);	};

				// ACT
				kr.reload_views();

				// ASSERT
				assert_is_empty(nv_log);

				// ACT
				kr.key_down(keyboard_input::tab, 0);

				// ASSERT
				mocks::keyboard_event reference_inout[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
					{ mocks::keyboard_event::focusout, 0, 0 },
				};
				native_view *reference_nv[] = {	nv.get(),	};

				assert_equal(reference_inout, v[0]->events);
				assert_equal(reference_nv, nv_log);
				assert_is_empty(v[1]->events);

				// ACT
				kr.key_down(keyboard_input::tab, 0);

				// ASSERT
				mocks::keyboard_event reference_in[] = {
					{ mocks::keyboard_event::focusin, 0, 0 },
				};

				assert_equal(reference_inout, v[0]->events);
				assert_equal(reference_nv, nv_log);
				assert_equal(reference_in, v[1]->events);
			}
		end_test_suite
	}
}
