//	Copyright (c) 2011-2022 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#pragma once

#include "../controls.h"
#include "../drag_helper.h"
#include "integrated.h"

#include <agge/stroke.h>

namespace wpl
{
	namespace controls
	{
		class range_slider_core : public integrated_control<wpl::range_slider>
		{
		public:
			enum thumb_part {	part_none, part_near, part_far, part_shaft,	};

		public:
			// control methods
			virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

			// unimodel_control methods
			virtual void set_model(std::shared_ptr<sliding_window_model> model) override;

		protected:
			struct range
			{
				agge::real_t near_x, far_x;
			};

			struct descriptor
			{
				agge::real_t y;
				range channel, thumb;
			};

		private:
			// mouse_input methods
			virtual void mouse_down(mouse_buttons button_, int depressed, int x, int y) override;

			// visual methods
			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const override;

			virtual descriptor initialize(agge::box_r box) const = 0;
			virtual void draw(const descriptor &state, gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const = 0;
			virtual thumb_part hit_test(const descriptor &state, agge::point_r point) = 0;

			static void recalculate(descriptor &state, const sliding_window_model &model);

		private:
			drag_helper _drag;
			std::shared_ptr<sliding_window_model> _model;
			descriptor _state;
			slot_connection _invalidation;
		};


		class range_slider : public range_slider_core
		{
			virtual descriptor initialize(agge::box_r box) const override;
			virtual void draw(const descriptor &state, gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const override;
			virtual thumb_part hit_test(const descriptor &state, agge::point_r point) override;

		private:
			mutable agge::stroke _stroke[2];
		};
	}
}
