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
		typedef agge::order_bgra pixel_color_model;
		typedef agge::blender_solid_color_rgb<gcontext::pixel_type, pixel_color_model> default_blender_t;


		struct ref_rectangle
		{
			ref_rectangle(agge::rect_i region_, agge::color color_)
				: region(region_), color(color_)
			{	}

			agge::rect_i region;
			agge::color color;
		};

		template <typename T>
		inline agge::point<T> make_point(T w, T h)
		{
			agge::point<T> b = { w, h };
			return b;
		}

		template <typename T>
		inline agge::box<T> make_box(T w, T h)
		{
			agge::box<T> b = { w, h };
			return b;
		}

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
		std::shared_ptr<gcontext::text_engine_type> create_faked_text_engine();
	}
}

namespace agge
{
	bool operator ==(const wpl::gcontext::pixel_type &lhs, const wpl::gcontext::pixel_type &rhs);

	bool operator ==(const color &lhs, const color &rhs);

	template <typename T>
	inline bool operator ==(const rect<T> &lhs, const rect<T> &rhs)
	{	return lhs.x1 == rhs.x1 && lhs.y1 == rhs.y1 && lhs.x2 == rhs.x2 && lhs.y2 == rhs.y2;	}

	template <typename T>
	inline bool operator ==(const box<T> &lhs, const box<T> &rhs)
	{	return lhs.w == rhs.w && lhs.h == rhs.h;	}

	template <typename T>
	inline bool operator ==(const point<T> &lhs, const point<T> &rhs)
	{	return lhs.x == rhs.x && lhs.y == rhs.y;	}
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

	inline void are_equal(const wpl::tests::ref_rectangle &expected, const wpl::gcontext::surface_type &actual,
		const LocationInfo &location)
	{
		const wpl::gcontext::pixel_type ref = wpl::tests::make_pixel(expected.color);

		is_true(expected.region.x2 <= static_cast<int>(actual.width()), location);
		is_true(expected.region.y2 <= static_cast<int>(actual.height()), location);
		for (int y = expected.region.y1; y < expected.region.y2; ++y)
		{
			const wpl::gcontext::pixel_type *p = actual.row_ptr(y);

			for (int x = expected.region.x1; x < expected.region.x2; ++x)
				are_equal(ref, p[x], location);
		}
	}
}
