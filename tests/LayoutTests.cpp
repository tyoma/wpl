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


			test( MinimumWidthIncludesPadding )
			{
				// INIT
				shared_ptr<mocks::control> c = make_shared<mocks::control>();
				auto p = pad_control(c, 3, 0);

				c->minimum_height = 1000;
				c->minimum_width = 11;

				// ACT / ASSERT
				assert_equal(17, p->min_width(0));

				// INIT
				c->minimum_width = 19;

				// ACT / ASSERT
				assert_equal(25, p->min_width(0));

				// INIT / ACT
				p = pad_control(c, 9, 0);

				// ACT / ASSERT
				assert_equal(37, p->min_width(0));
			}


			test( MinimumHeightIncludesPadding )
			{
				// INIT
				shared_ptr<mocks::control> c = make_shared<mocks::control>();
				auto p = pad_control(c, 1, 3);

				c->minimum_width = 1000;
				c->minimum_height = 11;

				// ACT / ASSERT
				assert_equal(17, p->min_height(0));

				// INIT
				c->minimum_height = 19;

				// ACT / ASSERT
				assert_equal(25, p->min_height(0));

				// INIT / ACT
				p = pad_control(c, 0, 9);

				// ACT / ASSERT
				assert_equal(37, p->min_height(0));
			}


			test( ReducedHeightIsPassedToInnerControlOnRequestMinWidth )
			{
				// INIT
				shared_ptr<mocks::control> c = make_shared<mocks::control>();
				auto p = pad_control(c, 11, 13);

				// ACT
				p->min_width(100);

				// ASSERT
				int reference1[] = {	74,	};

				assert_equal(reference1, c->for_height_log);

				// ACT
				p->min_width(73);

				// ASSERT
				int reference2[] = {	74, 47,	};

				assert_equal(reference2, c->for_height_log);

				// INIT
				p = pad_control(c, 10, 7);

				// ACT
				p->min_width(90);

				// ASSERT
				int reference3[] = {	74, 47, 76,	};

				assert_equal(reference3, c->for_height_log);
			}


			test( ReducedWidthIsPassedToInnerControlOnRequestMinHeight )
			{
				// INIT
				shared_ptr<mocks::control> c = make_shared<mocks::control>();
				auto p = pad_control(c, 13, 1);

				// ACT
				p->min_height(100);

				// ASSERT
				int reference1[] = {	74,	};

				assert_equal(reference1, c->for_width_log);

				// ACT
				p->min_height(73);

				// ASSERT
				int reference2[] = {	74, 47,	};

				assert_equal(reference2, c->for_width_log);

				// INIT
				p = pad_control(c, 7, 1);

				// ACT
				p->min_height(90);

				// ASSERT
				int reference3[] = {	74, 47, 76,	};

				assert_equal(reference3, c->for_width_log);
			}


			test( MaxForWidthForHeightArePassedAsIs )
			{
				// INIT
				shared_ptr<mocks::control> c = make_shared<mocks::control>();
				auto p = pad_control(c, 13, 1);

				// ACT
				p->min_height(maximum_size);
				p->min_width(maximum_size);

				// ASSERT
				int reference[] = {	maximum_size,	};

				assert_equal(reference, c->for_width_log);
				assert_equal(reference, c->for_height_log);
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


			test( MaximumMinWidthIsReturned )
			{
				// INIT
				const shared_ptr<mocks::control> ctls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				overlay o;

				ctls[0]->minimum_height = 1000;
				ctls[0]->minimum_width = 91;
				o.add(ctls[0]);
				ctls[1]->minimum_width = 29;
				o.add(ctls[1]);

				// ACT / ASSERT
				assert_equal(91, o.min_width(0));

				// INIT
				ctls[2]->minimum_width = 109;
				o.add(ctls[2]);

				// ACT / ASSERT
				assert_equal(109, o.min_width(0));
			}


			test( MaximumMinHeightIsReturned )
			{
				// INIT
				const shared_ptr<mocks::control> ctls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				overlay o;

				ctls[0]->minimum_width = 1000;
				ctls[0]->minimum_height = 91;
				o.add(ctls[0]);
				ctls[1]->minimum_height = 29;
				o.add(ctls[1]);

				// ACT / ASSERT
				assert_equal(91, o.min_height(0));

				// INIT
				ctls[2]->minimum_height = 109;
				o.add(ctls[2]);

				// ACT / ASSERT
				assert_equal(109, o.min_height(0));
			}


			test( ForWidthForHeightPassedAsIsToAllChildren )
			{
				// INIT
				const shared_ptr<mocks::control> ctls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				overlay o;

				o.add(ctls[0]);
				o.add(ctls[1]);
				o.add(ctls[2]);

				// ACT
				o.min_width(123);

				// ASSERT
				int reference1[] = {	123,	};

				assert_equal(reference1, ctls[0]->for_height_log);
				assert_equal(reference1, ctls[1]->for_height_log);
				assert_equal(reference1, ctls[2]->for_height_log);

				// ACT
				o.min_width(1000);
				o.min_height(97);

				// ASSERT
				int reference2[] = {	123, 1000,	};
				int reference3[] = {	97,	};

				assert_equal(reference2, ctls[0]->for_height_log);
				assert_equal(reference2, ctls[1]->for_height_log);
				assert_equal(reference2, ctls[2]->for_height_log);
				assert_equal(reference3, ctls[0]->for_width_log);
				assert_equal(reference3, ctls[1]->for_width_log);
				assert_equal(reference3, ctls[2]->for_width_log);

				// ACT
				o.min_height(19);

				// ASSERT
				int reference4[] = {	97, 19,	};

				assert_equal(reference4, ctls[0]->for_width_log);
				assert_equal(reference4, ctls[1]->for_width_log);
				assert_equal(reference4, ctls[2]->for_width_log);
			}

		end_test_suite
	}
}
