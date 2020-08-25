#pragma once

#include <agge/blenders_generic.h>
#include <agge/color.h>
#include <agge/types.h>
#include <ut/assert.h>
#include <wpl/concepts.h>
#include <wpl/visual.h>

namespace wpl
{
	namespace tests
	{
		class context : noncopyable
		{
		public:
			typedef agge::order_bgra pixel_color_model;
			typedef agge::blender_solid_color_rgb<gcontext::pixel_type, pixel_color_model> blender_t;

		public:
			context();
			~context();

			void resize(agge::count_t width, agge::count_t height);
			void reset(agge::color c);

		public:
			gcontext::surface_type surface;
			gcontext::rasterizer_ptr rasterizer;
			std::shared_ptr<gcontext::renderer_type> renderer;
		};



		template <typename T>
		inline agge::agge_vector<T> make_vector(T dx, T dy)
		{
			agge::agge_vector<T> v = { dx, dy };
			return v;
		}

		template <typename T>
		inline agge::rect<T> make_rect(T x1, T y1, T x2, T y2)
		{
			agge::rect<T> r = { x1, y1, x2, y2 };
			return r;
		}

		view_location make_position(int x, int y, int width, int height);
		gcontext::pixel_type make_pixel(const agge::color& color);
		gcontext::pixel_type make_pixel_real(const agge::color& color);
		void reset(gcontext::surface_type &surface, gcontext::pixel_type ref_pixel);
		void rectangle(gcontext &ctx, agge::color c, int x1, int y1, int x2, int y2);
	}

	bool operator ==(const view_location &lhs, const view_location &rhs);
}

namespace agge
{
	bool operator ==(const wpl::gcontext::pixel_type &lhs, const wpl::gcontext::pixel_type &rhs);

	template <typename T>
	inline bool operator ==(const rect<T> &lhs, const rect<T> &rhs)
	{	return lhs.x1 == rhs.x1 && lhs.y1 == rhs.y1 && lhs.x2 == rhs.x2 && lhs.y2 == rhs.y2;	}
}

namespace ut
{
	template <size_t n>
	inline void are_equal(const wpl::gcontext::pixel_type (&expected)[n], const wpl::gcontext::surface_type &actual,
		const LocationInfo &location)
	{
		are_equal(n, actual.width() * actual.height(), location);
		for (agge::count_t y = 0; y < actual.height(); ++y)
		{
			const wpl::gcontext::pixel_type *p = actual.row_ptr(y);

			for (agge::count_t x = 0; x < actual.width(); ++x)
				are_equal(expected[x + y * actual.width()], p[x], location);
		}
	}
}
