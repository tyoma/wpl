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


			test( ChildViewsAreDrawnWithCorrespondentOffset )
			{
				// INIT
				visual_router vr(views, vrhost);
				gcontext::surface_type surface(150, 100, 0);
				gcontext ctx(surface, *renderer, *text_engine, make_vector(103, 71));
				const auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv[] = {
					{	 v, nullptr_nv, {	 13, 17, 1000, 1000	 }		},
					{	 v, nullptr_nv, {	 90, 40, 1000, 1000	 }		},
				};

				v->transcending = true;
				views.assign(begin(pv), end(pv));

				// ACT
				vr.draw(ctx, rasterizer);

				// ASSERT
				agge::rect_i reference1[] = {	{	90, 54, 240, 154	}, {	13, 31, 163, 131	},	};

				assert_equal(reference1, v->update_area_log);

				// INIT
				placed_view pv2 = { v, nullptr_nv, { -103, -71, 1000, 1000 }	};

				views.push_back(pv2);
				v->update_area_log.clear();

				// ACT
				vr.draw(ctx, rasterizer);

				// ASSERT
				agge::rect_i reference2[] = {	{	90, 54, 240, 154	}, {	13, 31, 163, 131	}, { 206, 142, 356, 242	}, };

				assert_equal(reference2, v->update_area_log);
			}


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


			test( ViewsAreRenderedAsExpected )
			{
				// INIT
				visual_router vr(views, vrhost);
				agge::color color_o = agge::color::make(0, 0, 0);
				gcontext::pixel_type o = make_pixel_real(color_o);
				agge::color color_a = agge::color::make(255, 0, 0);
				gcontext::pixel_type a = make_pixel_real(color_a);
				agge::color color_b = agge::color::make(0, 255, 0);
				gcontext::pixel_type b = make_pixel_real(color_b);
				agge::color color_c = agge::color::make(0, 0, 255);
				gcontext::pixel_type c = make_pixel_real(color_c);
				shared_ptr<view> v[] = {
					make_shared< mocks::filling_visual<view> >(color_a),
					make_shared< mocks::filling_visual<view> >(color_b),
					make_shared< mocks::filling_visual<view> >(color_c),
				};
				placed_view pv[] = {
					{	v[0], nullptr_nv, {	3, 1, 9, 6	},	},
					{	v[1], nullptr_nv, {	1, 2, 4, 5	},	},
					{	v[2], nullptr_nv, {	5, 4, 10, 6	},	},
				};
				gcontext::surface_type surface(10, 7, 0);
				gcontext::renderer_type ren(1);
				gcontext::rasterizer_ptr ras(new gcontext::rasterizer_type);
				gcontext ctx(surface, ren, *text_engine, agge::zero());

				views = mkvector(pv);

				reset(surface, o);

				// ACT
				vr.draw(ctx, ras);

				// ASSERT
				const gcontext::pixel_type reference[] = {
					o, o, o, o, o, o, o, o, o, o,
					o, o, o, a, a, a, a, a, a, o,
					o, b, b, b, a, a, a, a, a, o,
					o, b, b, b, a, a, a, a, a, o,
					o, b, b, b, a, c, c, c, c, c,
					o, o, o, a, a, c, c, c, c, c,
					o, o, o, o, o, o, o, o, o, o,
				};

				assert_equal(reference, surface);
			}


			test( ChildInvalidationIsBroadcastForChildViewRect )
			{
				// INIT
				visual_router vr(views, vrhost);
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv[] = {
					{ v[0], nullptr_nv, { 13, 17, 44, 40 },	},
					{ v[1], nullptr_nv, { 91, 45, 1091, 1045 },	},
				};
				vector<agge::rect_i> invalidation_log;

				views.assign(begin(pv), end(pv));
				vrhost.on_invalidate = [&] (const agge::rect_i &area) {	invalidation_log.push_back(area);	};

				vr.reload_views();

				// ACT
				v[0]->invalidate(nullptr);

				// ASSERT
				assert_equal(1u, invalidation_log.size());
				assert_equal(create_rect(13, 17, 44, 40), invalidation_log[0]);

				// ACT
				v[1]->invalidate(nullptr);

				// ASSERT
				assert_equal(2u, invalidation_log.size());
				assert_equal(create_rect(91, 45, 1091, 1045), invalidation_log[1]);
			}


			test( ChildInvalidationTranslatesInvalidArea )
			{
				// INIT
				visual_router vr(views, vrhost);
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv[] = {
					{ v[0], nullptr_nv, { 13, 17, 44, 40 },	},
					{ v[1], nullptr_nv, { 91, 45, 1091, 1045 },	},
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
				assert_equal(create_rect(23, 28, 33, 39), invalidation_log.back());

				// ACT
				v[1]->invalidate(&a1);

				// ASSERT
				assert_equal(2u, invalidation_log.size());
				assert_equal(create_rect(101, 56, 111, 67), invalidation_log.back());

				// ACT
				v[0]->invalidate(&a2);

				// ASSERT
				assert_equal(3u, invalidation_log.size());
				assert_equal(create_rect(3, 16, 30, 37), invalidation_log.back());
			}


			test( NotificationsFromRemovedViewsAreIgnored )
			{
				// INIT
				visual_router vr(views, vrhost);
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv1[] = {	{ v[0], nullptr_nv, { 1, 2, 5, 10 },	},	};
				placed_view pv2[] = {	{ v[1], nullptr_nv, { 10, 10, 100, 100 },	},	};
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
				assert_equal(create_rect(10, 10, 100, 100), invalidation_log.back());
			}


			test( NativeViewsArePermittedOnReload )
			{
				// INIT
				visual_router vr(views, vrhost);
				auto v = make_shared<view>();
				placed_view pv[] = {
					{ nullptr, nullptr_nv /* we don't actually use native view */, { 0, 0, 10, 10 },	},
					{ v, nullptr_nv, { 0, 0, 100, 55 },	},
				};
				vector<agge::rect_i> invalidation_log;

				views.assign(begin(pv), end(pv));
				vrhost.on_invalidate = [&] (const agge::rect_i &area) {	invalidation_log.push_back(area);	};

				// ACT
				vr.reload_views();
				v->invalidate(nullptr);

				// ASSERT
				assert_equal(create_rect(0, 0, 100, 55), invalidation_log.back());
			}


			test( NoInvalidationIsDoneIfViewsSizeIsMismatched )
			{
				// INIT
				visual_router vr(views, vrhost);
				shared_ptr<view> v[] = {	make_shared<view>(), make_shared<view>(),	};
				placed_view pv1[] = {
					{ v[0], nullptr_nv, { 0, 0, 100, 55 },	},
					{ v[1], nullptr_nv, { 0, 0, 100, 55 },	},
				};
				placed_view pv2[] = {
					{ v[1], nullptr_nv, { 0, 0, 100, 55 },	},
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


			test( NonTranscendingViewsAreDrawnOnlyIfIntersectUpdateRectangle )
			{
				// INIT
				visual_router vr(views, vrhost);
				gcontext::surface_type surface(3, 3, 0);
				shared_ptr< mocks::logging_visual<view> > v[] = {
					make_shared< mocks::logging_visual<view> >(), make_shared< mocks::logging_visual<view> >(),
					make_shared< mocks::logging_visual<view> >(), make_shared< mocks::logging_visual<view> >(),
				};
				placed_view pv[] = {
					{	v[0], nullptr_nv, {	1, 1, 4, 4	},	},
					{	v[1], nullptr_nv, {	6, 1, 9, 4	},	},
					{	v[2], nullptr_nv, {	1, 6, 4, 9	},	},
					{	v[3], nullptr_nv, {	6, 6, 9, 9	},	},
				};

				views = mkvector(pv);

				// ACT
				gcontext ctx1(surface, *renderer, *text_engine, make_vector(0, 0));
				vr.draw(ctx1, rasterizer);

				// ASSERT
				assert_equal(1u, v[0]->update_area_log.size());
				assert_is_empty(v[1]->update_area_log);
				assert_is_empty(v[2]->update_area_log);
				assert_is_empty(v[3]->update_area_log);

				// INIT
				v[0]->update_area_log.clear();

				// ACT
				gcontext ctx2(surface, *renderer, *text_engine, make_vector(7, 1));
				vr.draw(ctx2, rasterizer);

				// ASSERT
				assert_is_empty(v[0]->update_area_log);
				assert_equal(1u, v[1]->update_area_log.size());
				assert_is_empty(v[2]->update_area_log);
				assert_is_empty(v[3]->update_area_log);

				// INIT
				v[1]->update_area_log.clear();

				// ACT
				gcontext ctx3(surface, *renderer, *text_engine, make_vector(1, 4));
				vr.draw(ctx3, rasterizer);

				// ASSERT
				assert_is_empty(v[0]->update_area_log);
				assert_is_empty(v[1]->update_area_log);
				assert_equal(1u, v[2]->update_area_log.size());
				assert_is_empty(v[3]->update_area_log);

				// INIT
				v[2]->update_area_log.clear();

				// ACT
				gcontext ctx4(surface, *renderer, *text_engine, make_vector(6, 6));
				vr.draw(ctx4, rasterizer);

				// ASSERT
				assert_is_empty(v[0]->update_area_log);
				assert_is_empty(v[1]->update_area_log);
				assert_is_empty(v[2]->update_area_log);
				assert_equal(1u, v[3]->update_area_log.size());
			}


			test( TranscendingViewsAreDrawnEvenIfNoIntersectionWithUpdateRectangle )
			{
				// INIT
				visual_router vr(views, vrhost);
				gcontext::surface_type surface(3, 3, 0);
				auto v = make_shared< mocks::logging_visual<view> >();
				placed_view pv[] = {	{	v, nullptr_nv, {	1, 1, 4, 4	},	},	};

				views = mkvector(pv);
				v->transcending = true;

				// ACT
				gcontext ctx1(surface, *renderer, *text_engine, make_vector(4, 1));
				vr.draw(ctx1, rasterizer);

				// ASSERT
				assert_equal(1u, v->update_area_log.size());

				// ACT
				gcontext ctx2(surface, *renderer, *text_engine, make_vector(1, 4));
				vr.draw(ctx2, rasterizer);

				// ASSERT
				assert_equal(2u, v->update_area_log.size());
			}

		end_test_suite
	}
}
