#include <wpl/visual.h>

#include "helpers.h"

#include <agge/blenders_generic.h>
#include <agge/filling_rules.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace agge;
using namespace std;

namespace ut
{
	template <size_t n>
	inline void are_equal(const wpl::gcontext::pixel_type (&expected)[n], const wpl::gcontext::surface_type &actual,
		const LocationInfo &location)
	{
		are_equal(n, actual.width() * actual.height(), location);
		for (count_t y = 0; y < actual.height(); ++y)
		{
			auto p = actual.row_ptr(y);

			for (count_t x = 0; x < actual.width(); ++x)
				are_equal(expected[x + y * actual.width()], p[x], location);
		}
	}
}

namespace wpl
{
	namespace tests
	{
		begin_test_suite( VisualTests )
			typedef blender_solid_color_rgb<gcontext::pixel_type, order_bgra> blender_t;

			gcontext::rasterizer_ptr rasterizer;
			shared_ptr<gcontext::renderer_type> renderer;
			gcontext::pixel_type x, o;

			init( Init )
			{
				const blender_t::cover_type c = 255;

				rasterizer.reset(new gcontext::rasterizer_type);
				renderer.reset(new gcontext::renderer_type(1));
				o = zero();
				blender_t(color::make(255, 255, 255))(&x, 0, 0, 1, &c);
			}

			void reset(gcontext::surface_type &surface) const
			{
				for (count_t y = 0; y < surface.height(); ++y)
				{
					auto p = surface.row_ptr(y);

					for (count_t x_ = 0; x_ < surface.width(); ++x_)
						p[x_] = o;
				}
			}

			void rectangle(gcontext &ctx, int x1, int y1, int x2, int y2)
			{
				rasterizer->move_to(static_cast<real_t>(x1), static_cast<real_t>(y1));
				rasterizer->line_to(static_cast<real_t>(x2), static_cast<real_t>(y1));
				rasterizer->line_to(static_cast<real_t>(x2), static_cast<real_t>(y2));
				rasterizer->line_to(static_cast<real_t>(x1), static_cast<real_t>(y2));
				rasterizer->line_to(static_cast<real_t>(x1), static_cast<real_t>(y1));
				ctx(rasterizer, blender_t(color::make(255, 255, 255)), winding<>());
			}


			test( ImageIsDrawnAtCoordinatesUnchangedNoOffsetNoWindow )
			{
				// INIT
				gcontext::surface_type s(8, 8, 0);
				gcontext ctx(s, *renderer, agge::zero());

				// ACT
				rectangle(ctx, 0, 0, 3, 3);
				rectangle(ctx, 6, 4, 8, 6);

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
				reset(s);

				// ACT
				rectangle(ctx, 3, 4, 7, 7);

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
				gcontext ctx(s, *renderer, agge::zero());

				// ACT
				gcontext ctx2 = ctx.translate(2, 1);
				rectangle(ctx2, 0, 0, 3, 3);

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
				reset(s);

				// ACT
				gcontext ctx3 = ctx2.translate(-4, -1);
				rectangle(ctx3, 0, 0, 3, 3);

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
				assert_equal(make_rect(0, 0, 15, 153), gcontext(s1, *renderer, agge::zero()).update_area());

				// INIT
				gcontext::surface_type s2(191, 321, 0);

				// ACT / ASSERT
				assert_equal(make_rect(0, 0, 191, 321), gcontext(s2, *renderer, agge::zero()).update_area());
			}


			test( UpdateAreaIsTranslatedIfOffsetIsSpecified )
			{
				// INIT
				gcontext::surface_type s(200, 150, 0);

				// ACT / ASSERT
				assert_equal(make_rect(12, -31, 212, 119), gcontext(s, *renderer, make_vector(-12, 31)).update_area());
				assert_equal(make_rect(-5, 10, 195, 160), gcontext(s, *renderer, make_vector(5, -10)).update_area());
			}


			test( WindowBecomesUpdateAreaIfProvided )
			{
				// INIT
				gcontext::surface_type s(200, 150, 0);

				// ACT / ASSERT
				rect_i w1 = { 10, 13, 50, 40 };
				assert_equal(w1, gcontext(s, *renderer, agge::zero(), &w1).update_area());
				rect_i w2 = { 100, 3, 150, 50 };
				assert_equal(w2, gcontext(s, *renderer, agge::zero(), &w2).update_area());
				rect_i w3 = { 10, 13, 50, 40 };
				assert_equal(w1, gcontext(s, *renderer, make_vector(10, 13), &w3).update_area());
				rect_i w4 = { 100, 3, 150, 50 };
				assert_equal(w2, gcontext(s, *renderer, make_vector(-1, -2), &w4).update_area());
			}


			test( WindowIsInheritedByTranslatedContext )
			{
				// INIT
				gcontext::surface_type s(200, 200, 0);
				rect_i w1 = { 10, 13, 50, 40 };
				rect_i w2 = { 20, 23, 51, 41 };

				// ACT / ASSERT
				assert_equal(make_rect(9, 10, 49, 37),
					gcontext(s, *renderer, agge::zero(), &w1).translate(1, 3).update_area());
				assert_equal(make_rect(30, 30, 61, 48),
					gcontext(s, *renderer, agge::zero(), &w2).translate(-10, -7).update_area());
			}
		end_test_suite
	}
}
