//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../models.h"
#include "../view.h"

namespace wpl
{
	namespace controls
	{
		class header : public view, public index_traits_t<short int>
		{
		public:
			enum item_state_flags {
				ascending = 1 << 0,
				sorted = 1 << 1,
			};

		public:
			header();

			void set_model(const std::shared_ptr<columns_model> &model);

			// mouse_input methods
			virtual void mouse_move(int depressed, int x, int y);
			virtual void mouse_down(mouse_buttons button, int buttons, int x, int y);
			virtual void mouse_up(mouse_buttons button, int buttons, int x, int y);

			// visual methods
			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const;
			virtual void resize(unsigned cx, unsigned cy, positioned_native_views &native_views);

		private:
			enum handle_type { none_handle, column_handle, resize_handle };

		private:
			virtual void draw_item_background(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned /*item_state_flags*/ state) const;
			virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned /*item_state_flags*/ state,
				const std::wstring &text) const = 0;

			std::pair<index_type, handle_type> handle_from_point(int x) const;

		private:
			std::shared_ptr<columns_model> _model;
			agge::box_r _size;
			std::pair<index_type, int /* click point */> _resizing_colum;
			std::shared_ptr<void> _resizing_capture;
			std::pair<index_type, bool /*ascending*/> _sorted_column;
			slot_connection _model_invalidation;
			slot_connection _model_sorting_change;
		};
	}
}
