#pragma once

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/figures.h>
#include <agge/filling_rules.h>
#include <agge/path.h>

template <typename BaseT>
class view_fill : public BaseT
{
	void draw(wpl::ui::gcontext &ctx, wpl::ui::gcontext::rasterizer_ptr &ras) const
	{
		using namespace agge;

		typedef blender_solid_color<simd::blender_solid_color, order_bgra> blender_t;

		rect_i rc = ctx.update_area();

		ras->reset();
		add_path(*ras, rectangle((real_t)rc.x1, (real_t)rc.y1, (real_t)rc.x2, (real_t)rc.y2));
		ctx(ras, blender_t(agge::color::make(10, 20, 40, 255)), winding<>());
		BaseT::draw(ctx, ras);
	}
};
