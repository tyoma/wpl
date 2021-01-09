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

		begin_test_suite( ResizableStackTests )

			shared_ptr<mocks::cursor_manager> cursor_manager_;

			init( Init )
			{
				cursor_manager_.reset(new mocks::cursor_manager);

				cursor_manager_->cursors[cursor_manager::arrow].reset(new cursor(10, 10, 1, 1));
				cursor_manager_->cursors[cursor_manager::h_resize].reset(new cursor(10, 10, 1, 1));
				cursor_manager_->cursors[cursor_manager::v_resize].reset(new cursor(10, 10, 1, 1));
				cursor_manager_->cursors[cursor_manager::hand].reset(new cursor(10, 10, 1, 1));
			}


			test( SingleChildOccupiesEntireStack )
			{
				// INIT
				const auto c = make_shared<mocks::control>();
				vector<placed_view> v;
				placed_view pv[] = {	{	make_shared<view>(), nullptr_nv, create_rect(1, 2, 3, 4), 1,	},	};

				c->views = mkvector(pv);

				// INIT / ACT
				resizable_stack sh(0, true, cursor_manager_), sv(0, false, cursor_manager_);

				sh.add(c, 1.3, 1);
				sv.add(c, 1000.0, 11);

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
				resizable_stack s(0, true, cursor_manager_);

				s.add(c[0], 1.3, 1);
				s.add(c[1], 1);

				// ACT
				s.layout(make_appender(v), make_box(110, 120));

				// ASSERT
				placed_view reference1_views[] = {
					{ nullptr, nullptr_nv, create_rect(0, 0, 10, 10), 0 },
					{ nullptr, nullptr_nv, create_rect(62, 0, 62, 120), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(62, 0, 81, 180), 171 },
				};
				agge::box<int> reference11_box[] = {	{ 62, 120 },	};
				agge::box<int> reference12_box[] = {	{ 48, 120 },	};

				assert_equal_pred(reference1_views, v, eq());
				assert_equal(pv[0].regular, v[0].regular);
				assert_equal(pv[1].regular, v[2].regular);
				assert_equal(reference11_box, c[0]->size_log);
				assert_equal(reference12_box, c[1]->size_log);

				// INIT
				c[0]->size_log.clear();
				c[1]->size_log.clear();
				v.clear();

				s.add(c[2], 5);

				// ACT
				s.layout(make_appender(v), make_box(200, 231));

				// ASSERT
				placed_view reference2_views[] = {
					{ nullptr, nullptr_nv, create_rect(0, 0, 10, 10), 0 },
					{ nullptr, nullptr_nv, create_rect(36, 0, 36, 231), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(36, 0, 55, 180), 171 },
					{ nullptr, nullptr_nv, create_rect(63, 0, 63, 231), 0 }, // splitter
					{ nullptr, nullptr_nv, create_rect(64, 0, 93, 20), 191 },
				};
				agge::box<int> reference21_box[] = {	{ 36, 231 },	};
				agge::box<int> reference22_box[] = {	{ 27, 231 },	};
				agge::box<int> reference23_box[] = {	{ 137, 231 },	};

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
				resizable_stack s(0, false, cursor_manager_);

				s.add(c, 1.05);
				s.add(c, 1.45);
				s.add(c, 3);

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
				resizable_stack sv(7, false, cursor_manager_);

				sv.add(c, 1.05);
				sv.add(c, 1.45);
				sv.add(c, 3);

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
				resizable_stack sh(11, true, cursor_manager_);

				sh.add(c, 0.53);
				sh.add(c, 0.47);
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
				resizable_stack sv(7, false, cursor_manager_);

				sv.add(c, 1.05);
				sv.add(c, 1.45);
				sv.add(c, 3);

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
				resizable_stack s(5, false, cursor_manager_);
				auto layout_invalidations = 0;

				c->views = mkvector(pv);
				s.add(c, 1);
				s.add(c, 2);
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
				resizable_stack s(5, true, cursor_manager_);
				auto layout_invalidations = 0;

				c->views = mkvector(pv);
				s.add(c, 1);
				s.add(c, 2);
				s.add(c, 3);
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
				resizable_stack s(5, true, cursor_manager_);

				c->views = mkvector(pv);
				s.add(c, 1);
				s.add(c, 2);

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
				resizable_stack s(5, true, cursor_manager_);

				c->views = mkvector(pv);
				s.add(c, 1);
				s.add(c, 2);
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
				resizable_stack sh(5, true, cursor_manager_), sv(5, false, cursor_manager_);

				c->views = mkvector(pv);
				sh.add(c, 1);
				sh.add(c, 2);
				sh.layout(make_appender(v), make_box(100, 100));
				auto splitterh = v[1].regular;
				sv.add(c, 1);
				sv.add(c, 2);
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

		end_test_suite
	}
}
