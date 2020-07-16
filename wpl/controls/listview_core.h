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

#include "../listview.h"
#include "../view.h"

#include <set>

namespace wpl
{
	namespace controls
	{
		class listview_core : public wpl::listview, public view, public std::enable_shared_from_this<view>, noncopyable
		{
		public:
			enum item_state_flags {	hovered = 1, selected = 2, focused = 4,	};

		public:
			listview_core();
			~listview_core();

			std::shared_ptr<scroll_model> get_vscroll_model();
			std::shared_ptr<scroll_model> get_hscroll_model();

			// keyboard_input methods
			virtual void key_down(unsigned code, int modifiers);

			// mouse_input methods
			virtual void mouse_down(mouse_buttons button, int buttons, int x, int y);
			virtual void mouse_up(mouse_buttons button, int buttons, int x, int y);

			// visual methods
			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const;
			virtual void resize(unsigned cx, unsigned cy, positioned_native_views &native_views);

			// control methods
			virtual std::shared_ptr<view> get_view();

			// listview methods
			virtual void set_columns_model(std::shared_ptr<columns_model> cmodel);
			virtual void set_model(std::shared_ptr<table_model> model);

			virtual void adjust_column_widths();

			virtual void select(index_type item, bool reset_previous);
			virtual void clear_selection();

			virtual void ensure_visible(index_type item);

		private:
			struct vertical_scroll_model;
			struct horizontal_scroll_model;

		private:
			virtual agge::real_t get_item_height() const = 0;
			virtual void draw_item_background(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned /*item_state_flags*/ state) const = 0;
			virtual void draw_subitem_background(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned /*item_state_flags*/ state, index_type subitem) const;
			virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned /*item_state_flags*/ state) const;
			virtual void draw_subitem(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned /*item_state_flags*/ state, index_type subitem,
				const std::wstring &text) const = 0;

			index_type get_item(int y) const;
			void toggle_selection(index_type item);

		private:
			std::shared_ptr<columns_model_base> _cmodel;
			std::shared_ptr<table_model> _model;
			table_model::index_type _item_count;
			std::shared_ptr<vertical_scroll_model> _vsmodel;
			std::shared_ptr<horizontal_scroll_model> _hsmodel;
			slot_connection _model_invalidation;
			agge::box_r _size;
			double _first_visible;
			mutable std::vector<agge::real_t> _widths;
			mutable std::wstring _text_buffer;

			table_model::index_type _focus_item;
			std::set<index_type> _selected_items;
		};
	}
}
