#include <wpl/layout.h>

#include <tests/common/helpers.h>
#include <tests/common/helpers-visual.h>
#include <tests/common/mock-control.h>

#include <ut/assert.h>
#include <ut/test.h>
#include <wpl/view.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			const auto nullptr_nv = shared_ptr<native_view>();
		}

		begin_test_suite( PadLayoutTests )
			test( ForceLayoutIsPropagatedUpstream )
			{
				// INIT
				auto layout_forced = 0;
				auto expect_change = true;
				shared_ptr<mocks::control> ctl = make_shared<mocks::control>();

				// INIT / ACT
				auto p = pad_control(ctl, 0, 0);

				slot_connection conn = p->layout_changed += [&] (bool hierarchy_changed) {
					layout_forced++;
					assert_equal(expect_change, hierarchy_changed);
				};

				// ACT 
				ctl->layout_changed(true);

				// ASSERT
				assert_equal(1, layout_forced);

				// INIT
				expect_change = false;

				// ACT
				ctl->layout_changed(false);

				// ASSERT
				assert_equal(2, layout_forced);
			}


			test( ControlIsArrangedWithReducedSize )
			{
				// INIT
				shared_ptr<mocks::control> c = make_shared<mocks::control>();
				vector<placed_view> v;

				// INIT / ACT
				auto p = pad_control(c, 1, 2);

				// ACT
				p->layout(make_appender(v), make_box(100, 97));

				// ASSERT
				agge::box<int> reference1[] = {	{ 98, 93 }, };

				assert_equal(reference1, c->size_log);

				// INIT / ACT
				p = pad_control(c, 13, 17);

				// ACT
				p->layout(make_appender(v), make_box(1000, 970));

				// ASSERT
				agge::box<int> reference2[] = {	{ 98, 93 }, { 974, 936 },	};

				assert_equal(reference2, c->size_log);
			}


			test( ControlViewsAreOffset )
			{
				// INIT
				shared_ptr<mocks::control> c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(7, 13, 15, 30), 1,	},
					{	make_shared<view>(), nullptr_nv, create_rect(10, 20, 30, 200), 2,	},
				};

				c->views.push_back(pv[0]);
				c->views.push_back(pv[1]);

				// INIT / ACT
				auto p = pad_control(c, 1, 1);

				// ACT
				p->layout(make_appender(v), make_box(100, 100));

				// ASSERT
				placed_view reference1[] = {
					{ pv[0].regular, nullptr_nv, create_rect(8, 14, 16, 31), 1 },
					{ pv[1].regular, nullptr_nv, create_rect(11, 21, 31, 201), 2 },
				};

				assert_equal(reference1, v);

				// INIT / ACT
				p = pad_control(c, 3, 5);
				v.clear();

				// ACT
				p->layout(make_appender(v), make_box(100, 100));

				// ASSERT
				placed_view reference2[] = {
					{ pv[0].regular, nullptr_nv, create_rect(10, 18, 18, 35), 1 },
					{ pv[1].regular, nullptr_nv, create_rect(13, 25, 33, 205), 2 },
				};

				assert_equal(reference2, v);
			}

		end_test_suite


		begin_test_suite( OverlayLayoutTests )
			test( AddingAViewForcesLayoutRecalculate )
			{
				// INIT
				overlay o;
				auto layout_forced = 0;
				slot_connection conn = o.layout_changed += [&] (bool hierarchy_changed) {
					layout_forced++;
					assert_is_true(hierarchy_changed);
				};

				// ACT
				o.add(make_shared<mocks::control>());

				// ASSERT
				assert_equal(1, layout_forced);

				// ACT
				o.add(make_shared<mocks::control>());

				// ASSERT
				assert_equal(2, layout_forced);
			}


			test( ForceLayoutIsPropagatedUpstream )
			{
				// INIT
				overlay o;
				auto layout_forced = 0;
				auto expect_change = true;
				shared_ptr<mocks::control> ctls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				slot_connection conn = o.layout_changed += [&] (bool hierarchy_changed) {
					layout_forced++;
					assert_equal(expect_change, hierarchy_changed);
				};

				o.add(ctls[0]);
				o.add(ctls[1]);
				layout_forced = 0;

				// ACT
				ctls[0]->layout_changed(true);

				// ASSERT
				assert_equal(1, layout_forced);

				// INIT
				expect_change = false;

				// ACT
				ctls[1]->layout_changed(false);

				// ASSERT
				assert_equal(2, layout_forced);
			}


			test( ControlOccupyFullContainer )
			{
				// INIT
				const shared_ptr<mocks::control> ctls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				overlay o;
				vector<placed_view> v;

				// INIT / ACT
				o.add(ctls[0]);

				// ACT
				o.layout(make_appender(v), make_box(100, 97));

				// ASSERT
				agge::box<int> reference1[] = {	{ 100, 97 }, };

				assert_equal(reference1, ctls[0]->size_log);

				// INIT / ACT
				o.add(ctls[1]);

				// ACT
				o.layout(make_appender(v), make_box(1000, 970));

				// ASSERT
				agge::box<int> reference21[] = {	{ 100, 97 }, { 1000, 970 },	};
				agge::box<int> reference22[] = {	{ 1000, 970 },	};

				assert_equal(reference21, ctls[0]->size_log);
				assert_equal(reference22, ctls[1]->size_log);
			}


			test( ControlViewsAreAppendedAsIs )
			{
				// INIT
				const shared_ptr<mocks::control> ctls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(7, 13, 15, 30), 1,	},
					{	make_shared<view>(), nullptr_nv, create_rect(10, 20, 30, 200), 3,	},
					{	make_shared<view>(), nullptr_nv, create_rect(10, 20, 30, 200), 2,	},
				};
				overlay o;
				vector<placed_view> v;

				ctls[0]->views.push_back(pv[0]);
				ctls[0]->views.push_back(pv[1]);
				ctls[1]->views.push_back(pv[2]);

				// INIT / ACT
				o.add(ctls[0]);
				o.add(ctls[1]);

				// ACT
				o.layout(make_appender(v), make_box(100, 100));

				// ASSERT
				placed_view reference[] = {
					{ pv[0].regular, nullptr_nv, create_rect(7, 13, 15, 30), 1 },
					{ pv[1].regular, nullptr_nv, create_rect(10, 20, 30, 200), 3 },
					{ pv[2].regular, nullptr_nv, create_rect(10, 20, 30, 200), 2 },
				};

				assert_equal(reference, v);
			}
		end_test_suite
	}
}
