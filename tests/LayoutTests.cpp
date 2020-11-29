#include <wpl/layout.h>

#include "helpers.h"
#include "helpers-visual.h"
#include "mock-control.h"

#include <ut/assert.h>
#include <ut/test.h>
#include <wpl/view.h>

using namespace std;

namespace wpl
{
	static bool operator ==(const placed_view &lhs, const placed_view &rhs)
	{
		return lhs.regular == rhs.regular
			&& lhs.native == rhs.native
			&& lhs.location == rhs.location
			&& lhs.tab_order == rhs.tab_order;
	}

	namespace tests
	{
		namespace
		{
			const auto nullptr_nv = shared_ptr<native_view>();
		}

		begin_test_suite( StackLayoutTest )

			test( SingleItemOccupiesAbsoluteSizeAssigned )
			{
				// INIT
				auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv = {	make_shared<view>(), nullptr_nv, make_rect(1, 2, 3, 4), 1,	};

				c->views.push_back(pv);

				// INIT / ACT
				stack sh(0, true);
				stack sv(0, false);

				sh.add(c, 17, 190);
				sv.add(c, 91, 11);

				// ACT
				sh.layout(make_appender(v), make_box(100, 100));
				sv.layout(make_appender(v), make_box(100, 100));

				// ASSERT
				placed_view reference1_views[] = {
					{ pv.regular, nullptr_nv, make_rect(1, 2, 3, 4), 190 },
					{ pv.regular, nullptr_nv, make_rect(1, 2, 3, 4), 11 },
				};
				agge::box<int> reference1_box[] = {
					{ 17, 100 },
					{ 100, 91 },
				};

				assert_equal(reference1_views, v);
				assert_equal(reference1_box, c->size_log);

				// INIT / ACT
				stack sh2(100, true);
				stack sv2(10, false);

				sh2.add(c, 170, 1);
				sv2.add(c, 190, 2);

				// INIT
				v.clear();
				c->size_log.clear();

				// ACT
				sh2.layout(make_appender(v), make_box(1000, 1000));
				sv2.layout(make_appender(v), make_box(1000, 1000));

				// ASSERT
				placed_view reference2_views[] = {
					{ pv.regular, nullptr_nv, make_rect(1, 2, 3, 4), 1 },
					{ pv.regular, nullptr_nv, make_rect(1, 2, 3, 4), 2 },
				};
				agge::box<int> reference2_box[] = {
					{ 170, 1000 },
					{ 1000, 190 },
				};

				assert_equal(reference2_views, v);
				assert_equal(reference2_box, c->size_log);
			}


			test( LayoutSeveralWidgetsAbsolute )
			{
				// INIT
				shared_ptr<mocks::control> controls[] = {
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
				};
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, make_rect(0, 0, 30, 30), 1,	},
					{	make_shared<view>(), nullptr_nv, make_rect(1, 2, 10, 15), 1,	},
					{	make_shared<view>(), nullptr_nv, make_rect(3, 5, 30, 20), 1,	},
				};
				stack sh(0, true);

				controls[0]->views.push_back(pv[0]);
				controls[1]->views.push_back(pv[1]);
				controls[2]->views.push_back(pv[2]);

				sh.add(controls[0], 17, 1);
				sh.add(controls[1], 23, 2);

				// ACT
				sh.layout(make_appender(v), make_box(100, 100));

				// ASSERT
				placed_view reference1_views[] = {
					{ pv[0].regular, nullptr_nv, make_rect(0, 0, 30, 30), 1 },
					{ pv[1].regular, nullptr_nv, make_rect(1 + 17, 2, 10 + 17, 15), 2 },
				};
				agge::box<int> reference10_box[] = {	{ 17, 100 },	};
				agge::box<int> reference11_box[] = {	{ 23, 100 },	};

				assert_equal(reference1_views, v);
				assert_equal(reference10_box, controls[0]->size_log);
				assert_equal(reference11_box, controls[1]->size_log);

				// INIT
				stack sv(0, false);

				sv.add(controls[0], 90, 10);
				sv.add(controls[1], 80, 11);
				sv.add(controls[2], 55, 12);

				v.clear();
				controls[0]->size_log.clear();
				controls[1]->size_log.clear();

				// ACT
				sv.layout(make_appender(v), make_box(10, 250));

				placed_view reference2_views[] = {
					{ pv[0].regular, nullptr_nv, make_rect(0, 0, 30, 30), 10 },
					{ pv[1].regular, nullptr_nv, make_rect(1, 2 + 90, 10, 15 + 90), 11 },
					{ pv[2].regular, nullptr_nv, make_rect(3, 5 + 170, 30, 20 + 170), 12 },
				};
				agge::box<int> reference20_box[] = {	{ 10, 90 },	};
				agge::box<int> reference21_box[] = {	{ 10, 80 },	};
				agge::box<int> reference22_box[] = {	{ 10, 55 },	};

				assert_equal(reference2_views, v);
				assert_equal(reference20_box, controls[0]->size_log);
				assert_equal(reference21_box, controls[1]->size_log);
				assert_equal(reference22_box, controls[2]->size_log);
			}


			test( LayoutSeveralWidgetsAbsoluteSpaced )
			{
				// INIT
				shared_ptr<mocks::control> controls[] = {
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
					make_shared<mocks::control>(),
				};
				vector<placed_view> v;
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, make_rect(0, 0, 30, 30), 3,	},
					{	make_shared<view>(), nullptr_nv, make_rect(1, 2, 10, 15), 10,	},
					{	make_shared<view>(), nullptr_nv, make_rect(3, 5, 30, 20), 191,	},
				};
				stack sh(3, true);

				controls[0]->views.push_back(pv[0]);
				controls[1]->views.push_back(pv[1]);
				controls[2]->views.push_back(pv[2]);

				sh.add(controls[0], 17);
				sh.add(controls[1], 23);
				sh.add(controls[2], 13);

				// ACT
				sh.layout(make_appender(v), make_box(100, 77));

				// ASSERT
				placed_view reference1_views[] = {
					{ pv[0].regular, nullptr_nv, make_rect(0, 0, 30, 30), 3 },
					{ pv[1].regular, nullptr_nv, make_rect(1 + 17 + 3, 2, 10 + 17 + 3, 15), 10 },
					{ pv[2].regular, nullptr_nv, make_rect(3 + 17 + 3 + 23 + 3, 5, 30 + 17 + 3 + 23 + 3, 20), 191 },
				};
				agge::box<int> reference10_box[] = {	{ 17, 77 },	};
				agge::box<int> reference11_box[] = {	{ 23, 77 },	};
				agge::box<int> reference12_box[] = {	{ 13, 77 },	};

				assert_equal(reference1_views, v);
				assert_equal(reference10_box, controls[0]->size_log);
				assert_equal(reference11_box, controls[1]->size_log);
				assert_equal(reference12_box, controls[2]->size_log);
			}


			test( LayoutVSeveralWidgetsRelativelyAndAbsolutelySpacedWithInnerSpacing )
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
					{	make_shared<view>(), nullptr_nv, make_rect(0, 0, 30, 30), 1,	},
					{	make_shared<view>(), nullptr_nv, make_rect(1, 2, 10, 15), 2,	},
					{	make_shared<view>(), nullptr_nv, make_rect(3, 5, 30, 20), 3,	},
					{	make_shared<view>(), nullptr_nv, make_rect(3, 5, 30, 20), 4,	},
					{	make_shared<view>(), nullptr_nv, make_rect(3, 5, 30, 20), 4,	},
				};
				stack sh(3, true);

				controls[0]->views.push_back(pv[0]);
				controls[1]->views.push_back(pv[1]);
				controls[2]->views.push_back(pv[2]);
				controls[3]->views.push_back(pv[3]);
				controls[4]->views.push_back(pv[4]);

				sh.add(controls[0], 17);
				sh.add(controls[1], -3);
				sh.add(controls[2], 13);
				sh.add(controls[3], -1);

				// ACT
				sh.layout(make_appender(v), make_box(59, 20));

				// ASSERT
				placed_view reference1_views[] = {
					{ pv[0].regular, nullptr_nv, make_rect(0, 0, 30, 30), 1 },
					{ pv[1].regular, nullptr_nv, make_rect(1 + 17 + 3, 2, 10 + 17 + 3, 15), 2 },
					{ pv[2].regular, nullptr_nv, make_rect(3 + 17 + 3 + 15 + 3, 5, 30 + 17 + 3 + 15 + 3, 20), 3 },
					{ pv[3].regular, nullptr_nv, make_rect(3 + 17 + 3 + 15 + 3 + 13 + 3, 5, 30 + 17 + 3 + 15 + 3 + 13 + 3, 20), 4 },
				};
				agge::box<int> reference10_box[] = {	{ 17, 20 },	};
				agge::box<int> reference11_box[] = {	{ 15, 20 },	};
				agge::box<int> reference12_box[] = {	{ 13, 20 },	};
				agge::box<int> reference13_box[] = {	{ 5, 20 },	};

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
				agge::box<int> reference21_box[] = {	{ 15, 20 }, { 18, 20 },	};
				agge::box<int> reference22_box[] = {	{ 13, 20 }, { 13, 20 },	};
				agge::box<int> reference23_box[] = {	{ 5, 20 }, { 6, 20 },	};

				assert_equal(reference20_box, controls[0]->size_log);
				assert_equal(reference21_box, controls[1]->size_log);
				assert_equal(reference22_box, controls[2]->size_log);
				assert_equal(reference23_box, controls[3]->size_log);

				// INIT
				sh.add(controls[4], -2);

				// ACT
				sh.layout(make_appender(v), make_box(66, 20));

				// ASSERT
				agge::box<int> reference31_box[] = {	{ 15, 20 }, { 18, 20 }, { 12, 20 },	};
				agge::box<int> reference33_box[] = {	{ 5, 20 }, { 6, 20 }, { 4, 20 },	};
				agge::box<int> reference34_box[] = {	{ 8, 20 },	};

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
					{	make_shared<view>(), nullptr_nv, make_rect(0, 0, 10, 10), 1,	},
					{	make_shared<view>(), nullptr_nv, make_rect(10, 10, 30, 20), 2,	},
					{	make_shared<view>(), nullptr_nv, make_rect(0, 0, 30, 30), 3,	},
					{	make_shared<view>(), nullptr_nv, make_rect(3, 5, 30, 20), 4,	},
					{	make_shared<view>(), nullptr_nv, make_rect(7, 10, 30, 20), 4,	},
				};
				stack sh(5, true);

				controls[0]->views.push_back(pv[0]);
				controls[0]->views.push_back(pv[1]);
				controls[1]->views.push_back(pv[2]);
				controls[1]->views.push_back(pv[3]);
				controls[1]->views.push_back(pv[4]);

				sh.add(controls[0], 30, 100);
				sh.add(controls[1], 50);

				// ACT
				sh.layout(make_appender(v), make_box(150, 20));

				// ASSERT
				placed_view reference[] = {
					{ pv[0].regular, nullptr_nv, make_rect(0, 0, 10, 10), 100 },
					{ pv[1].regular, nullptr_nv, make_rect(10, 10, 30, 20), 100 },
					{ pv[2].regular, nullptr_nv, make_rect(35, 0, 65, 30), 3 },
					{ pv[3].regular, nullptr_nv, make_rect(38, 5, 65, 20), 4 },
					{ pv[4].regular, nullptr_nv, make_rect(42, 10, 65, 20), 4 },
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
				stack sv(0, false);

				control->views.assign(begin(pv), end(pv));

				sv.add(control, 1, 100);
				sv.add(control, 1);
				sv.add(control, 1, 13);

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
		end_test_suite


		begin_test_suite( PadLayoutTests )
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
					{	make_shared<view>(), nullptr_nv, make_rect(7, 13, 15, 30), 1,	},
					{	make_shared<view>(), nullptr_nv, make_rect(10, 20, 30, 200), 2,	},
				};

				c->views.push_back(pv[0]);
				c->views.push_back(pv[1]);

				// INIT / ACT
				auto p = pad_control(c, 1, 1);

				// ACT
				p->layout(make_appender(v), make_box(100, 100));

				// ASSERT
				placed_view reference1[] = {
					{ pv[0].regular, nullptr_nv, make_rect(8, 14, 16, 31), 1 },
					{ pv[1].regular, nullptr_nv, make_rect(11, 21, 31, 201), 2 },
				};

				assert_equal(reference1, v);

				// INIT / ACT
				p = pad_control(c, 3, 5);
				v.clear();

				// ACT
				p->layout(make_appender(v), make_box(100, 100));

				// ASSERT
				placed_view reference2[] = {
					{ pv[0].regular, nullptr_nv, make_rect(10, 18, 18, 35), 1 },
					{ pv[1].regular, nullptr_nv, make_rect(13, 25, 33, 205), 2 },
				};

				assert_equal(reference2, v);
			}
		end_test_suite
	}
}
