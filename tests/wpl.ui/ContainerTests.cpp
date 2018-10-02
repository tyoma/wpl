#include <wpl/ui/container.h>

#include "TestHelpers.h"
#include "Mockups.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;
using namespace std::placeholders;

namespace wpl
{
	namespace ui
	{
		inline bool operator ==(const container::positioned_view &lhs, const container::positioned_view &rhs)
		{
			return lhs.left == rhs.left && lhs.top == rhs.top && lhs.width == rhs.width && lhs.height == rhs.height
				&& lhs.child == rhs.child;
		}

		namespace tests
		{
			namespace
			{
				mocks::logging_layout_manager::position make_position(int x, int y, int width, int height)
				{
					mocks::logging_layout_manager::position p = { x, y, width, height };
					return p;
				}
			}

			begin_test_suite( ContainerTests )
				test( NoLayoutIsMadeOnResizingEmptyContainer )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);

					c.set_layout(lm);

					// ACT
					c.resize(100, 117);

					// ASSERT
					assert_is_empty(lm->reposition_log);
				}


				test( LayoutIsRecalculatedOnAddingAChild )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr< mocks::logging_visual<view> > v1(new mocks::logging_visual<view>());
					shared_ptr< mocks::logging_visual<view> > v2(new mocks::logging_visual<view>());

					c.set_layout(lm);
					c.resize(200, 317);

					// ACT
					c.add_view(v1);

					// ASSERT
					container::positioned_view reference1[] = { { 0, 0, 0, 0, v1 }, };

					assert_equal(1u, lm->reposition_log.size());
					assert_equal(make_pair(200u, 317u), lm->reposition_log[0]);
					assert_equal(reference1, lm->last_widgets);

					// INIT
					c.resize(150, 114);
					lm->reposition_log.clear();

					// ACT
					c.add_view(v2);

					// ASSERT
					container::positioned_view reference2[] = { { 0, 0, 0, 0, v1 }, { 0, 0, 0, 0, v2 }, };

					assert_equal(1u, lm->reposition_log.size());
					assert_equal(make_pair(150u, 114u), lm->reposition_log[0]);
					assert_equal(reference2, lm->last_widgets);
				}


				test( LayoutIsMadeOnNonEmptyContainerResize )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr< mocks::logging_visual<view> > v1(new mocks::logging_visual<view>());
					shared_ptr< mocks::logging_visual<view> > v2(new mocks::logging_visual<view>());

					c.set_layout(lm);
					c.add_view(v1);
					c.add_view(v2);
					lm->reposition_log.clear();

					// ACT
					c.resize(100, 117);

					// ASSERT
					assert_equal(1u, lm->reposition_log.size());
					assert_equal(make_pair(100u, 117u), lm->reposition_log[0]);

					// ACT
					c.resize(53, 91);

					// ASSERT
					assert_equal(2u, lm->reposition_log.size());
					assert_equal(make_pair(53u, 91u), lm->reposition_log[1]);
				}


				test( ChildrenAreResizedOnAdd )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr< mocks::logging_visual<view> > v1(new mocks::logging_visual<view>());
					shared_ptr< mocks::logging_visual<view> > v2(new mocks::logging_visual<view>());

					lm->positions.push_back(make_position(13, 17, 100, 200));
					lm->positions.push_back(make_position(90, 40, 110, 112));
					c.set_layout(lm);

					// ACT
					c.add_view(v1);

					// ASSERT
					assert_equal(1u, v1->resize_log.size());
					assert_equal(make_pair(100, 200), v1->resize_log[0]);

					// ACT
					c.add_view(v2);

					// ASSERT
					assert_equal(2u, v1->resize_log.size());
					assert_equal(make_pair(100, 200), v1->resize_log[1]);
					assert_equal(1u, v2->resize_log.size());
					assert_equal(make_pair(110, 112), v2->resize_log[0]);
				}


				test( ChildrenAreResizedOnContainerResize )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr< mocks::logging_visual<view> > v1(new mocks::logging_visual<view>());
					shared_ptr< mocks::logging_visual<view> > v2(new mocks::logging_visual<view>());

					c.set_layout(lm);
					c.add_view(v1);
					c.add_view(v2);
					lm->positions.push_back(make_position(13, 17, 100, 200));
					lm->positions.push_back(make_position(90, 40, 110, 112));
					v1->resize_log.clear();
					v2->resize_log.clear();

					// ACT
					c.resize(1000, 1000);

					// ASSERT
					assert_equal(1u, v1->resize_log.size());
					assert_equal(1u, v2->resize_log.size());
					assert_equal(make_pair(100, 200), v1->resize_log[0]);
					assert_equal(make_pair(110, 112), v2->resize_log[0]);

					// INIT
					lm->positions[0] = make_position(13, 17, 10, 20);
					lm->positions[1] = make_position(90, 40, 11, 12);

					// ACT
					c.resize(1001, 1002);

					// ASSERT
					assert_equal(2u, v1->resize_log.size());
					assert_equal(2u, v2->resize_log.size());
					assert_equal(make_pair(10, 20), v1->resize_log[1]);
					assert_equal(make_pair(11, 12), v2->resize_log[1]);
				}


				test( ChildViewsAreDrawnWithCorrespondentOffset )
				{
					// INIT
					container c1, c2;
					shared_ptr<mocks::logging_layout_manager> lm1(new mocks::logging_layout_manager);
					shared_ptr<mocks::logging_layout_manager> lm2(new mocks::logging_layout_manager);
					shared_ptr< mocks::logging_visual<view> > v1(new mocks::logging_visual<view>());
					shared_ptr< mocks::logging_visual<view> > v2(new mocks::logging_visual<view>());
					gcontext::surface_type surface(1000, 1000, 0);
					gcontext::renderer_type ren(1);
					gcontext::rasterizer_ptr ras(new gcontext::rasterizer_type);

					gcontext ctx(surface, ren, make_rect(103, 71, 130, 110));

					lm1->positions.push_back(make_position(13, 17, 1000, 1000));
					lm2->positions.push_back(make_position(90, 40, 1000, 1000));

					// INIT / ACT
					c1.set_layout(lm1);
					c1.add_view(v1);
					c2.set_layout(lm2);
					c2.add_view(v2);

					// ACT
					c1.draw(ctx, ras);

					// ASSERT
					assert_equal(1u, v1->update_area_log.size());
					assert_equal(make_rect(103 - 13, 71 - 17, 130 - 13, 110 - 17), v1->update_area_log[0]);

					// ACT
					c2.draw(ctx, ras);

					// ASSERT
					assert_equal(1u, v2->update_area_log.size());
					assert_equal(make_rect(103 - 90, 71 - 40, 130 - 90, 110 - 40), v2->update_area_log[0]);
				}


				test( MultipleChildViewsAreDrawnWithCorrespondentOffset )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr< mocks::logging_visual<view> > v1(new mocks::logging_visual<view>());
					shared_ptr< mocks::logging_visual<view> > v2(new mocks::logging_visual<view>());
					gcontext::surface_type surface(1000, 1000, 0);
					gcontext::renderer_type ren(1);
					gcontext::rasterizer_ptr ras(new gcontext::rasterizer_type);

					gcontext ctx(surface, ren, make_rect(13, 17, 137, 100));

					lm->positions.push_back(make_position(13, 17, 1000, 1000));
					lm->positions.push_back(make_position(91, 45, 1000, 1000));

					// INIT / ACT
					c.set_layout(lm);
					c.add_view(v1);
					c.add_view(v2);

					// ACT
					c.draw(ctx, ras);

					// ASSERT
					assert_equal(1u, v1->update_area_log.size());
					assert_equal(make_rect(13 - 13, 17 - 17, 137 - 13, 100 - 17), v1->update_area_log[0]);
					assert_equal(1u, v2->update_area_log.size());
					assert_equal(make_rect(13 - 91, 17 - 45, 137 - 91, 100 - 45), v2->update_area_log[0]);
				}


				vector<agge::rect_i> invalidation_log;

				void on_invalidate(const agge::rect_i *area)
				{
					assert_not_null(area);
					invalidation_log.push_back(*area);
				}

				test( ChildInvalidationIsBroadcastForChildViewRect )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr< mocks::logging_visual<view> > v1(new mocks::logging_visual<view>());
					shared_ptr< mocks::logging_visual<view> > v2(new mocks::logging_visual<view>());
					slot_connection conn = c.invalidate += bind(&ContainerTests::on_invalidate, this, _1);

					lm->positions.push_back(make_position(13, 17, 31, 23));
					lm->positions.push_back(make_position(91, 45, 1000, 1000));
					c.set_layout(lm);
					c.add_view(v1);
					c.add_view(v2);

					// ACT
					v1->invalidate(0);

					// ASSERT
					assert_equal(1u, invalidation_log.size());
					assert_equal(make_rect(13, 17, 44, 40), invalidation_log[0]);

					// ACT
					v2->invalidate(0);

					// ASSERT
					assert_equal(2u, invalidation_log.size());
					assert_equal(make_rect(91, 45, 1091, 1045), invalidation_log[1]);
				}


				test( ChildInvalidationTranslatesInvalidArea )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr< mocks::logging_visual<view> > v1(new mocks::logging_visual<view>());
					shared_ptr< mocks::logging_visual<view> > v2(new mocks::logging_visual<view>());
					slot_connection conn = c.invalidate += bind(&ContainerTests::on_invalidate, this, _1);
					agge::rect_i a1 = { 10, 11, 20, 22 }, a2 = { -10, -1, 17, 20 };

					lm->positions.push_back(make_position(13, 17, 31, 23));
					lm->positions.push_back(make_position(91, 45, 1000, 1000));
					c.set_layout(lm);
					c.add_view(v1);
					c.add_view(v2);

					// ACT
					v1->invalidate(&a1);

					// ASSERT
					assert_equal(1u, invalidation_log.size());
					assert_equal(make_rect(23, 28, 33, 39), invalidation_log[0]);

					// ACT
					v2->invalidate(&a1);

					// ASSERT
					assert_equal(2u, invalidation_log.size());
					assert_equal(make_rect(101, 56, 111, 67), invalidation_log[1]);

					// ACT
					v1->invalidate(&a2);

					// ASSERT
					assert_equal(3u, invalidation_log.size());
					assert_equal(make_rect(3, 16, 30, 37), invalidation_log[2]);
				}
			end_test_suite
		}
	}
}
