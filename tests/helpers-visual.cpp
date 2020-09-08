#include "helpers-visual.h"

#include <agge/filling_rules.h>
#include <agge/pixel.h>
#include <string.h>

using namespace agge;

namespace wpl
{
	namespace tests
	{
		context::context()
			: surface(200, 150, 0), rasterizer(new gcontext::rasterizer_type), renderer(new gcontext::renderer_type(1))
		{	}

		context::~context()
		{	}

		void context::resize(count_t width, count_t height)
		{	surface.resize(width, height);	}

		void context::reset(color c)
		{
			fill(surface, make_rect(0, 0, static_cast<int>(surface.width()), static_cast<int>(surface.height())),
				blender_t(c));
		}


		view_location make_position(int x, int y, int width, int height)
		{
			view_location p = { x, y, width, height };
			return p;
		}

		gcontext::pixel_type make_pixel(const color &color_)
		{
			gcontext::pixel_type p;

			p.components[context::pixel_color_model::R] = color_.r;
			p.components[context::pixel_color_model::G] = color_.g;
			p.components[context::pixel_color_model::B] = color_.b;
			p.components[context::pixel_color_model::A] = color_.a;
			return p;
		}

		gcontext::pixel_type make_pixel_real(const color& color)
		{
			context::blender_t::cover_type cover = static_cast<context::blender_t::cover_type>(-1);
			context::blender_t b(color);
			gcontext::pixel_type p = {};

			b(&p, 0, 0, 1u, &cover);
			return p;
		}

		void reset(gcontext::surface_type &surface, gcontext::pixel_type ref_pixel)
		{
			for (count_t y = 0; y < surface.height(); ++y)
			{
				auto p = surface.row_ptr(y);

				for (count_t x_ = 0; x_ < surface.width(); ++x_)
					p[x_] = ref_pixel;
			}
		}

		void rectangle(gcontext &ctx, color c, int x1, int y1, int x2, int y2)
		{
			gcontext::rasterizer_ptr ras(new gcontext::rasterizer_type);

			ras->move_to(static_cast<real_t>(x1), static_cast<real_t>(y1));
			ras->line_to(static_cast<real_t>(x2), static_cast<real_t>(y1));
			ras->line_to(static_cast<real_t>(x2), static_cast<real_t>(y2));
			ras->line_to(static_cast<real_t>(x1), static_cast<real_t>(y2));
			ras->line_to(static_cast<real_t>(x1), static_cast<real_t>(y1));
			ctx(ras, context::blender_t(c), winding<>());
		}
	}

	bool operator ==(const view_location &lhs, const view_location &rhs)
	{	return lhs.left == rhs.left && lhs.top == rhs.top && lhs.width == rhs.width && lhs.height == rhs.height;	}
}

namespace agge
{
	bool operator ==(const wpl::gcontext::pixel_type &lhs, const wpl::gcontext::pixel_type &rhs)
	{	return !memcmp(&lhs, &rhs, sizeof(wpl::gcontext::pixel_type));	}

	bool operator ==(const color &lhs, const color &rhs)
	{	return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;	}
}
