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

namespace wpl
{
	struct cursor_manager;

	namespace controls
	{
		class header_core : public integrated_control<wpl::header>, public index_traits
		{
		public:
			enum item_state_flags {
				ascending = 1 << 0,
				sorted = 1 << 1,
			};

		public:
			header_core(std::shared_ptr<cursor_manager> cursor_manager_);
			~header_core();

			void set_offset(double offset);
			void adjust_column_widths();

			// control methods
			virtual int min_height(int for_width) const override;

			// header methods
			virtual void set_model(std::shared_ptr<headers_model> model) override;

			// mouse_input methods
			virtual void mouse_enter() override;
			virtual void mouse_leave() throw() override;
			virtual void mouse_move(int depressed, int x, int y) override;
			virtual void mouse_down(mouse_buttons button_, int buttons, int x, int y) override;
			virtual void mouse_up(mouse_buttons button_, int buttons, int x, int y) override;

			// visual methods
			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const override;

		private:
			enum handle_type { none_handle, column_handle, resize_handle };

		private:
			virtual agge::box<int> measure_item(const headers_model &model, index_type item) const;
			virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer, const agge::rect_r &box,
				const headers_model &model, index_type item, unsigned /*item_state_flags*/ state) const = 0;

			std::pair<index_type, handle_type> handle_from_point(int x) const;

		private:
			const std::shared_ptr<cursor_manager> _cursor_manager;
			std::shared_ptr<headers_model> _model;
			agge::real_t _offset;
			drag_helper _resize;
			std::pair<index_type, bool /*ascending*/> _sorted_column;
			slot_connection _model_invalidation;
			slot_connection _model_sorting_change;
			bool _ignore_invalidations;
		};
	}
}
