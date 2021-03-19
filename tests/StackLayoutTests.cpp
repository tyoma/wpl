#include <wpl/layout.h>

#include <tests/common/helpers.h>
#include <tests/common/helpers-visual.h>
#include <tests/common/mock-control.h>
#include <tests/common/Mockups.h>

#include <ut/assert.h>
#include <ut/test.h>
#include <wpl/view.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		using agge::zero;
		namespace
		{
			const auto nullptr_nv = shared_ptr<native_view>();

			struct eq
			{
				bool operator ()(const placed_view &lhs, const placed_view &rhs) const
				{	return lhs.location == rhs.location && lhs.tab_order == rhs.tab_order;	}
			};
		}

		begin_test_suite( StackLayoutTests )

			shared_ptr<mocks::cursor_manager> cursor_manager_;

			init( Init )
			{
				cursor_manager_.reset(new mocks::cursor_manager);

				cursor_manager_->cursors[cursor_manager::arrow].reset(new cursor(10, 10, 1, 1));
				cursor_manager_->cursors[cursor_manager::h_resize].reset(new cursor(10, 10, 1, 1));
				cursor_manager_->cursors[cursor_manager::v_resize].reset(new cursor(10, 10, 1, 1));
			}


			test( AddingAViewForcesLayoutRecalculate )
			{
				// INIT
				stack s(false, cursor_manager_);
				auto layout_forced = 0;
				slot_connection conn = s.layout_changed += [&] (bool hierarchy_changed) {
					layout_forced++;
					assert_is_true(hierarchy_changed);
				};

				// ACT
				s.add(make_shared<mocks::control>(), pixels(10));

				// ASSERT
				assert_equal(1, layout_forced);

				// ACT
				s.add(make_shared<mocks::control>(), pixels(50));

				// ASSERT
				assert_equal(2, layout_forced);
			}


			test( ChangingSpacingForcesLayoutRecalculate )
			{
				// INIT
				stack s(false, cursor_manager_);
				auto layout_forced = 0;
				slot_connection conn = s.layout_changed += [&] (bool hierarchy_changed) {
					layout_forced++;
					assert_is_false(hierarchy_changed);
				};

				// ACT
				s.set_spacing(6);

				// ASSERT
				assert_equal(1, layout_forced);

				// ACT
				s.set_spacing(16);

				// ASSERT
				assert_equal(2, layout_forced);
			}


			test( ForceLayoutIsPropagatedUpstream )
			{
				// INIT
				stack s(false, cursor_manager_);
				auto layout_forced = 0;
				auto expect_change = true;
				shared_ptr<mocks::control> ctls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				slot_connection conn = s.layout_changed += [&] (bool hierarchy_changed) {
					layout_forced++;
					assert_equal(expect_change, hierarchy_changed);
				};

				s.add(ctls[0], pixels(10));
				s.add(ctls[1], pixels(10));
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


			test( SingleItemOccupiesAbsoluteSizeAssigned )
			{
				// INIT
				auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv = {	make_shared<view>(), nullptr_nv, create_rect(1, 2, 3, 4), 1,	};

				c->views.push_back(pv);

				// INIT / ACT
				stack sh(true, cursor_manager_);
				stack sv(false, cursor_manager_);

				sh.add(c, pixels(17), false, 190);
				sv.add(c, pixels(91), false, 11);

				// ACT
				sh.layout(make_appender(v), make_box(100, 100));
				sv.layout(make_appender(v), make_box(100, 100));

				// ASSERT
				placed_view reference1_views[] = {
					{ pv.regular, nullptr_nv, create_rect(1, 2, 3, 4), 190 },
					{ pv.regular, nullptr_nv, create_rect(1, 2, 3, 4), 11 },
				};
				agge::box<int> reference1_box[] = {
					{ 17, 100 },
					{ 100, 91 },
				};

				assert_equal(reference1_views, v);
				assert_equal(reference1_box, c->size_log);

				// INIT / ACT
				stack sh2(true, cursor_manager_);
				stack sv2(false, cursor_manager_);

				sh2.set_spacing(100);
				sh2.add(c, pixels(170), false, 1);
				sv2.set_spacing(10);
				sv2.add(c, pixels(190), false, 2);

				// INIT
				v.clear();
				c->size_log.clear();

				// ACT
				sh2.layout(make_appender(v), make_box(1000, 1000));
				sv2.layout(make_appender(v), make_box(1000, 1000));

				// ASSERT
				placed_view reference2_views[] = {
					{ pv.regular, nullptr_nv, create_rect(1, 2, 3, 4), 1 },
					{ pv.regular, nullptr_nv, create_rect(1, 2, 3, 4), 2 },
				};
				agge::box<int> reference2_box[] = {
					{ 170, 1000 },
					{ 1000, 190 },
				};

				assert_equal(reference2_views, v);
				assert_equal(reference2_box, c->size_log);
			}


			test( SingleChildOccupiesRelativeSpaceAssigned )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {	{	make_shared<view>(), nullptr_nv, create_rect(1, 2, 3, 4), 1,	},	};

				c->views = mkvector(pv);

				// INIT / ACT
				stack sh(true, cursor_manager_), sv(false, cursor_manager_);

				sh.add(c, percents(100.0), true, 1);
				sv.add(c, percents(100.0), true, 11);

				// ACT
				sh.layout(make_appender(v), make_box(110, 120));

				// ASSERT
				placed_view reference1_views[] = {
					{ pv[0].regular, nullptr_nv, create_rect(1, 2, 3, 4), 1 },
				};
				agge::box<int> reference1_box[] = {
					{ 110, 120 },
				};

				assert_equal(reference1_views, v);
				assert_equal(reference1_box, c->size_log);

				// INIT
				c->size_log.clear();
				v.clear();

				// ACT
				sv.layout(make_appender(v), make_box(200, 231));

				// ASSERT
				placed_view reference2_views[] = {
					{ pv[0].regular, nullptr_nv, create_rect(1, 2, 3, 4), 11 },
				};
				agge::box<int> reference2_box[] = {
					{ 200, 231 },
				};

				assert_equal(reference2_views, v);
				assert_equal(reference2_box, c->size_log);
			}


			test( LayoutSeveralChildrenAbsolute )
			{
				// INIT
				shared_ptr<mocks::control> controls[] = {
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
				};
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 30, 30), 1,	},
					{	make_shared<view>(), nullptr_nv, create_rect(1, 2, 10, 15), 1,	},
					{	make_shared<view>(), nullptr_nv, create_rect(3, 5, 30, 20), 1,	},
				};
				stack sh(true, cursor_manager_);

				controls[0]->views.push_back(pv[0]);
				controls[1]->views.push_back(pv[1]);
				controls[2]->views.push_back(pv[2]);

				sh.add(controls[0], pixels(17), false, 1);
				sh.add(controls[1], pixels(23), false, 2);

				// ACT
				sh.layout(make_appender(v), make_box(100, 100));

				// ASSERT
				placed_view reference1_views[] = {
					{ pv[0].regular, nullptr_nv, create_rect(0, 0, 30, 30), 1 },
					{ pv[1].regular, nullptr_nv, create_rect(1 + 17, 2, 10 + 17, 15), 2 },
				};
				agge::box<int> reference10_box[] = {	{ 17, 100 },	};
				agge::box<int> reference11_box[] = {	{ 23, 100 },	};

				assert_equal(reference1_views, v);
				assert_equal(reference10_box, controls[0]->size_log);
				assert_equal(reference11_box, controls[1]->size_log);

				// INIT
				stack sv(false, cursor_manager_);

				sv.add(controls[0], pixels(90), false, 10);
				sv.add(controls[1], pixels(80), false, 11);
				sv.add(controls[2], pixels(55), false, 12);

				v.clear();
				controls[0]->size_log.clear();
				controls[1]->size_log.clear();

				// ACT
				sv.layout(make_appender(v), make_box(10, 250));

				placed_view reference2_views[] = {
					{ pv[0].regular, nullptr_nv, create_rect(0, 0, 30, 30), 10 },
					{ pv[1].regular, nullptr_nv, create_rect(1, 2 + 90, 10, 15 + 90), 11 },
					{ pv[2].regular, nullptr_nv, create_rect(3, 5 + 170, 30, 20 + 170), 12 },
				};
				agge::box<int> reference20_box[] = {	{ 10, 90 },	};
				agge::box<int> reference21_box[] = {	{ 10, 80 },	};
				agge::box<int> reference22_box[] = {	{ 10, 55 },	};

				assert_equal(reference2_views, v);
				assert_equal(reference20_box, controls[0]->size_log);
				assert_equal(reference21_box, controls[1]->size_log);
				assert_equal(reference22_box, controls[2]->size_log);
			}


			test( SeveralChildrenOccupyStackAccordinglyToFractionsH )
			{
				// INIT
				shared_ptr<mocks::control> c[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 10, 10), 0,	},
					{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 19, 180), 171,	},
					{	make_shared<view>(), nullptr_nv, create_rect(1, 0, 30, 20), 191,	},
				};

				c[0]->views.push_back(pv[0]);
				c[1]->views.push_back(pv[1]);
				c[2]->views.push_back(pv[2]);

				// INIT / ACT
				stack s(true, cursor_manager_);

				s.add(c[0], percents(17), true, 1);
				s.add(c[1], percents(13), true);

				// ACT
				s.layout(make_appender(v), make_box(110, 120));

				// ASSERT
				placed_view reference1_views[] = {
					{ nullptr, nullptr_nv, create_rect(0, 0, 10, 10), 0 },
					{ nullptr, nullptr_nv, create_rect(19, 0, 19, 120), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(19, 0, 38, 180), 171 },
				};
				agge::box<int> reference11_box[] = {	{ 19, 120 },	};
				agge::box<int> reference12_box[] = {	{ 14, 120 },	};

				assert_equal_pred(reference1_views, v, eq());
				assert_equal(pv[0].regular, v[0].regular);
				assert_equal(pv[1].regular, v[2].regular);
				assert_equal(reference11_box, c[0]->size_log);
				assert_equal(reference12_box, c[1]->size_log);

				// INIT
				c[0]->size_log.clear();
				c[1]->size_log.clear();
				v.clear();

				s.add(c[2], percents(70), true);

				// ACT
				s.layout(make_appender(v), make_box(200, 231));

				// ASSERT
				placed_view reference2_views[] = {
					{ nullptr, nullptr_nv, create_rect(0, 0, 10, 10), 0 },
					{ nullptr, nullptr_nv, create_rect(34, 0, 34, 231), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(34, 0, 53, 180), 171 },
					{ nullptr, nullptr_nv, create_rect(60, 0, 60, 231), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(61, 0, 90, 20), 191 },
				};
				agge::box<int> reference21_box[] = {	{ 34, 231 },	};
				agge::box<int> reference22_box[] = {	{ 26, 231 },	};
				agge::box<int> reference23_box[] = {	{ 140, 231 },	};

				assert_equal_pred(reference2_views, v, eq());
				assert_equal(reference21_box, c[0]->size_log);
				assert_equal(reference22_box, c[1]->size_log);
				assert_equal(reference23_box, c[2]->size_log);
			}


			test( SeveralChildrenOccupyStackAccordinglyToFractionsV )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {	{	nullptr, nullptr_nv, create_rect(0, 0, 10, 10), 0,	},	};

				c->views = mkvector(pv);

				// INIT / ACT
				stack s(false, cursor_manager_);

				s.add(c, percents(19.090909), true);
				s.add(c, percents(26.363636), true);
				s.add(c, percents(54.545454), true);

				// ACT
				s.layout(make_appender(v), make_box(107, 150));

				// ASSERT
				placed_view reference_views[] = {
					{ nullptr, nullptr_nv, create_rect(0, 0, 10, 10), 0 },
					{ nullptr, nullptr_nv, create_rect(0, 29, 107, 29), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(0, 29, 10, 39), 0 },
					{ nullptr, nullptr_nv, create_rect(0, 68, 107, 68), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(0, 68, 10, 78), 0 },
				};
				agge::box<int> reference_box[] = {	{ 107, 29 }, { 107, 39 }, { 107, 82 },	};

				assert_equal_pred(reference_views, v, eq());
				assert_equal(reference_box, c->size_log);
			}


			test( LayoutSeveralChildrenAbsoluteSpaced )
			{
				// INIT
				shared_ptr<mocks::control> controls[] = {
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
				};
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 30, 30), 3,	},
					{	make_shared<view>(), nullptr_nv, create_rect(1, 2, 10, 15), 10,	},
					{	make_shared<view>(), nullptr_nv, create_rect(3, 5, 30, 20), 191,	},
				};
				stack sh(true, cursor_manager_);

				controls[0]->views.push_back(pv[0]);
				controls[1]->views.push_back(pv[1]);
				controls[2]->views.push_back(pv[2]);

				sh.set_spacing(3);
				sh.add(controls[0], pixels(17));
				sh.add(controls[1], pixels(23));
				sh.add(controls[2], pixels(13));

				// ACT
				sh.layout(make_appender(v), make_box(100, 77));

				// ASSERT
				placed_view reference1_views[] = {
					{ pv[0].regular, nullptr_nv, create_rect(0, 0, 30, 30), 3 },
					{ pv[1].regular, nullptr_nv, create_rect(1 + 17 + 3, 2, 10 + 17 + 3, 15), 10 },
					{ pv[2].regular, nullptr_nv, create_rect(3 + 17 + 3 + 23 + 3, 5, 30 + 17 + 3 + 23 + 3, 20), 191 },
				};
				agge::box<int> reference10_box[] = {	{ 17, 77 },	};
				agge::box<int> reference11_box[] = {	{ 23, 77 },	};
				agge::box<int> reference12_box[] = {	{ 13, 77 },	};

				assert_equal(reference1_views, v);
				assert_equal(reference10_box, controls[0]->size_log);
				assert_equal(reference11_box, controls[1]->size_log);
				assert_equal(reference12_box, controls[2]->size_log);
			}


			test( LayoutVSeveralChildrenRelativelyAndAbsolutelySpacedWithInnerSpacing )
			{
				// INIT
				shared_ptr<mocks::control> controls[] = {
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
				};
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 30, 30), 1,	},
					{	make_shared<view>(), nullptr_nv, create_rect(1, 2, 10, 15), 2,	},
					{	make_shared<view>(), nullptr_nv, create_rect(3, 5, 30, 20), 3,	},
					{	make_shared<view>(), nullptr_nv, create_rect(3, 5, 30, 20), 4,	},
					{	make_shared<view>(), nullptr_nv, create_rect(3, 5, 30, 20), 4,	},
				};
				stack sh(true, cursor_manager_);

				controls[0]->views.push_back(pv[0]);
				controls[1]->views.push_back(pv[1]);
				controls[2]->views.push_back(pv[2]);
				controls[3]->views.push_back(pv[3]);
				controls[4]->views.push_back(pv[4]);

				sh.set_spacing(3);
				sh.add(controls[0], pixels(17));
				sh.add(controls[1], percents(50 /*3*/));
				sh.add(controls[2], pixels(13));
				sh.add(controls[3], percents(16.666666 /*1*/));

				// ACT
				sh.layout(make_appender(v), make_box(59, 20));

				// ASSERT
				placed_view reference1_views[] = {
					{ pv[0].regular, nullptr_nv, create_rect(0, 0, 30, 30), 1 },
					{ pv[1].regular, nullptr_nv, create_rect(1 + 17 + 3, 2, 10 + 17 + 3, 15), 2 },
					{ pv[2].regular, nullptr_nv, create_rect(3 + 17 + 3 + 10 + 3, 5, 30 + 17 + 3 + 10 + 3, 20), 3 },
					{ pv[3].regular, nullptr_nv, create_rect(3 + 17 + 3 + 10 + 3 + 13 + 3, 5, 30 + 17 + 3 + 10 + 3 + 13 + 3, 20), 4 },
				};
				agge::box<int> reference10_box[] = {	{ 17, 20 },	};
				agge::box<int> reference11_box[] = {	{ 10, 20 },	};
				agge::box<int> reference12_box[] = {	{ 13, 20 },	};
				agge::box<int> reference13_box[] = {	{ 3, 20 },	};

				assert_equal(reference1_views, v);
				assert_equal(reference10_box, controls[0]->size_log);
				assert_equal(reference11_box, controls[1]->size_log);
				assert_equal(reference12_box, controls[2]->size_log);
				assert_equal(reference13_box, controls[3]->size_log);

				// INIT
				v.clear();

				// ACT
				sh.layout(make_appender(v), make_box(63, 20));

				// ASSERT
				agge::box<int> reference20_box[] = {	{ 17, 20 }, { 17, 20 },	};
				agge::box<int> reference21_box[] = {	{ 10, 20 }, { 12, 20 },	};
				agge::box<int> reference22_box[] = {	{ 13, 20 }, { 13, 20 },	};
				agge::box<int> reference23_box[] = {	{ 3, 20 }, { 4, 20 },	};

				assert_equal(reference20_box, controls[0]->size_log);
				assert_equal(reference21_box, controls[1]->size_log);
				assert_equal(reference22_box, controls[2]->size_log);
				assert_equal(reference23_box, controls[3]->size_log);

				// INIT
				sh.add(controls[4], percents(33.333333 /*2*/));

				// ACT
				sh.layout(make_appender(v), make_box(79, 20));

				// ASSERT
				agge::box<int> reference31_box[] = {	{ 10, 20 }, { 12, 20 }, { 19, 20 },	};
				agge::box<int> reference33_box[] = {	{ 3, 20 }, { 4, 20 }, { 6, 20 },	};
				agge::box<int> reference34_box[] = {	{ 12, 20 },	};

				assert_equal(reference31_box, controls[1]->size_log);
				assert_equal(reference33_box, controls[3]->size_log);
				assert_equal(reference34_box, controls[4]->size_log);
			}


			test( AllControlViewsLocationsAreConverted )
			{
				// INIT
				shared_ptr<mocks::control> controls[] = {
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
				};
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 10, 10), 1,	},
					{	make_shared<view>(), nullptr_nv, create_rect(10, 10, 30, 20), 2,	},
					{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 30, 30), 3,	},
					{	make_shared<view>(), nullptr_nv, create_rect(3, 5, 30, 20), 4,	},
					{	make_shared<view>(), nullptr_nv, create_rect(7, 10, 30, 20), 4,	},
				};
				stack sh(true, cursor_manager_);

				controls[0]->views.push_back(pv[0]);
				controls[0]->views.push_back(pv[1]);
				controls[1]->views.push_back(pv[2]);
				controls[1]->views.push_back(pv[3]);
				controls[1]->views.push_back(pv[4]);

				sh.set_spacing(5);
				sh.add(controls[0], pixels(30), false, 100);
				sh.add(controls[1], pixels(50));

				// ACT
				sh.layout(make_appender(v), make_box(150, 20));

				// ASSERT
				placed_view reference[] = {
					{ pv[0].regular, nullptr_nv, create_rect(0, 0, 10, 10), 100 },
					{ pv[1].regular, nullptr_nv, create_rect(10, 10, 30, 20), 100 },
					{ pv[2].regular, nullptr_nv, create_rect(35, 0, 65, 30), 3 },
					{ pv[3].regular, nullptr_nv, create_rect(38, 5, 65, 20), 4 },
					{ pv[4].regular, nullptr_nv, create_rect(42, 10, 65, 20), 4 },
				};

				assert_equal(reference, v);
			}


			test( OnlyTabStoppableViewsObtainTabOrder )
			{
				// INIT
				const auto control = make_shared<mocks::control>();
				vector<placed_view> layout;
				placed_view pv[] = {
					{	nullptr, nullptr_nv, {}, 0,	},
					{	nullptr, nullptr_nv, {}, 2,	},
				};
				stack sv(false, cursor_manager_);

				control->views.assign(begin(pv), end(pv));

				sv.add(control, pixels(1), false, 100);
				sv.add(control, pixels(1));
				sv.add(control, pixels(1), false, 13);

				// ACT
				sv.layout(make_appender(layout), make_box(150, 20));

				// ASSERT
				placed_view reference[] = {
					{	nullptr, nullptr_nv, {	0, 0, 0, 0	}, 0	},
					{	nullptr, nullptr_nv, {	0, 0, 0, 0	}, 100	},
					{	nullptr, nullptr_nv, {	0, 1, 0, 1	}, 0	},
					{	nullptr, nullptr_nv, {	0, 1, 0, 1	}, 2	},
					{	nullptr, nullptr_nv, {	0, 2, 0, 2	}, 0	},
					{	nullptr, nullptr_nv, {	0, 2, 0, 2	}, 13	},
				};

				assert_equal(reference, layout);
			}


			test( SplitterViewsAreAddedToLayoutH )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 10, 10), 0,	},
				};

				c->views = mkvector(pv);

				// INIT / ACT
				stack sv(false, cursor_manager_);

				sv.set_spacing(7);
				sv.add(c, percents(19.090909), true);
				sv.add(c, percents(26.363636), true);
				sv.add(c, percents(54.545454), true);

				// ACT
				sv.layout(make_appender(v), make_box(100, 164));

				// ASSERT
				placed_view reference1_views[] = {
					{ nullptr, nullptr_nv, create_rect(0, 0, 10, 10), 0 },
					{ nullptr, nullptr_nv, create_rect(0, 29, 100, 36), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(0, 36, 10, 46), 0 },
					{ nullptr, nullptr_nv, create_rect(0, 75, 100, 82), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(0, 82, 10, 92), 0 },
				};
				agge::box<int> reference1_box[] = {	{ 100, 29 }, { 100, 39 }, { 100, 82 },	};

				assert_equal_pred(reference1_views, v, eq());
				assert_equal(reference1_box, c->size_log);

				assert_not_null(v[1].regular);
				assert_not_null(v[3].regular);
				assert_not_equal(v[1].regular, v[3].regular);

				// INIT / ACT
				stack sh(true, cursor_manager_);

				sh.set_spacing(11);
				sh.add(c, percents(53), true);
				sh.add(c, percents(47), true);
				c->size_log.clear();
				v.clear();

				// ACT
				sh.layout(make_appender(v), make_box(194, 201));

				// ASSERT
				placed_view reference2_views[] = {
					{ nullptr, nullptr_nv, create_rect(0, 0, 10, 10), 0 },
					{ nullptr, nullptr_nv, create_rect(97, 0, 108, 201), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(108, 0, 118, 10), 0 },
				};
				agge::box<int> reference2_box[] = {	{ 97, 201 }, { 86, 201 },	};

				assert_equal_pred(reference2_views, v, eq());
				assert_equal(reference2_box, c->size_log);

				assert_not_null(v[1].regular);
			}


			test( SplitterViewsAreOnlyAddedBetweenResizableControls )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 11, 13), 0,	},
				};

				c->views = mkvector(pv);

				// INIT / ACT
				stack sv(false, cursor_manager_);

				sv.set_spacing(5);
				sv.add(c, percents(10));
				sv.add(c, percents(15));
				sv.add(c, percents(10), true);
				sv.add(c, percents(20), true);
				sv.add(c, percents(10));

				// ACT
				sv.layout(make_appender(v), make_box(20, 120));

				// ASSERT
				placed_view reference1_views[] = {
					{ nullptr, nullptr_nv, create_rect(0, 0, 11, 13), 0 },
					{ nullptr, nullptr_nv, create_rect(0, 15, 11, 28), 0 },
					{ nullptr, nullptr_nv, create_rect(0, 35, 11, 48), 0 },
					{ nullptr, nullptr_nv, create_rect(0, 45, 20, 50), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(0, 50, 11, 63), 0 },
					{ nullptr, nullptr_nv, create_rect(0, 75, 11, 88), 0 },
				};

				assert_equal_pred(reference1_views, v, eq());

				assert_equal(pv[0].regular, v[0].regular);
				assert_equal(pv[0].regular, v[1].regular);
				assert_equal(pv[0].regular, v[2].regular);
				assert_not_null(v[3].regular);
				assert_not_equal(pv[0].regular, v[3].regular);
				assert_equal(pv[0].regular, v[4].regular);
				assert_equal(pv[0].regular, v[5].regular);
			}


			test( SameSplitterViewsAreReturnedAmongLayouts )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v1, v2;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 10, 10), 0,	},
				};

				c->views = mkvector(pv);

				// INIT / ACT
				stack sv(false, cursor_manager_);

				sv.set_spacing(7);
				sv.add(c, percents(1.05), true);
				sv.add(c, percents(1.45), true);
				sv.add(c, percents(3), true);

				sv.layout(make_appender(v1), make_box(100, 164));

				// ACT
				sv.layout(make_appender(v2), make_box(100, 130));

				// ASSERT
				assert_equal(v1[1].regular, v2[1].regular);
				assert_equal(v1[3].regular, v2[3].regular);
			}


			test( DraggingSplitterChangesSizesOfNeighboringViewsV )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {	{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 10, 10), 0,	},	};
				stack s(false, cursor_manager_);
				auto layout_invalidations = 0;

				c->views = mkvector(pv);
				s.set_spacing(5);
				s.add(c, percents(33.333333), true);
				s.add(c, percents(66.666667), true);
				s.layout(make_appender(v), make_box(100, 105));

				auto splitter = v[1].regular;
				auto conn = s.layout_changed += [&] (bool hierarchy_changed) {
					layout_invalidations++;
					assert_is_false(hierarchy_changed);
				};

				// ACT
				splitter->mouse_down(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(0, layout_invalidations);

				// ACT
				splitter->mouse_move(mouse_input::left, 1000, 1);

				// ASSERT
				agge::box<int> reference1_box[] = {	{ 100, 34 }, { 100, 66 },	};

				v.clear();
				c->size_log.clear();
				s.layout(make_appender(v), make_box(100, 105));

				assert_equal(reference1_box, c->size_log);
				assert_equal(1, layout_invalidations);

				// ACT
				splitter->mouse_move(mouse_input::left, 1000, -13);

				// ASSERT
				agge::box<int> reference2_box[] = {	{ 100, 21 }, { 100, 79 },	};

				v.clear();
				c->size_log.clear();
				s.layout(make_appender(v), make_box(100, 105));

				assert_equal(reference2_box, c->size_log);
				assert_equal(2, layout_invalidations);
			}


			test( DraggingSplitterChangesSizesOfNeighboringViewsH )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {	{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 10, 10), 0,	},	};
				stack s(true, cursor_manager_);
				auto layout_invalidations = 0;

				c->views = mkvector(pv);
				s.set_spacing(5);
				s.add(c, percents(16.666667), true);
				s.add(c, percents(33.333333), true);
				s.add(c, percents(50), true);
				s.layout(make_appender(v), make_box(311, 105));

				auto splitter = v[3].regular;
				auto conn = s.layout_changed += [&] (bool hierarchy_changed) {
					layout_invalidations++;
					assert_is_false(hierarchy_changed);
				};

				// ACT
				splitter->mouse_down(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(0, layout_invalidations);

				// ACT
				splitter->mouse_move(mouse_input::left, -20, 1000);

				// ASSERT
				agge::box<int> reference1_box[] = {	{ 50, 105 }, { 81, 105 }, { 170, 105 },	};

				v.clear();
				c->size_log.clear();
				s.layout(make_appender(v), make_box(311, 105));

				assert_equal(reference1_box, c->size_log);
				assert_equal(1, layout_invalidations);
			}


			test( LayoutWithRemainZeroOrLessRemainderSizeProducesZeroSizedBoxes )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {	{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 10, 10), 0,	},	};
				stack s(true, cursor_manager_);

				c->views = mkvector(pv);
				s.set_spacing(5);
				s.add(c, percents(1), true);
				s.add(c, percents(2), true);

				// ACT
				s.layout(make_appender(v), make_box(5, 10));

				// ASSERT
				agge::box<int> reference_box[] = {	{ 0, 10 }, { 0, 10 },	};

				assert_equal(reference_box, c->size_log);

				// INIT
				v.clear();
				c->size_log.clear();

				// ACT
				s.layout(make_appender(v), make_box(4, 10));

				// ASSERT
				assert_equal(reference_box, c->size_log);
			}


			test( DraggingSplitterWithZeroLastSizeDoesNotAffectLayoutWhenSizeIsChanged )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {	{	make_shared<view>(), nullptr_nv, agge::zero(), 0,	},	};
				stack s(true, cursor_manager_);

				c->views = mkvector(pv);
				s.set_spacing(5);
				s.add(c, percents(33.333333), true);
				s.add(c, percents(66.666667), true);
				s.layout(make_appender(v), make_box(5, 10));

				auto splitter = v[1].regular;
				auto conn = s.layout_changed += [&] (bool /*hierarchy_changed*/) {
					assert_is_true(false);
				};

				// ACT
				splitter->mouse_down(mouse_input::left, 0, 0, 0);
				splitter->mouse_move(0, 3, 0);
				splitter->mouse_up(mouse_input::left, 0, 3, 0);

				// ASSERT
				agge::box<int> reference_box[] = {	{ 49, 105 }, { 98, 105 },	};

				v.clear();
				c->size_log.clear();
				s.layout(make_appender(v), make_box(152, 105));

				assert_equal(reference_box, c->size_log);
			}


			test( CursorChangeToAppropriateOnMouseEnteringSplitter )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {	{	make_shared<view>(), nullptr_nv, agge::zero(), 0,	},	};
				stack sh(true, cursor_manager_), sv(false, cursor_manager_);

				c->views = mkvector(pv);
				sh.set_spacing(5);
				sh.add(c, percents(1), true);
				sh.add(c, percents(2), true);
				sh.layout(make_appender(v), make_box(100, 100));
				auto splitterh = v[1].regular;
				sv.set_spacing(5);
				sv.add(c, percents(1), true);
				sv.add(c, percents(2), true);
				sv.layout(make_appender(v), make_box(100, 100));
				auto splitterv = v[4].regular;

				// ACT
				splitterh->mouse_enter();

				// ASSERT
				assert_equal(cursor_manager_->cursors[cursor_manager::h_resize], cursor_manager_->recently_set);
				assert_equal(1u, cursor_manager_->stack_level);
				assert_equal(1u, cursor_manager_->attempts);

				// ACT
				splitterh->mouse_leave();

				// ASSERT
				assert_equal(0u, cursor_manager_->stack_level);

				// ACT
				splitterv->mouse_enter();

				// ASSERT
				assert_equal(cursor_manager_->cursors[cursor_manager::v_resize], cursor_manager_->recently_set);
				assert_equal(1u, cursor_manager_->stack_level);
				assert_equal(2u, cursor_manager_->attempts);

				// ACT
				splitterv->mouse_leave();

				// ASSERT
				assert_equal(0u, cursor_manager_->stack_level);
			}


			test( ResizingIsLimitedToZeroSize )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {	{	make_shared<view>(), nullptr_nv, agge::zero(), 0,	},	};
				stack s(true, cursor_manager_);

				c->views = mkvector(pv);
				s.set_spacing(5);
				s.add(c, percents(33.333333), true);
				s.add(c, percents(66.666667), true);
				s.layout(make_appender(v), make_box(100, 10));

				auto splitter = v[1].regular;

				splitter->mouse_down(mouse_input::left, 0, 0, 0);

				// ACT
				splitter->mouse_move(0, -200, 0);

				// ASSERT
				agge::box<int> reference1_box[] = {	{ 0, 10 }, { 147, 10 },	};

				v.clear();
				c->size_log.clear();
				s.layout(make_appender(v), make_box(152, 10));

				assert_equal(reference1_box, c->size_log);

				// ACT
				splitter->mouse_move(0, 200, 0);

				// ASSERT
				agge::box<int> reference2_box[] = {	{ 95, 10 }, { 0, 10 },	};

				v.clear();
				c->size_log.clear();
				s.layout(make_appender(v), make_box(100, 10));

				assert_equal(reference2_box, c->size_log);
			}


			test( DraggableSplitterModifiesOnlyNeighboringControls )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {	{	make_shared<view>(), nullptr_nv, agge::zero(), 0,	},	};
				stack s(true, cursor_manager_);

				c->views = mkvector(pv);
				s.set_spacing(5);
				s.add(c, percents(25), true);
				s.add(c, percents(25));
				s.add(c, percents(30), true);
				s.add(c, percents(20), true);
				s.layout(make_appender(v), make_box(115, 10));

				auto splitter = v[3].regular;

				splitter->mouse_down(mouse_input::left, 0, 0, 0);

				// ACT
				splitter->mouse_move(0, 7, 0);

				// ASSERT
				agge::box<int> reference_box[] = {	{ 25, 10 }, { 25, 10 }, { 37, 10 }, { 13, 10 },	};

				c->size_log.clear();
				s.layout(make_appender(v), make_box(115, 10));
				assert_equal(reference_box, c->size_log);
			}


			test( HorizontalAllocatedSizeCannotBeLessThanControlMinSizeForAbsoluteSizes )
			{
				// INIT
				shared_ptr<mocks::control> controls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, agge::zero(), 0,	},
				};
				stack s(true, cursor_manager_);

				controls[1]->views = mkvector(pv);
				s.add(controls[0], pixels(10));
				s.add(controls[1], pixels(16));

				controls[0]->minimum_width = 13;
				controls[1]->minimum_width = 17;

				// ACT
				s.layout(make_appender(v), make_box(115, 10));

				// ASSERT
				agge::box<int> reference1_box[] = {	{ 13, 10 },	};
				agge::box<int> reference2_box[] = {	{ 17, 10 },	};
				placed_view reference_views[] = {
					{	nullptr, nullptr_nv, create_rect(13, 0, 13, 0), 0	},
				};

				assert_equal(reference1_box, controls[0]->size_log);
				assert_equal(reference2_box, controls[1]->size_log);
				assert_equal_pred(reference_views, v, eq());
			}


			test( VerticalAllocatedSizeCannotBeLessThanControlMinSizeForAbsoluteSizes )
			{
				// INIT
				shared_ptr<mocks::control> controls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, agge::zero(), 0,	},
				};
				stack s(false, cursor_manager_);

				controls[1]->views = mkvector(pv);
				s.add(controls[0], pixels(10));
				s.add(controls[1], pixels(16));

				controls[0]->minimum_height = 13;
				controls[1]->minimum_height = 17;

				// ACT
				s.layout(make_appender(v), make_box(115, 10));

				// ASSERT
				agge::box<int> reference1_box[] = {	{ 115, 13 },	};
				agge::box<int> reference2_box[] = {	{ 115, 17 },	};
				placed_view reference_views[] = {
					{	nullptr, nullptr_nv, create_rect(0, 13, 0, 13), 0	},
				};

				assert_equal(reference1_box, controls[0]->size_log);
				assert_equal(reference2_box, controls[1]->size_log);
				assert_equal_pred(reference_views, v, eq());
			}


			test( MinSizeIsTakenIntoAccountWhenCalculatingSpaceForRelativelySizedControls )
			{
				// INIT
				shared_ptr<mocks::control> controls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				vector<placed_view> v;
				stack sh(true, cursor_manager_);
				stack sv(false, cursor_manager_);

				sh.add(controls[0], percents(100));
				sh.add(controls[1], pixels(10));
				sv.add(controls[0], percents(100));
				sv.add(controls[1], pixels(10));

				controls[1]->minimum_width = 13;
				controls[1]->minimum_height = 17;

				// ACT
				sh.layout(make_appender(v), make_box(115, 10));

				// ASSERT
				agge::box<int> reference1_box[] = {	{ 102, 10 },	};
				int reference_for_height[] = {	10, 10,	};

				assert_equal(reference1_box, controls[0]->size_log);
				assert_equal(reference_for_height, controls[0]->for_height_log);
				assert_equal(reference_for_height, controls[1]->for_height_log);
				assert_is_empty(controls[0]->for_width_log);
				assert_is_empty(controls[1]->for_width_log);

				// ACT
				sv.layout(make_appender(v), make_box(115, 100));

				// ASSERT
				agge::box<int> reference2_box[] = {	{ 102, 10 }, { 115, 83 },	};
				int reference_for_width[] = {	115, 115,	};

				assert_equal(reference2_box, controls[0]->size_log);
				assert_equal(reference_for_height, controls[0]->for_height_log);
				assert_equal(reference_for_height, controls[1]->for_height_log);
				assert_equal(reference_for_width, controls[0]->for_width_log);
				assert_equal(reference_for_width, controls[1]->for_width_log);
			}


			test( SharedMinSizeIsCalculatedAsSumOfMinAndOrFixedSizes )
			{
				// INIT
				shared_ptr<mocks::control> controls[] = {
					make_shared<mocks::control>(), // use fixed size
					make_shared<mocks::control>(), // use min size
					make_shared<mocks::control>(), // use zero (for a relatively sized)
					make_shared<mocks::control>(), // use min size (for a relatively sized)
				};
				vector<placed_view> v;
				stack sh(true, cursor_manager_);
				stack sv(false, cursor_manager_);

				controls[1]->minimum_width = 19;
				controls[1]->minimum_height = 13;
				controls[3]->minimum_width = 17;
				controls[3]->minimum_height = 11;

				sh.set_spacing(3);
				sh.add(controls[0], pixels(31));
				sh.add(controls[1], pixels(10));
				sh.add(controls[2], percents(100));
				sv.set_spacing(4);
				sv.add(controls[0], pixels(32));
				sv.add(controls[1], pixels(10));
				sv.add(controls[2], percents(100));

				// ACT / ASSERT
				assert_equal(56, sh.min_width(131));
				assert_equal(53, sv.min_height(111));

				// ASSERT
				int reference_for_height1[] = {	131,	};
				int reference_for_width1[] = {	111,	};

				assert_equal(reference_for_height1, controls[0]->for_height_log);
				assert_equal(reference_for_height1, controls[1]->for_height_log);
				assert_equal(reference_for_height1, controls[2]->for_height_log);

				assert_equal(reference_for_width1, controls[0]->for_width_log);
				assert_equal(reference_for_width1, controls[1]->for_width_log);
				assert_equal(reference_for_width1, controls[2]->for_width_log);

				// INIT
				sh.add(controls[3], percents(100));
				sv.add(controls[3], percents(100));

				// ACT / ASSERT
				assert_equal(76, sh.min_width(121));
				assert_equal(68, sv.min_height(101));

				// ASSERT
				int reference_for_height2[] = {	131, 121,	};
				int reference_for_width2[] = {	111, 101,	};

				assert_equal(reference_for_height2, controls[0]->for_height_log);
				assert_equal(reference_for_height2, controls[1]->for_height_log);
				assert_equal(reference_for_height2, controls[2]->for_height_log);

				assert_equal(reference_for_width2, controls[0]->for_width_log);
				assert_equal(reference_for_width2, controls[1]->for_width_log);
				assert_equal(reference_for_width2, controls[2]->for_width_log);
			}

		end_test_suite
	}
}
