#pragma once

#include "animation.h"

#include <agge/color.h>
#include <vector>
#include <wpl/control.h>
#include <wpl/controls/integrated.h>
#include <wpl/factory_context.h>

namespace wpl
{
	class piechart : public controls::integrated_control<control>
	{
	public:
		struct model;

	public:
		piechart(const control_context &ctx);

		void set_model(const std::shared_ptr<model> &m);
		void limit(unsigned max_segments);

	private:
		struct segment
		{
			float value;
			animation_line aline;
			agge::color clr;
		};

		typedef std::vector<segment> segments_t;

	private:
		virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

		virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const override;

		virtual void mouse_move(int depressed, int x, int y) override;
		virtual void mouse_leave() throw() override;

		void update_animation();

	private:
		segments_t _segments;
		int _center_x, _center_y, _base_radius;
		int _hover_index;
		wpl::clock _clock;
		wpl::queue _animation_queue;
	};

	struct piechart::model
	{
		virtual ~model() {	}
		virtual float get_value(unsigned index) const = 0;
		virtual unsigned get_count() const = 0;
	};
}
