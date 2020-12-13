#include <wpl/visual.h>

#include "Mockups.h"
#include "helpers-visual.h"

#include <agge/blenders_generic.h>
#include <agge/filling_rules.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace tests
	{
		begin_test_suite( VisualTests )
			typedef default_blender_t blender_t;

			mocks::font_loader fake_loader;
			shared_ptr<gcontext::renderer_type> renderer;
			shared_ptr<gcontext::text_engine_type> text_engine;
			color color_x, color_o;
			gcontext::pixel_type x, o;

			init( Init )
			{
				renderer.reset(new gcontext::renderer_type(1));
				text_engine.reset(new gcontext::text_engine_type(fake_loader, 0));
				x = make_pixel_real(color_x = color::make(255, 255, 255));
				o = make_pixel_real(color_o = color::make(0, 0, 0));
			}

			test( VisualIsNotTranscendingByDefault )
			{
				// INIT
				visual v;

				// ACT / ASSERT
				assert_is_false(v.transcending);
			}


			test( ImageIsDrawnAtCoordinatesUnchangedNoOffsetNoWindow )
			{
				// INIT
				gcontext::surface_type s(8, 8, 0);
				gcontext ctx(s, *renderer, *text_engine, agge::zero());

				// ACT
				rectangle(ctx, color_x, 0, 0, 3, 3);
				rectangle(ctx, color_x, 6, 4, 8, 6);

				// ASSERT
				const gcontext::pixel_type reference1[] = {
					x, x, x, o, o, o, o, o,
					x, x, x, o, o, o, o, o,
					x, x, x, o, o, o, o, o,
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, o, x, x,
					o, o, o, o, o, o, x, x,
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, o, o, o,
				};

				assert_equal(reference1, s);

				// INIT
				reset(s, o);

				// ACT
				rectangle(ctx, color_x, 3, 4, 7, 7);

				// ASSERT
				const gcontext::pixel_type reference2[] = {
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, o, o, o,
					o, o, o, x, x, x, x, o,
					o, o, o, x, x, x, x, o,
					o, o, o, x, x, x, x, o,
					o, o, o, o, o, o, o, o,
				};

				assert_equal(reference2, s);
			}


			test( ImageIsDrawnAtOffsetCoordinatesForTranslatedContext )
			{
				// INIT
				gcontext::surface_type s(5, 5, 0);
				gcontext ctx(s, *renderer, *text_engine, agge::zero());

				// ACT
				gcontext ctx2 = ctx.translate(2, 1);
				rectangle(ctx2, color_x, 0, 0, 3, 3);

				// ASSERT
				const gcontext::pixel_type reference1[] = {
					o, o, o, o, o,
					o, o, x, x, x,
					o, o, x, x, x,
					o, o, x, x, x,
					o, o, o, o, o,
				};

				assert_equal(reference1, s);
				assert_equal(make_rect(-2, -1, 3, 4), ctx2.update_area());

				// INIT
				reset(s, o);

				// ACT
				gcontext ctx3 = ctx2.translate(-4, -1);
				rectangle(ctx3, color_x, 0, 0, 3, 3);

				// ASSERT
				const gcontext::pixel_type reference2[] = {
					x, o, o, o, o,
					x, o, o, o, o,
					x, o, o, o, o,
					o, o, o, o, o,
					o, o, o, o, o,
				};

				assert_equal(reference2, s);
				assert_equal(make_rect(2, 0, 7, 5), ctx3.update_area());
			}


			test( UpdateAreaIsSetToUnderlyingSurfaceBounds )
			{
				// INIT
				gcontext::surface_type s1(15, 153, 0);

				// ACT / ASSERT
				assert_equal(make_rect(0, 0, 15, 153), gcontext(s1, *renderer, *text_engine, agge::zero()).update_area());

				// INIT
				gcontext::surface_type s2(191, 321, 0);

				// ACT / ASSERT
				assert_equal(make_rect(0, 0, 191, 321), gcontext(s2, *renderer, *text_engine, agge::zero()).update_area());
			}


			test( UpdateAreaIsTranslatedIfOffsetIsSpecified )
			{
				// INIT
				gcontext::surface_type s(200, 150, 0);

				// ACT / ASSERT
				assert_equal(make_rect(12, -31, 212, 119), gcontext(s, *renderer, *text_engine, make_vector(-12, 31)).update_area());
				assert_equal(make_rect(-5, 10, 195, 160), gcontext(s, *renderer, *text_engine, make_vector(5, -10)).update_area());
			}


			test( WindowBecomesUpdateAreaIfProvided )
			{
				// INIT
				gcontext::surface_type s(200, 150, 0);

				// ACT / ASSERT
				rect_i w1 = { 10, 13, 50, 40 };
				assert_equal(w1, gcontext(s, *renderer, *text_engine, agge::zero(), w1).update_area());
				rect_i w2 = { 100, 3, 150, 50 };
				assert_equal(w2, gcontext(s, *renderer, *text_engine, agge::zero(), w2).update_area());
				rect_i w3 = { 10, 13, 50, 40 };
				assert_equal(w1, gcontext(s, *renderer, *text_engine, make_vector(10, 13), w3).update_area());
				rect_i w4 = { 100, 3, 150, 50 };
				assert_equal(w2, gcontext(s, *renderer, *text_engine, make_vector(-1, -2), w4).update_area());
			}


			test( WindowIsInheritedByTranslatedContext )
			{
				// INIT
				gcontext::surface_type s(200, 200, 0);
				rect_i w1 = { 10, 13, 50, 40 };
				rect_i w2 = { 20, 23, 51, 41 };

				// ACT / ASSERT
				assert_equal(make_rect(9, 10, 49, 37),
					gcontext(s, *renderer, *text_engine, agge::zero(), w1).translate(1, 3).update_area());
				assert_equal(make_rect(30, 30, 61, 48),
					gcontext(s, *renderer, *text_engine, agge::zero(), w2).translate(-10, -7).update_area());
			}


			test( UpdateAreaInWindowedContextEqualsToWindow )
			{
				// INIT
				gcontext::surface_type s(200, 200, 0);
				rect_i w1 = { 10, 13, 50, 40 };
				rect_i w2 = { 20, 23, 51, 41 };

				// ACT / ASSERT
				assert_equal(make_rect(11, 17, 40, 37),
					gcontext(s, *renderer, *text_engine, agge::zero(), w1).window(11, 17, 40, 37).update_area());
				assert_equal(make_rect(23, 27, 50, 40),
					gcontext(s, *renderer, *text_engine, agge::zero(), w2).window(23, 27, 50, 40).update_area());
			}


			test(WindowedRenderDoesNotAffectPixelsOutsideWindow)
			{
				// INIT
				gcontext::surface_type s(8, 8, 0);
				gcontext ctx(s, *renderer, *text_engine, agge::zero());

				// ACT
				gcontext ctx2 = ctx.window(1, 2, 7, 8);
				rectangle(ctx2, color_x, 0, 0, 3, 3);
				rectangle(ctx2, color_x, 6, 4, 8, 6);

				// ASSERT
				const gcontext::pixel_type reference1[] = {
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, o, o, o,
					o, x, x, o, o, o, o, o,
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, o, x, o,
					o, o, o, o, o, o, x, o,
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, o, o, o,
				};

				assert_equal(reference1, s);

				// INIT
				reset(s, o);

				// ACT
				gcontext ctx3 = ctx.translate(1, -1).window(1, 0, 6, 5);
				rectangle(ctx3, color_x, 0, 0, 3, 3);
				rectangle(ctx3, color_x, 4, 4, 8, 6);

				// ASSERT
				const gcontext::pixel_type reference2[] = {
					o, o, x, x, o, o, o, o,
					o, o, x, x, o, o, o, o,
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, x, x, o,
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, o, o, o,
					o, o, o, o, o, o, o, o,
				};

				assert_equal(reference2, s);
			}
		end_test_suite
	}
}
