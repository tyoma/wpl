#pragma once

#include "animation.h"

#include <agge/types.h>
#include <ui/view.h>

namespace wpl
{
	namespace ui
	{
		struct color { unsigned char r, g, b, a; };

		class piechart : public view
		{
		public:
			struct model;

		public:
			piechart();

			void set_model(const std::shared_ptr<model> &m);
			void limit(unsigned max_segments);

		private:
			struct segment
			{
				float value;
				animation_line aline;
				color clr;
			};

			typedef std::vector<segment> segments_t;

		private:
			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const;
			virtual void resize(unsigned cx, unsigned cy);

			virtual void mouse_move(int buttons, int x, int y);
			virtual void mouse_leave();

			void update_animation(unsigned elapsed);

		private:
			segments_t _segments;
			int _center_x, _center_y, _base_radius;
			int _hover_index;
			std::shared_ptr<void> _animation_timer;
		};

		struct piechart::model
		{
			virtual ~model() {	}
			virtual float get_value(unsigned index) const = 0;
			virtual unsigned get_count() const = 0;
		};
	}
}
