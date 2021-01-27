#include <wpl/layout.h>

#include <tests/common/helpers.h>
#include <tests/common/helpers-visual.h>
#include <tests/common/mock-control.h>
#include <ut/assert.h>
#include <ut/test.h>
#include <wpl/helpers.h>
#include <wpl/view.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			const auto nullptr_nv = shared_ptr<native_view>();

			struct eq
			{
				bool operator ()(const placed_view &lhs, const placed_view &rhs) const
				{
					return lhs.regular == rhs.regular && lhs.native == rhs.native && lhs.location == rhs.location
						&& lhs.tab_order == rhs.tab_order && lhs.overlay == rhs.overlay;
				}
			};
		}

		begin_test_suite( StaggeredLayoutTests )
			test( AddingAViewForcesLayoutRecalculate )
			{
				// INIT
				staggered l;
				auto layout_forced = 0;
				slot_connection conn = l.layout_changed += [&] (bool hierarchy_changed) {
					layout_forced++;
					assert_is_true(hierarchy_changed);
				};

				// ACT
				l.add(make_shared<mocks::control>());

				// ASSERT
				assert_equal(1, layout_forced);

				// ACT
				l.add(make_shared<mocks::control>());

				// ASSERT
				assert_equal(2, layout_forced);
			}


			test( ChangingBaseWidthLeadsToLayoutChange )
			{
				// INIT
				staggered l;
				auto layout_forced = 0;
				slot_connection conn = l.layout_changed += [&] (bool hierarchy_changed) {
					layout_forced++;
					assert_is_false(hierarchy_changed);
				};

				// ACT
				l.set_base_width(pixels(123));

				// ASSERT
				assert_equal(1, layout_forced);
			}


			test( SingleChildOccupiesFullSizeUpToTheDoubleBaseWidth )
			{
				// INIT
				staggered l;
				const auto ctl = make_shared<mocks::control>();
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(0, 0, 0, 0), 1, false },
				};
				vector<placed_view> v;

				ctl->views = mkvector(pv), l.add(ctl);
				l.set_base_width(pixels(30));

				// ACT
				l.layout(make_appender(v), create_box(30, 1000));

				// ASSERT
				box<int> reference1_box[] = {	create_box(30, 0),	};

				assert_equal(reference1_box, ctl->size_log);
				assert_equal(pv, v);

				// INIT
				ctl->size_log.clear();
				v.clear();

				// ACT
				l.layout(make_appender(v), create_box(37, 1000));

				// ASSERT
				box<int> reference2_box[] = {	create_box(37, 0),	};

				assert_equal(reference2_box, ctl->size_log);
				assert_equal(pv, v);

				// INIT
				ctl->size_log.clear();

				// ACT
				l.layout(make_appender(v), create_box(59, 1000));

				// ASSERT
				box<int> reference3_box[] = {	create_box(59, 0),	};

				assert_equal(reference3_box, ctl->size_log);

				// INIT
				ctl->size_log.clear();

				// ACT
				l.layout(make_appender(v), create_box(60, 1000));

				// ASSERT
				box<int> reference4_box[] = {	create_box(30, 0),	};

				assert_equal(reference4_box, ctl->size_log);

				// INIT
				ctl->size_log.clear();

				// ACT
				l.layout(make_appender(v), create_box(64, 1000));

				// ASSERT
				box<int> reference5_box[] = {	create_box(32, 0),	};

				assert_equal(reference5_box, ctl->size_log);

				// INIT
				ctl->size_log.clear();

				// ACT
				l.layout(make_appender(v), create_box(81, 1000));

				// ASSERT
				box<int> reference6_box[] = {	create_box(41, 0),	};

				assert_equal(reference6_box, ctl->size_log);

				// INIT
				ctl->size_log.clear();

				// ACT
				l.layout(make_appender(v), create_box(91, 1000));

				// ASSERT
				box<int> reference7_box[] = {	create_box(30 /*30.333*/, 0),	};

				assert_equal(reference7_box, ctl->size_log);

				// INIT
				ctl->size_log.clear();

				// ACT
				l.layout(make_appender(v), create_box(92, 1000));

				// ASSERT
				box<int> reference8_box[] = {	create_box(31 /*30.666*/, 0),	};

				assert_equal(reference8_box, ctl->size_log);
			}


			test( SingleControlOccupiesFullContainerWhenItIsNarrowerThanBaseWidth )
			{
				// INIT
				staggered l;
				const auto ctl = make_shared<mocks::control>();
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(10, 11, 13, 21), 3, true},
				};
				vector<placed_view> v;

				ctl->views = mkvector(pv), l.add(ctl);
				l.set_base_width(pixels(45));

				// ACT
				l.layout(make_appender(v), create_box(45, 1000));

				// ASSERT
				box<int> reference1_box[] = {	create_box(45, 0),	};

				assert_equal(reference1_box, ctl->size_log);
				assert_equal(pv, v);

				// INIT
				ctl->size_log.clear();

				// ACT
				l.layout(make_appender(v), create_box(31, 1000));

				// ASSERT
				box<int> reference2_box[] = {	create_box(31, 0),	};

				assert_equal(reference2_box, ctl->size_log);
			}


			test( SeveralControlsAreLaidOutWithAdjustedWidth )
			{
				// INIT
				staggered l;
				shared_ptr<mocks::control> ctls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(10, 11, 13, 21), 3, false	},
					{	make_shared<view>(), nullptr_nv, create_rect(1, 2, 3, 4), 4, false	},
					{	make_shared<view>(), nullptr_nv, create_rect(5, 5, 20, 25), 7, false	},
				};
				vector<placed_view> v;

				ctls[0]->minimum_height = 1, ctls[0]->views.push_back(pv[0]), l.add(ctls[0]);
				ctls[1]->minimum_height = 1, ctls[1]->views.push_back(pv[1]), l.add(ctls[1]);
				ctls[2]->minimum_height = 1, ctls[2]->views.push_back(pv[2]), l.add(ctls[2]);
				l.set_base_width(pixels(17));

				// ACT
				l.layout(make_appender(v), create_box(51, 1000));

				// ASSERT
				box<int> reference1_box[] = {	create_box(17, 1),	};
				placed_view reference1_views[] = {
					{	pv[0].regular, nullptr_nv, create_rect(10, 11, 13, 21), 3, false	},
					{	pv[1].regular, nullptr_nv, create_rect(1 + 17, 2, 3 + 17, 4), 4, false	},
					{	pv[2].regular, nullptr_nv, create_rect(5 + 34, 5, 20 + 34, 25), 7, false	},
				};

				assert_equal(reference1_box, ctls[0]->size_log);
				assert_equal(reference1_box, ctls[1]->size_log);
				assert_equal(reference1_box, ctls[2]->size_log);
				assert_equal(reference1_views, v);

				// INIT
				ctls[0]->size_log.clear(), ctls[1]->size_log.clear(), ctls[2]->size_log.clear();
				v.clear();

				// ACT
				l.layout(make_appender(v), create_box(52, 1000));

				// ASSERT
				box<int> reference2_box[] = {	create_box(18, 1),	};
				placed_view reference2_views[] = {
					{	pv[0].regular, nullptr_nv, create_rect(10, 11, 13, 21), 3, false	},
					{	pv[1].regular, nullptr_nv, create_rect(1 + 17, 2, 3 + 17, 4), 4, false	},
					{	pv[2].regular, nullptr_nv, create_rect(5 + 35, 5, 20 + 35, 25), 7, false	},
				};

				assert_equal(reference1_box, ctls[0]->size_log);
				assert_equal(reference2_box, ctls[1]->size_log);
				assert_equal(reference1_box, ctls[2]->size_log);
				assert_equal(reference2_views, v);

				// INIT
				ctls[0]->size_log.clear(), ctls[1]->size_log.clear(), ctls[2]->size_log.clear();
				v.clear();

				// ACT
				l.layout(make_appender(v), create_box(53, 1000));

				// ASSERT
				placed_view reference3_views[] = {
					{	pv[0].regular, nullptr_nv, create_rect(10, 11, 13, 21), 3, false	},
					{	pv[1].regular, nullptr_nv, create_rect(1 + 18, 2, 3 + 18, 4), 4, false	},
					{	pv[2].regular, nullptr_nv, create_rect(5 + 35, 5, 20 + 35, 25), 7, false	},
				};

				assert_equal(reference2_box, ctls[0]->size_log);
				assert_equal(reference1_box, ctls[1]->size_log);
				assert_equal(reference2_box, ctls[2]->size_log);
				assert_equal(reference3_views, v);

				// INIT
				ctls[0]->size_log.clear(), ctls[1]->size_log.clear(), ctls[2]->size_log.clear();
				v.clear();

				// ACT
				l.layout(make_appender(v), create_box(68, 1000));

				// ASSERT
				assert_equal(reference1_box, ctls[0]->size_log);
				assert_equal(reference1_box, ctls[1]->size_log);
				assert_equal(reference1_box, ctls[2]->size_log);
				assert_equal(reference1_views, v);
			}


			test( MinHeightIsAssignedToLaidOutControls )
			{
				// INIT
				staggered l;
				shared_ptr<mocks::control> ctls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				placed_view pv[] = {
					{	make_shared<view>(), nullptr_nv, create_rect(10, 11, 13, 21), 3, false	},
					{	make_shared<view>(), nullptr_nv, create_rect(1, 2, 3, 4), 4, false	},
					{	make_shared<view>(), nullptr_nv, create_rect(5, 5, 20, 25), 7, false	},
				};
				vector<placed_view> v;

				ctls[0]->minimum_height = 40, ctls[0]->views.push_back(pv[0]), l.add(ctls[0]);
				ctls[1]->minimum_height = 37, ctls[1]->views.push_back(pv[1]), l.add(ctls[1]);
				ctls[2]->minimum_height = 51, ctls[2]->views.push_back(pv[2]), l.add(ctls[2]);
				l.set_base_width(pixels(17));

				// ACT
				l.layout(make_appender(v), create_box(51, 1000));

				// ASSERT
				box<int> reference_box1[] = {	create_box(17, 40),	};
				box<int> reference_box2[] = {	create_box(17, 37),	};
				box<int> reference_box3[] = {	create_box(17, 51),	};
				placed_view reference_views[] = {
					{	pv[0].regular, nullptr_nv, create_rect(10, 11, 13, 21), 3, false	},
					{	pv[1].regular, nullptr_nv, create_rect(1 + 17, 2, 3 + 17, 4), 4, false	},
					{	pv[2].regular, nullptr_nv, create_rect(5 + 34, 5, 20 + 34, 25), 7, false	},
				};

				assert_equal(reference_box1, ctls[0]->size_log);
				assert_equal(reference_box2, ctls[1]->size_log);
				assert_equal(reference_box3, ctls[2]->size_log);
				assert_equal(reference_views, v);
			}


			test( ColumnsWidthIsPassedToAChildWhenCalculatingMiniumHeight )
			{
				// INIT
				staggered l;
				const auto ctl = make_shared<mocks::control>();
				vector<placed_view> v;

				l.add(ctl);
				l.set_base_width(pixels(17));

				// ACT
				l.layout(make_appender(v), create_box(19, 100));

				// ASSERT
				int reference1[] = {	19,	};

				assert_equal(reference1, ctl->for_width_log);

				// ACT
				l.layout(make_appender(v), create_box(29, 100));

				// ASSERT
				int references2[] = {	19, 29,	};

				assert_equal(references2, ctl->for_width_log);
			}


			test( OverflownControlsOccupyTheHighestPossiblePosition )
			{
				// INIT
				staggered l;
				shared_ptr<mocks::control> ctls[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(), make_shared<mocks::control>(),
					make_shared<mocks::control>(), make_shared<mocks::control>(), make_shared<mocks::control>(),
					make_shared<mocks::control>(), make_shared<mocks::control>(), make_shared<mocks::control>(),
				};
				placed_view pv = {	nullptr, nullptr_nv, zero(), 0, false	};
				vector<placed_view> v;

				ctls[0]->minimum_height = 40, pv.tab_order = 1, ctls[0]->views.push_back(pv), l.add(ctls[0]);
				ctls[1]->minimum_height = 37, pv.tab_order = 2, ctls[1]->views.push_back(pv), l.add(ctls[1]);
				ctls[2]->minimum_height = 51, pv.tab_order = 3, ctls[2]->views.push_back(pv), l.add(ctls[2]);
				ctls[3]->minimum_height = 2, pv.tab_order = 4, ctls[3]->views.push_back(pv), l.add(ctls[3]);
				ctls[4]->minimum_height = 10, pv.tab_order = 5, ctls[4]->views.push_back(pv), l.add(ctls[4]);
				ctls[5]->minimum_height = 20, pv.tab_order = 6, ctls[5]->views.push_back(pv), l.add(ctls[5]);
				l.set_base_width(pixels(20));

				// ACT
				l.layout(make_appender(v), create_box(60, 1000));

				// ASSERT
				placed_view reference1_views[] = {
					{	nullptr, nullptr_nv, create_rect(0, 0, 0, 0), 1, false	},
					{	nullptr, nullptr_nv, create_rect(20, 0, 20, 0), 2, false	},
					{	nullptr, nullptr_nv, create_rect(40, 0, 40, 0), 3, false	},
					{	nullptr, nullptr_nv, create_rect(20, 37, 20, 37), 4, false	},
					{	nullptr, nullptr_nv, create_rect(20, 39, 20, 39), 5, false	},
					{	nullptr, nullptr_nv, create_rect(0, 40, 0, 40), 6, false	},
				};

				assert_equal(reference1_views, v);

				// INIT
				ctls[6]->minimum_height = 15, pv.tab_order = 7, ctls[6]->views.push_back(pv), l.add(ctls[6]);
				ctls[7]->minimum_height = 15, pv.tab_order = 8, ctls[7]->views.push_back(pv), l.add(ctls[7]);
				ctls[8]->minimum_height = 15, pv.tab_order = 9, ctls[8]->views.push_back(pv), l.add(ctls[8]);
				v.clear();

				// ACT
				l.layout(make_appender(v), create_box(60, 1000));

				// ASSERT
				placed_view reference2_views[] = {
					{	nullptr, nullptr_nv, create_rect(0, 0, 0, 0), 1, false	},
					{	nullptr, nullptr_nv, create_rect(20, 0, 20, 0), 2, false	},
					{	nullptr, nullptr_nv, create_rect(40, 0, 40, 0), 3, false	}, // 51
					{	nullptr, nullptr_nv, create_rect(20, 37, 20, 37), 4, false	},
					{	nullptr, nullptr_nv, create_rect(20, 39, 20, 39), 5, false	}, // 49
					{	nullptr, nullptr_nv, create_rect(0, 40, 0, 40), 6, false	}, // 60
					{	nullptr, nullptr_nv, create_rect(20, 49, 20, 49), 7, false	},
					{	nullptr, nullptr_nv, create_rect(40, 51, 40, 51), 8, false	},
					{	nullptr, nullptr_nv, create_rect(0, 60, 0, 60), 9, false	},
				};

				assert_equal(reference2_views, v);
			}
		end_test_suite
	}
}
