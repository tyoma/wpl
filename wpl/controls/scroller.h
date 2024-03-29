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

#include "../concepts.h"
#include "../controls.h"
#include "../drag_helper.h"
#include "../models.h"
#include "integrated.h"

#include <agge/stroke.h>

namespace wpl
{
	namespace controls
	{
		class scroller : public integrated_control<wpl::scroller>
		{
		public:
			enum orientation { vertical, horizontal };

			struct thumb
			{
				bool active;
				agge::real_t width, lbound, ubound;
			};

		public:
			explicit scroller(orientation orientation_);

			// control methods
			virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

			// scroller methods
			virtual void set_model(std::shared_ptr<scroll_model> model) override;

			thumb get_thumb() const;

			// mouse_input methods
			virtual void mouse_down(mouse_buttons button_, int depressed, int x, int y) override;
			virtual void mouse_scroll(int depressed, int x, int y, int delta_x, int delta_y) override;

			// visual methods
			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const override;

		private:
			void page_less();
			void page_more();
			agge::real_t to_screen(const std::pair<double, double> &range, double c) const;

		private:
			const orientation _orientation;
			std::shared_ptr<scroll_model> _model;
			double _rextent;
			agge::real_t _width, _extent;
			mutable agge::stroke _thumb_style;
			slot_connection _underlying_invalidate;
			drag_helper _scroll;
		};
	}
}
