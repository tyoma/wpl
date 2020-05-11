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
		{	return lhs.location == rhs.location && lhs.child == rhs.child;	}

		inline void increment(int *v)
		{	++*v;	}

		namespace tests
		{
			begin_test_suite( ContainerTests )
				vector<visual::positioned_native_view> nviews;

				test( NoLayoutIsMadeOnResizingEmptyContainer )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);

					c.set_layout(lm);

					// ACT
					c.resize(100, 117, nviews);

					// ASSERT
					assert_is_empty(lm->reposition_log);
				}


				test( AddingAViewForcesLayoutRecalculate )
				{
					// INIT
					container c;
					shared_ptr<view> v1(new view);
					shared_ptr<view> v2(new view);
					shared_ptr<layout_manager> lm(new mocks::logging_layout_manager);
					int layout_forced = 0;
					slot_connection conn = c.force_layout += bind(&increment, &layout_forced);

					// ACT
					c.add_view(v1);

					// ASSERT
					assert_equal(1, layout_forced);

					// ACT
					c.add_view(v2);

					// ASSERT
					assert_equal(2, layout_forced);
				}


				test( ForceLyaoutIsPropagatedUpstream )
				{
					// INIT
					container c;
					shared_ptr<view> v1(new view);
					shared_ptr<view> v2(new view);
					shared_ptr<layout_manager> lm(new mocks::logging_layout_manager);
					int layout_forced = 0;

					c.add_view(v1);
					c.add_view(v2);

					slot_connection conn = c.force_layout += bind(&increment, &layout_forced);

					// ACT
					v1->force_layout();

					// ASSERT
					assert_equal(1, layout_forced);

					// ACT
					v2->force_layout();

					// ASSERT
					assert_equal(2, layout_forced);
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
					c.resize(100, 117, nviews);

					// ASSERT
					assert_equal(1u, lm->reposition_log.size());
					assert_equal(make_pair(100u, 117u), lm->reposition_log[0]);

					// ACT
					c.resize(53, 91, nviews);

					// ASSERT
					assert_equal(2u, lm->reposition_log.size());
					assert_equal(make_pair(53u, 91u), lm->reposition_log[1]);
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
					c.resize(1000, 1000, nviews);

					// ASSERT
					assert_equal(1u, v1->resize_log.size());
					assert_equal(1u, v2->resize_log.size());
					assert_equal(make_pair(100, 200), v1->resize_log[0]);
					assert_equal(make_pair(110, 112), v2->resize_log[0]);

					// INIT
					lm->positions[0] = make_position(13, 17, 10, 20);
					lm->positions[1] = make_position(90, 40, 11, 12);

					// ACT
					c.resize(1001, 1002, nviews);

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
					c1.resize(1000, 1000, nviews);
					c2.resize(1000, 1000, nviews);

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
					c.resize(1000, 1000, nviews);

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
					c.resize(1000, 1000, nviews);

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
					c.resize(1000, 1000, nviews);

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


				test( MouseEventsAreRedirectedWithOffset )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr< mocks::logging_mouse_input<view> > v1(new mocks::logging_mouse_input<view>());
					shared_ptr< mocks::logging_mouse_input<view> > v2(new mocks::logging_mouse_input<view>());

					lm->positions.push_back(make_position(13, 17, 31, 23));
					lm->positions.push_back(make_position(91, 45, 100, 100));
					c.set_layout(lm);
					c.add_view(v1);
					c.add_view(v2);
					c.resize(1000, 1000, nviews);

					// ACT
					c.mouse_move(mouse_input::left, 14, 19);
					c.mouse_move(mouse_input::right, 40, 37);
					c.mouse_move(mouse_input::middle, 100, 50);
					c.mouse_move(mouse_input::left, 150, 140);

					// ASSERT
					mocks::mouse_event reference1[] = {
						mocks::me_enter(),
						mocks::me_move(mouse_input::left, 1, 2),
						mocks::me_move(mouse_input::right, 27, 20),
						mocks::me_leave(),
					};
					mocks::mouse_event reference2[] = {
						mocks::me_enter(),
						mocks::me_move(mouse_input::middle, 9, 5),
						mocks::me_move(mouse_input::left, 59, 95),
					};

					assert_equal(reference1, v1->events_log);
					assert_equal(reference2, v2->events_log);

					// INIT
					v1->events_log.clear();
					v2->events_log.clear();

					// ACT
					c.mouse_down(mouse_input::left, mouse_input::middle | mouse_input::right, 14, 19);
					c.mouse_down(mouse_input::right, mouse_input::middle, 17, 20);
					c.mouse_down(mouse_input::left, mouse_input::middle | mouse_input::right, 101, 51);
					c.mouse_down(mouse_input::right, mouse_input::middle, 110, 53);

					// ASSERT
					mocks::mouse_event reference3[] = {
						mocks::me_enter(),
						mocks::me_down(mouse_input::left, mouse_input::middle | mouse_input::right, 1, 2),
						mocks::me_down(mouse_input::right, mouse_input::middle, 4, 3),
						mocks::me_leave(),
					};
					mocks::mouse_event reference4[] = {
						mocks::me_leave(),
						mocks::me_enter(),
						mocks::me_down(mouse_input::left, mouse_input::middle | mouse_input::right, 10, 6),
						mocks::me_down(mouse_input::right, mouse_input::middle, 19, 8),
					};

					assert_equal(reference3, v1->events_log);
					assert_equal(reference4, v2->events_log);

					// INIT
					v1->events_log.clear();
					v2->events_log.clear();

					// ACT
					c.mouse_up(mouse_input::left, mouse_input::middle | mouse_input::right, 14, 19);
					c.mouse_up(mouse_input::right, mouse_input::middle, 17, 20);
					c.mouse_up(mouse_input::left, mouse_input::middle | mouse_input::right, 101, 51);
					c.mouse_up(mouse_input::right, mouse_input::middle, 110, 53);

					// ASSERT
					mocks::mouse_event reference5[] = {
						mocks::me_enter(),
						mocks::me_up(mouse_input::left, mouse_input::middle | mouse_input::right, 1, 2),
						mocks::me_up(mouse_input::right, mouse_input::middle, 4, 3),
						mocks::me_leave(),
					};
					mocks::mouse_event reference6[] = {
						mocks::me_leave(),
						mocks::me_enter(),
						mocks::me_up(mouse_input::left, mouse_input::middle | mouse_input::right, 10, 6),
						mocks::me_up(mouse_input::right, mouse_input::middle, 19, 8),
					};

					assert_equal(reference5, v1->events_log);
					assert_equal(reference6, v2->events_log);

					// INIT
					v1->events_log.clear();
					v2->events_log.clear();

					// ACT
					c.mouse_double_click(mouse_input::left, mouse_input::middle | mouse_input::right, 14, 19);
					c.mouse_double_click(mouse_input::right, mouse_input::middle, 17, 20);
					c.mouse_double_click(mouse_input::left, mouse_input::middle | mouse_input::right, 101, 51);
					c.mouse_double_click(mouse_input::right, mouse_input::middle, 110, 53);

					// ASSERT
					mocks::mouse_event reference7[] = {
						mocks::me_enter(),
						mocks::me_double_click(mouse_input::left, mouse_input::middle | mouse_input::right, 1, 2),
						mocks::me_double_click(mouse_input::right, mouse_input::middle, 4, 3),
						mocks::me_leave(),
					};
					mocks::mouse_event reference8[] = {
						mocks::me_leave(),
						mocks::me_enter(),
						mocks::me_double_click(mouse_input::left, mouse_input::middle | mouse_input::right, 10, 6),
						mocks::me_double_click(mouse_input::right, mouse_input::middle, 19, 8),
					};

					assert_equal(reference7, v1->events_log);
					assert_equal(reference8, v2->events_log);
				}


				test( MouseEnterAndMouseLeaveAreGeneratedWhenMouseCrossesBounds )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr< mocks::logging_mouse_input<view> > v(new mocks::logging_mouse_input<view>());

					lm->positions.push_back(make_position(13, 17, 31, 23));
					c.set_layout(lm);
					c.add_view(v);
					c.resize(1000, 1000, nviews);

					// ACT
					c.mouse_move(0, 13, 17);

					// ASSERT
					mocks::mouse_event reference1[] = { mocks::me_enter(), mocks::me_move(0, 0, 0), };

					assert_equal(reference1, v->events_log);

					// INIT
					v->events_log.clear();

					// ACT
					c.mouse_move(0, 13, 16);
					c.mouse_move(0, 12, 17);

					// ASSERT
					mocks::mouse_event reference2[] = { mocks::me_leave(), };

					assert_equal(reference2, v->events_log);

					// INIT
					v->events_log.clear();

					// ACT
					c.mouse_move(0, 43, 17);
					c.mouse_move(0, 44, 17);
					c.mouse_move(0, 43, 16);

					// ASSERT
					mocks::mouse_event reference3[] = { mocks::me_enter(), mocks::me_move(0, 30, 0), mocks::me_leave(), };

					assert_equal(reference3, v->events_log);

					// INIT
					v->events_log.clear();

					// ACT
					c.mouse_move(0, 43, 39);
					c.mouse_move(0, 44, 39);
					c.mouse_move(0, 43, 40);

					// ASSERT
					mocks::mouse_event reference4[] = { mocks::me_enter(), mocks::me_move(0, 30, 22), mocks::me_leave(), };

					assert_equal(reference4, v->events_log);
				}


				test( MouseLeaveIsGeneratedWhenMouseLeavesContainer )
				{
					// INIT
					container c;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr< mocks::logging_mouse_input<view> > v1(new mocks::logging_mouse_input<view>());
					shared_ptr< mocks::logging_mouse_input<view> > v2(new mocks::logging_mouse_input<view>());

					lm->positions.push_back(make_position(13, 17, 31, 23));
					lm->positions.push_back(make_position(91, 45, 100, 100));
					c.set_layout(lm);
					c.add_view(v1);
					c.add_view(v2);
					c.resize(1000, 1000, nviews);

					c.mouse_move(0, 14, 19);

					// ACT
					c.mouse_leave();

					// ASSERT
					mocks::mouse_event reference1[] = { mocks::me_enter(), mocks::me_move(0, 1, 2), mocks::me_leave(), };

					assert_equal(reference1, v1->events_log);
					assert_is_empty(v2->events_log);

					// INIT
					v1->events_log.clear();
					c.mouse_move(0, 91, 45);

					// ACT
					c.mouse_leave();

					// ASSERT
					mocks::mouse_event reference2[] = { mocks::me_enter(), mocks::me_move(0, 0, 0), mocks::me_leave(), };

					assert_is_empty(v1->events_log);
					assert_equal(reference2, v2->events_log);
				}


				test( NativeViewPlacementsAreShiftedAccordinglyToViewLocation )
				{
					// INIT
					shared_ptr<mocks::logging_layout_manager> lm[] = {
						shared_ptr<mocks::logging_layout_manager>(new mocks::logging_layout_manager),
						shared_ptr<mocks::logging_layout_manager>(new mocks::logging_layout_manager),
					};
					shared_ptr<container> c[] = {
						shared_ptr<container>(new container), shared_ptr<container>(new container),
					};
					shared_ptr<mocks::visual_with_native_view> v[] = {
						shared_ptr<mocks::visual_with_native_view>(new mocks::visual_with_native_view()),
						shared_ptr<mocks::visual_with_native_view>(new mocks::visual_with_native_view()),
						shared_ptr<mocks::visual_with_native_view>(new mocks::visual_with_native_view()),
					};

					lm[0]->positions.push_back(make_position(13, 17, 31, 23));
					lm[0]->positions.push_back(make_position(91, 45, 100, 100));
					lm[1]->positions.push_back(make_position(130, 171, 31, 23));
					lm[1]->positions.push_back(make_position(910, 145, 100, 100));

					c[0]->set_layout(lm[0]);
					c[1]->set_layout(lm[1]);

					c[1]->add_view(v[0]);
					c[1]->add_view(v[1]);
					c[0]->add_view(v[2]);
					c[0]->add_view(c[1]);

					// ACT
					c[1]->resize(10, 100, nviews);

					// ASSERT
					assert_equal(2u, nviews.size());
					assert_equal(make_position(130, 171, 31, 23), nviews[0].location);
					assert_equal(make_position(910, 145, 100, 100), nviews[1].location);

					// ACT
					c[0]->resize(10, 100, nviews);

					// ASSERT
					assert_equal(5u, nviews.size());
					assert_equal(make_position(13, 17, 31, 23), nviews[2].location);
					assert_equal(make_position(130 + 91, 171 + 45, 31, 23), nviews[3].location);
					assert_equal(make_position(910 + 91, 145 + 45, 100, 100), nviews[4].location);
				}


				test( MouseEventsAreDispatchedInReversedZOrder )
				{
					// INIT
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr<container> c(new container);
					shared_ptr< mocks::logging_mouse_input<view> > v1(new mocks::logging_mouse_input<view>());
					shared_ptr< mocks::logging_mouse_input<view> > v2(new mocks::logging_mouse_input<view>());
					shared_ptr< mocks::logging_mouse_input<view> > v3(new mocks::logging_mouse_input<view>());

					c->set_layout(lm);
					lm->positions.push_back(make_position(0, 0, 70, 50));
					c->add_view(v1);
					lm->positions.push_back(make_position(40, 30, 60, 40));
					c->add_view(v2);
					lm->positions.push_back(make_position(30, 35, 35, 20));
					c->add_view(v3);
					c->resize(200, 150, nviews);

					// ACT
					c->mouse_move(0, 43, 37);

					// ASSERT
					mocks::mouse_event reference1[] = {
						mocks::me_enter(),
						mocks::me_move(0, 13, 2),
					};

					assert_is_empty(v1->events_log);
					assert_is_empty(v2->events_log);
					assert_equal(reference1, v3->events_log);

					// ACT
					c->mouse_move(mouse_input::left, 69, 49);

					// ASSERT
					mocks::mouse_event reference2[] = {
						mocks::me_enter(),
						mocks::me_move(0, 13, 2),
						mocks::me_leave(),
					};
					mocks::mouse_event reference3[] = {
						mocks::me_enter(),
						mocks::me_move(mouse_input::left, 29, 19),
					};

					assert_is_empty(v1->events_log);
					assert_equal(reference3, v2->events_log);
					assert_equal(reference2, v3->events_log);
				}


				test( MouseCaptureFromAChildIsPropogatedUpstream )
				{
					// INIT
					mocks::capture_provider cp;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr<container> c(new container);
					shared_ptr< mocks::logging_mouse_input<view> > v1(new mocks::logging_mouse_input<view>());
					shared_ptr< mocks::logging_mouse_input<view> > v2(new mocks::logging_mouse_input<view>());
					shared_ptr<void> capture_handles[2];

					cp.add_view(*c);
					c->set_layout(lm);

					lm->positions.push_back(make_position(0, 0, 100, 55));
					c->add_view(v1);
					lm->positions.push_back(make_position(70, 30, 80, 70));
					c->add_view(v2);

					// ACT
					v1->capture(capture_handles[0]);

					// ASSERT
					assert_not_null(capture_handles[0]);
					assert_not_null(cp.log2[c.get()].lock());
					assert_not_equal(cp.log2[c.get()].lock(), capture_handles[0]);

					// ACT
					capture_handles[0].reset();

					// ASSERT
					pair<view *, bool> reference2[] = { make_pair(c.get(), true), make_pair(c.get(), false), };

					assert_equal(reference2, cp.log);
					assert_is_empty(cp.log2);

					// ACT
					v2->capture(capture_handles[1]);

					// ASSERT
					pair<view *, bool> reference3[] = {
						make_pair(c.get(), true), make_pair(c.get(), false), make_pair(c.get(), true),
					};

					assert_equal(reference3, cp.log);
					assert_not_null(capture_handles[1]);
					assert_not_null(cp.log2[c.get()].lock());
					assert_not_equal(cp.log2[c.get()].lock(), capture_handles[1]);
				}


				test( MouseEventsArePassedToCapturingViewRegardlessOfLocationAndZorder )
				{
					// INIT
					mocks::capture_provider cp;
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr<container> c(new container);
					shared_ptr< mocks::logging_mouse_input<view> > v1(new mocks::logging_mouse_input<view>());
					shared_ptr< mocks::logging_mouse_input<view> > v2(new mocks::logging_mouse_input<view>());
					shared_ptr< mocks::logging_mouse_input<view> > v3(new mocks::logging_mouse_input<view>());
					shared_ptr<void> capture_handle;

					cp.add_view(*c);
					c->set_layout(lm);

					lm->positions.push_back(make_position(0, 0, 100, 55));
					c->add_view(v1);
					lm->positions.push_back(make_position(70, 30, 80, 70));
					c->add_view(v2);
					lm->positions.push_back(make_position(0, 0, 150, 100));
					c->add_view(v3);
					c->resize(1000, 1000, nviews);

					// ACT (point of all-intersection)
					v1->capture(capture_handle);
					c->mouse_down(mouse_input::left, mouse_input::right, 90, 35);

					// ASSERT
					mocks::mouse_event reference1[] = {
						mocks::me_enter(),
						mocks::me_down(mouse_input::left, mouse_input::right, 90, 35),
					};

					assert_equal(reference1, v1->events_log);
					assert_is_empty(v2->events_log);
					assert_is_empty(v3->events_log);

					// ACT
					c->mouse_move(mouse_input::left | mouse_input::right, 110, 60);

					// ASSERT
					mocks::mouse_event reference2[] = {
						mocks::me_enter(),
						mocks::me_down(mouse_input::left, mouse_input::right, 90, 35),
						mocks::me_move(mouse_input::left | mouse_input::right, 110, 60),
					};

					assert_equal(reference2, v1->events_log);
					assert_is_empty(v2->events_log);
					assert_is_empty(v3->events_log);

					// ACT
					c->mouse_move(mouse_input::right, 10, -60);

					// ASSERT
					mocks::mouse_event reference3[] = {
						mocks::me_enter(),
						mocks::me_down(mouse_input::left, mouse_input::right, 90, 35),
						mocks::me_move(mouse_input::left | mouse_input::right, 110, 60),
						mocks::me_move(mouse_input::right, 10, -60),
					};

					assert_equal(reference3, v1->events_log);
					assert_is_empty(v2->events_log);
					assert_is_empty(v3->events_log);

					// ACT
					capture_handle.reset();

					// ASSERT
					assert_equal(reference3, v1->events_log);
					assert_is_empty(v2->events_log);
					assert_is_empty(v3->events_log);

					// ACT
					c->mouse_move(mouse_input::right, 9, -10);

					// ASSERT
					mocks::mouse_event reference4[] = {
						mocks::me_enter(),
						mocks::me_down(mouse_input::left, mouse_input::right, 90, 35),
						mocks::me_move(mouse_input::left | mouse_input::right, 110, 60),
						mocks::me_move(mouse_input::right, 10, -60),
						mocks::me_leave(),
					};

					assert_equal(reference4, v1->events_log);
					assert_is_empty(v2->events_log);
					assert_is_empty(v3->events_log);

					// ACT
					v2->capture(capture_handle);
					c->mouse_move(mouse_input::middle, 9, -10);

					// ASSERT
					mocks::mouse_event reference5[] = {
						mocks::me_enter(),
						mocks::me_move(mouse_input::middle, -61, -40),
					};

					assert_equal(reference4, v1->events_log);
					assert_equal(reference5, v2->events_log);
					assert_is_empty(v3->events_log);
				}

			end_test_suite
		}
	}
}
