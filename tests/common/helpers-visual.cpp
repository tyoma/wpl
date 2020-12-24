#include "helpers-visual.h"

#include <agge/filling_rules.h>
#include <agge/pixel.h>
#include <agge.text/text_engine.h>
#include <string.h>

#pragma warning(disable: 4355)

using namespace agge;
using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			struct faked_text_engine_composite : gcontext::text_engine_type::loader
			{
				faked_text_engine_composite()
					: text_engine_(*this)
				{	}

				virtual font::accessor_ptr load(const wchar_t *, int, bool, bool, font::key::grid_fit) override
				{	throw 0;	}

				gcontext::text_engine_type text_engine_;
			};
		}

		gcontext::pixel_type make_pixel(const color &color_)
		{
			gcontext::pixel_type p;

			p.components[pixel_color_model::R] = color_.r;
			p.components[pixel_color_model::G] = color_.g;
			p.components[pixel_color_model::B] = color_.b;
			p.components[pixel_color_model::A] = color_.a;
			return p;
		}

		gcontext::pixel_type make_pixel_real(const color& color_)
		{
			default_blender_t::cover_type cover = static_cast<default_blender_t::cover_type>(-1);
			default_blender_t b(color_);
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
			ctx(ras, default_blender_t(c), winding<>());
		}

		shared_ptr<gcontext::text_engine_type> create_faked_text_engine()
		{
			shared_ptr<faked_text_engine_composite> c(new faked_text_engine_composite);

			return shared_ptr<gcontext::text_engine_type>(c, &c->text_engine_);
		}
	}
}

namespace agge
{
	bool operator ==(const wpl::gcontext::pixel_type &lhs, const wpl::gcontext::pixel_type &rhs)
	{	return !memcmp(&lhs, &rhs, sizeof(wpl::gcontext::pixel_type));	}

	bool operator ==(const color &lhs, const color &rhs)
	{	return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;	}
}
