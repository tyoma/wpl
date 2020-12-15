#include <wpl/visual_router.h>

#include <tests/common/helpers-visual.h>
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
			const auto nullptr_nv = shared_ptr<native_view>();
		}

		begin_test_suite( VisualRouterTests )
			vector<placed_view> views;
			mocks::visual_router_host vrhost;
			unique_ptr<gcontext::renderer_type> renderer;
			shared_ptr<gcontext::text_engine_type> text_engine;
			gcontext::rasterizer_ptr rasterizer;

			init( Init )
			{
				renderer.reset(new gcontext::renderer_type(1));
				text_engine = create_faked_text_engine();
				rasterizer.reset(new gcontext::rasterizer_type);
			}


			//test( ForceLayoutRequestIsMadeOnSettingLayout )
			//{
			//	// INIT
			//	auto force_layouts = 0;
			//	container c;
			//	shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
			//	const auto conn = c.force_layout += [&] { force_layouts++; };

			//	// ACT
			//	c.set_layout(lm);

			//	// ASSERT
			//	assert_equal(1, force_layouts);

			//	// ACT
			//	c.set_layout(lm);
			//	c.set_layout(lm);

			//	// ASSERT
			//	assert_equal(3, force_layouts);
			//}


			//test( AddingAViewForcesLayoutRecalculate )
			//{
			//	// INIT
			//	container c;
			//	shared_ptr<view> v1(new view);
			//	shared_ptr<view> v2(new view);
			//	shared_ptr<layout_manager> lm(new mocks::logging_layout_manager);
			//	int layout_forced = 0;
			//	slot_connection conn = c.force_layout += bind(&increment, &layout_forced);

			//	// ACT
			//	c.add_view(v1);

			//	// ASSERT
			//	assert_equal(1, layout_forced);

			//	// ACT
			//	c.add_view(v2);

			//	// ASSERT
			//	assert_equal(2, layout_forced);
			//}


			//test( ForceLyaoutIsPropagatedUpstream )
			//{
			//	// INIT
			//	container c;
			//	shared_ptr<view> v1(new view);
			//	shared_ptr<view> v2(new view);
			//	shared_ptr<layout_manager> lm(new mocks::logging_layout_manager);
			//	int layout_forced = 0;

			//	c.add_view(v1);
			//	c.add_view(v2);

			//	slot_connection conn = c.force_layout += bind(&increment, &layout_forced);

			//	// ACT
			//	v1->force_layout();

			//	// ASSERT
			//	assert_equal(1, layout_forced);

			//	// ACT
			//	v2->force_layout();

			//	// ASSERT
			//	assert_equal(2, layout_forced);
			//}


			//test( LayoutIsMadeOnNonEmptyContainerResize )
			//{
			//	// INIT
			//	container c;
			//	shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
			//	shared_ptr< mocks::logging_visual<view> > v1(new mocks::logging_visual<view>());
			//	shared_ptr< mocks::logging_visual<view> > v2(new mocks::logging_visual<view>());

			//	c.set_layout(lm);
			//	c.add_view(v1);
			//	c.add_view(v2);
			//	lm->reposition_log.clear();

			//	// ACT
			//	c.resize(100, 117, nviews);

			//	// ASSERT
			//	assert_equal(1u, lm->reposition_log.size());
			//	assert_equal(make_pair(100u, 117u), lm->reposition_log[0]);

			//	// ACT
			//	c.resize(53, 91, nviews);

			//	// ASSERT
			//	assert_equal(2u, lm->reposition_log.size());
			//	assert_equal(make_pair(53u, 91u), lm->reposition_log[1]);
			//}


			//test( ChildrenAreResizedOnContainerResize )
			//{
			//	// INIT
			//	container c;
			//	shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
			//	shared_ptr< mocks::logging_visual<view> > v1(new mocks::logging_visual<view>());
			//	shared_ptr< mocks::logging_visual<view> > v2(new mocks::logging_visual<view>());

			//	c.set_layout(lm);
			//	c.add_view(v1);
			//	c.add_view(v2);
			//	lm->positions.push_back(make_position(13, 17, 100, 200));
			//	lm->positions.push_back(make_position(90, 40, 110, 112));
			//	v1->resize_log.clear();
			//	v2->resize_log.clear();

			//	// ACT
			//	c.resize(1000, 1000, nviews);

			//	// ASSERT
			//	assert_equal(1u, v1->resize_log.size());
			//	assert_equal(1u, v2->resize_log.size());
			//	assert_equal(make_pair(100, 200), v1->resize_log[0]);
			//	assert_equal(make_pair(110, 112), v2->resize_log[0]);

			//	// INIT
			//	lm->positions[0] = make_position(13, 17, 10, 20);
			//	lm->positions[1] = make_position(90, 40, 11, 12);

			//	// ACT
			//	c.resize(1001, 1002, nviews);

			//	// ASSERT
			//	assert_equal(2u, v1->resize_log.size());
			//	assert_equal(2u, v2->resize_log.size());
			//	assert_equal(make_pair(10, 20), v1->resize_log[1]);
			//	assert_equal(make_pair(11, 12), v2->resize_log[1]);
			//}


			test( ChildViewsAreDrawnWithCorrespondentOffset )
			{
				// INIT
				visual_router vr(views, vrhost);
				gcontext::surface_type surface(150, 100, 0);
				gcontext ctx(surface, *renderer, *text_engine, make_vector(103, 71));
				const auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv[] = {
					{ v, nullptr_nv, { 13, 17, 1000, 1000 }	},
					{ v, nullptr_nv, { 90, 40, 1000, 1000 }	},
				};

				v->transcending = true;
				views.assign(begin(pv), end(pv));

				// ACT
				vr.draw(ctx, rasterizer);

				// ASSERT
				agge::rect_i reference1[] = { { -116, -88, 34, 12 }, { -193, -111, -43, -11 }, };

				assert_equal(reference1, v->update_area_log);

				// INIT
				placed_view pv2 = { v, nullptr_nv, { -103, -71, 1000, 1000 }	};

				views.push_back(pv2);
				v->update_area_log.clear();

				// ACT
				vr.draw(ctx, rasterizer);

				// ASSERT
				agge::rect_i reference2[] = { { -116, -88, 34, 12 }, { -193, -111, -43, -11 }, { 0, 0, 150, 100 }, };

				assert_equal(reference2, v->update_area_log);
			}


			//test( MultipleChildViewsAreDrawnWithCorrespondentOffset )
			//{
			//	// INIT
			//	container c;
			//	shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
			//	shared_ptr< mocks::logging_visual<view> > v1(new mocks::logging_visual<view>());
			//	shared_ptr< mocks::logging_visual<view> > v2(new mocks::logging_visual<view>());
			//	gcontext::surface_type surface(200, 150, 0);
			//	gcontext::renderer_type ren(1);
			//	gcontext::text_engine_type text_engine(fake_loader, 0);
			//	gcontext::rasterizer_ptr ras(new gcontext::rasterizer_type);

			//	gcontext ctx(surface, ren, text_engine, make_vector(13, 17));

			//	lm->positions.push_back(make_position(13, 17, 1000, 1000));
			//	lm->positions.push_back(make_position(91, 45, 1000, 1000));

			//	// INIT / ACT
			//	c.set_layout(lm);
			//	v1->transcending = true;
			//	c.add_view(v1);
			//	v2->transcending = true;
			//	c.add_view(v2);
			//	c.resize(1000, 1000, nviews);

			//	// ACT
			//	c.draw(ctx, ras);

			//	// ASSERT
			//	agge::rect_i reference11[] = { { -26, -34, 174, 116 }, };
			//	agge::rect_i reference12[] = { { -104, -62, 96, 88 }, };

			//	assert_equal(reference11, v1->update_area_log);
			//	assert_equal(reference12, v2->update_area_log);
			//}


			test( ContextIsWindowedForNonTranscendingViews )
			{
				visual_router vr(views, vrhost);
				gcontext::surface_type surface(1000, 1000, 0);
				gcontext ctx(surface, *renderer, *text_engine, agge::zero());
				const auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv[] = {
					{ v, nullptr_nv, { 13, 17, 23, 28 }	},
					{ v, nullptr_nv, { 91, 45, 181, 65 }	},
					{ v, nullptr_nv, { 80, 60, 150, 90 }	},
				};

				v->transcending = false;
				views.assign(begin(pv), end(pv));

				// ACT
				vr.draw(ctx, rasterizer);

				// ASSERT
				agge::rect_i reference[] = {
					{ 0, 0, 10, 11 }, { 0, 0, 90, 20 }, { 0, 0, 70, 30 },
				};

				assert_equal(reference, v->update_area_log);
			}


			test( NativeViewsAreIgnoredOnDraw )
			{
				visual_router vr(views, vrhost);
				gcontext::surface_type surface(1000, 1000, 0);
				gcontext ctx(surface, *renderer, *text_engine, agge::zero());
				const auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv[] = {
					{ v, nullptr_nv, { 13, 17, 23, 28 }	},
					{ nullptr, nullptr_nv, { 91, 45, 181, 65 }	},
					{ v, nullptr_nv, { 80, 60, 150, 90 }	},
				};

				v->transcending = false;
				views.assign(begin(pv), end(pv));

				// ACT
				vr.draw(ctx, rasterizer);

				// ASSERT
				agge::rect_i reference[] = {
					{ 0, 0, 10, 11 }, { 0, 0, 70, 30 },
				};

				assert_equal(reference, v->update_area_log);
			}


			//test( ViewsAreRenderedAsExpected )
			//{
			//	// INIT
			//	container container_;
			//	agge::color color_o = agge::color::make(0, 0, 0);
			//	gcontext::pixel_type o = make_pixel_real(color_o);
			//	agge::color color_a = agge::color::make(255, 0, 0);
			//	gcontext::pixel_type a = make_pixel_real(color_a);
			//	agge::color color_b = agge::color::make(0, 255, 0);
			//	gcontext::pixel_type b = make_pixel_real(color_b);
			//	agge::color color_c = agge::color::make(0, 0, 255);
			//	gcontext::pixel_type c = make_pixel_real(color_c);
			//	shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
			//	shared_ptr< mocks::filling_visual<view> > v1(new mocks::filling_visual<view>(color_a));
			//	shared_ptr< mocks::filling_visual<view> > v2(new mocks::filling_visual<view>(color_b));
			//	shared_ptr< mocks::filling_visual<view> > v3(new mocks::filling_visual<view>(color_c));
			//	gcontext::surface_type surface(10, 7, 0);
			//	gcontext::renderer_type ren(1);
			//	gcontext::text_engine_type text_engine(fake_loader, 0);
			//	gcontext::rasterizer_ptr ras(new gcontext::rasterizer_type);
			//	gcontext ctx(surface, ren, text_engine, agge::zero());

			//	lm->positions.push_back(make_position(3, 1, 6, 5));
			//	lm->positions.push_back(make_position(1, 2, 3, 3));
			//	lm->positions.push_back(make_position(5, 4, 5, 2));

			//	container_.set_layout(lm);
			//	container_.add_view(v1);
			//	container_.add_view(v2);
			//	container_.add_view(v3);
			//	container_.resize(10, 7, nviews);

			//	reset(surface, o);

			//	// ACT
			//	container_.draw(ctx, ras);

			//	// ASSERT
			//	const gcontext::pixel_type reference[] = {
			//		o, o, o, o, o, o, o, o, o, o,
			//		o, o, o, a, a, a, a, a, a, o,
			//		o, b, b, b, a, a, a, a, a, o,
			//		o, b, b, b, a, a, a, a, a, o,
			//		o, b, b, b, a, c, c, c, c, c,
			//		o, o, o, a, a, c, c, c, c, c,
			//		o, o, o, o, o, o, o, o, o, o,
			//	};

			//	assert_equal(reference, surface);
			//}


			//vector<agge::rect_i> invalidation_log;

			//void on_invalidate(const agge::rect_i *area)
			//{
			//	assert_not_null(area);
			//	invalidation_log.push_back(*area);
			//}

			test( ChildInvalidationIsBroadcastForChildViewRect )
			{
				// INIT
				visual_router vr(views, vrhost);
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv[] = {
					{ v[0], nullptr_nv, { 13, 17, 44, 40 }	},
					{ v[1], nullptr_nv, { 91, 45, 1091, 1045 }	},
				};
				vector<agge::rect_i> invalidation_log;

				views.assign(begin(pv), end(pv));
				vrhost.on_invalidate = [&] (const agge::rect_i &area) {	invalidation_log.push_back(area);	};

				vr.reload_views();

				// ACT
				v[0]->invalidate(nullptr);

				// ASSERT
				assert_equal(1u, invalidation_log.size());
				assert_equal(make_rect(13, 17, 44, 40), invalidation_log[0]);

				// ACT
				v[1]->invalidate(nullptr);

				// ASSERT
				assert_equal(2u, invalidation_log.size());
				assert_equal(make_rect(91, 45, 1091, 1045), invalidation_log[1]);
			}


			test( ChildInvalidationTranslatesInvalidArea )
			{
				// INIT
				visual_router vr(views, vrhost);
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv[] = {
					{ v[0], nullptr_nv, { 13, 17, 44, 40 }	},
					{ v[1], nullptr_nv, { 91, 45, 1091, 1045 }	},
				};
				agge::rect_i a1 = { 10, 11, 20, 22 }, a2 = { -10, -1, 17, 20 };
				vector<agge::rect_i> invalidation_log;

				views.assign(begin(pv), end(pv));
				vrhost.on_invalidate = [&] (const agge::rect_i &area) {	invalidation_log.push_back(area);	};

				vr.reload_views();

				// ACT
				v[0]->invalidate(&a1);

				// ASSERT
				assert_equal(1u, invalidation_log.size());
				assert_equal(make_rect(23, 28, 33, 39), invalidation_log.back());

				// ACT
				v[1]->invalidate(&a1);

				// ASSERT
				assert_equal(2u, invalidation_log.size());
				assert_equal(make_rect(101, 56, 111, 67), invalidation_log.back());

				// ACT
				v[0]->invalidate(&a2);

				// ASSERT
				assert_equal(3u, invalidation_log.size());
				assert_equal(make_rect(3, 16, 30, 37), invalidation_log.back());
			}


			test( NotificationsFromRemovedViewsAreIgnored )
			{
				// INIT
				visual_router vr(views, vrhost);
				shared_ptr<view> v[] = {	make_shared<view>(),	make_shared<view>(),	};
				placed_view pv1[] = {	{ v[0], nullptr_nv, { 1, 2, 5, 10 }	},	};
				placed_view pv2[] = {	{ v[1], nullptr_nv, { 10, 10, 100, 100 }	},	};
				vector<agge::rect_i> invalidation_log;

				views.assign(begin(pv1), end(pv1));
				vrhost.on_invalidate = [&] (const agge::rect_i &area) {	invalidation_log.push_back(area);	};

				vr.reload_views();

				// ACT
				views.assign(begin(pv2), end(pv2));
				vr.reload_views();
				v[0]->invalidate(nullptr);

				// ASSERT
				assert_is_empty(invalidation_log);

				// ACT
				v[1]->invalidate(nullptr);

				// ASSERT
				assert_equal(make_rect(10, 10, 100, 100), invalidation_log.back());
			}


			test( NativeViewsArePermittedOnReload )
			{
				// INIT
				visual_router vr(views, vrhost);
				auto v = make_shared<view>();
				placed_view pv[] = {
					{ nullptr, nullptr_nv /* we don't actually use native view */, { 0, 0, 10, 10 }	},
					{ v, nullptr_nv, { 0, 0, 100, 55 }	},
				};
				vector<agge::rect_i> invalidation_log;

				views.assign(begin(pv), end(pv));
				vrhost.on_invalidate = [&] (const agge::rect_i &area) {	invalidation_log.push_back(area);	};

				// ACT
				vr.reload_views();
				v->invalidate(nullptr);

				// ASSERT
				assert_equal(make_rect(0, 0, 100, 55), invalidation_log.back());
			}


			test( NoInvalidationIsDoneIfViewsSizeIsMismatched )
			{
				// INIT
				visual_router vr(views, vrhost);
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv1[] = {
					{ v[0], nullptr_nv, { 0, 0, 100, 55 }	},
					{ v[1], nullptr_nv, { 0, 0, 100, 55 }	},
				};
				placed_view pv2[] = {
					{ v[1], nullptr_nv, { 0, 0, 100, 55 }	},
				};
				vector<agge::rect_i> invalidation_log;

				views.assign(begin(pv1), end(pv1));
				vrhost.on_invalidate = [&] (const agge::rect_i &area) {	invalidation_log.push_back(area);	};
				vr.reload_views();
				views.assign(begin(pv2), end(pv2));

				// ACT
				v[1]->invalidate(nullptr);

				// ASSERT
				assert_is_empty(invalidation_log);
			}

		end_test_suite
	}
}
