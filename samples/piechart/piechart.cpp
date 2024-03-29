#include "piechart.h"

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/curves.h>
#include <agge/filling_rules.h>
#include <agge/math.h>
#include <agge/path.h>
#include <agge/stroke_features.h>
#include <math.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace
	{
		join<arc, arc> pie_segment(real_t cx, real_t cy, real_t outer_r, real_t inner_r, real_t start, real_t end)
		{	return join<arc, arc>(arc(cx, cy, outer_r, start, end), arc(cx, cy, inner_r, end, start));	}
	}

	typedef blender_solid_color<simd::blender_solid_color, platform_pixel_order> blender_t;

	piechart::piechart(const control_context &ctx)
		: _base_radius(200), _hover_index(-1), _clock(ctx.clock_), _animation_queue(ctx.queue_)
	{
		segment s;

		s.clr = color::make(200, 40, 40);
		s.value = 100;
		_segments.push_back(s);
		s.clr = color::make(40, 200, 40);
		s.value = 50;
		_segments.push_back(s);
		s.clr = color::make(40, 40, 200);
		s.value = 20;
		_segments.push_back(s);
	}

	void piechart::layout(const placed_view_appender &append_view, const agge::box<int> &box_)
	{
		_center_x = box_.w / 2, _center_y = box_.h / 2;
		controls::integrated_control<wpl::control>::layout(append_view, box_);
	}

	void piechart::draw(gcontext &ctx, gcontext::rasterizer_ptr &ras) const
	{
		float sum = 0.0, angle = -pi / 2;

		for (segments_t::const_iterator i = _segments.begin(); i != _segments.end(); ++i)
			sum += i->value;
		for (segments_t::const_iterator i = _segments.begin(); i != _segments.end(); ++i)
		{
			float d = 2 * pi * i->value / sum;

			ras->reset();
			add_path(*ras, pie_segment((real_t)_center_x, (real_t)_center_y,
				0.6f * _base_radius, (1.0f + i->aline.get_value()) * _base_radius,
				angle, angle + d));
			ras->close_polygon();
			ctx(ras, blender_t(agge::color::make(i->clr.r, i->clr.g, i->clr.b, i->clr.a)), winding<>());
			angle += d;
		}
	}

	void piechart::mouse_move(int /*buttons*/, int x, int y)
	{
		float sum = 0.0;
		float angle = atan2f(float(x - _center_x), float(_center_y - y)), check_angle = 0;
		int index = 0;

		if (angle < 0.0f)
			angle += 2 * pi;

		for (segments_t::const_iterator i = _segments.begin(); i != _segments.end(); ++i)
			sum += i->value;
		for (segments_t::const_iterator i = _segments.begin(); i != _segments.end(); ++i, ++index)
		{
			float d = 2 * pi * i->value / sum;

			check_angle += d;
			if (angle < check_angle)
				break;
		}
		if (index != _hover_index)
		{
			int index2 = 0;

			for (segments_t::iterator i = _segments.begin(); i != _segments.end(); ++i, ++index2)
			{
				if (index == index2)
					i->aline.run(0.1f, 200, _clock());
				else
					i->aline.run(-0.05f, 600, _clock());
			}
			_animation_queue([this] {	update_animation();	}, 10);
			_hover_index = index;
		}
	}

	void piechart::mouse_leave() throw()
	{
		for (segments_t::iterator i = _segments.begin(); i != _segments.end(); ++i)
			i->aline.run(0.0f, 200, _clock());
		_animation_queue([this] {	update_animation();	}, 10);
		_hover_index = -1;
	}

	void piechart::update_animation()
	{
		bool keep_going = false;
		auto t = _clock();

		for (segments_t::iterator i = _segments.begin(); i != _segments.end(); ++i)
			keep_going = i->aline.update(t) || keep_going;
		if (keep_going)
			_animation_queue([this] {	update_animation();	}, 10);
		invalidate(0);
	}
}
