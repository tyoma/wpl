//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
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
#include "../view_helpers.h"
#include "integrated.h"

#include <vector>

namespace wpl
{
	namespace controls
	{
		class listview_core : public on_focus_invalidate< integrated_control<wpl::listview> >, public index_traits
		{
		public:
			enum item_state_flags {
				hovered = 1 << 0,
				selected = 1 << 1,
				focused = 1 << 2,
			};

		public:
			listview_core();
			~listview_core();

			std::shared_ptr<scroll_model> get_vscroll_model();
			std::shared_ptr<scroll_model> get_hscroll_model();
			void make_visible(index_type item);

			// keyboard_input methods
			virtual void key_down(unsigned code, int modifiers) override;

			// mouse_input methods
			virtual void mouse_down(mouse_buttons button_, int buttons, int x, int y) override;
			virtual void mouse_up(mouse_buttons button_, int buttons, int x, int y) override;
			virtual void mouse_double_click(mouse_buttons button_, int depressed, int x, int y) override;

			// visual methods
			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const override;

			// control methods
			virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;
			virtual int min_height(int for_width) const override;

			// listview methods
			void set_columns_model(std::shared_ptr<columns_model> cmodel);
			void set_model(std::shared_ptr<table_model_base> model);

			virtual void select(index_type item, bool reset_previous) override;
			virtual void focus(index_type item) override;

		private:
			typedef std::shared_ptr<const trackable> trackable_ptr;
			typedef std::vector<trackable_ptr> trackables;

			struct trackable_less;
			struct base_scroll_model;
			struct vertical_scroll_model;
			struct horizontal_scroll_model;

		private:
			virtual agge::real_t get_minimal_item_height() const = 0;
			virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, unsigned layer, index_type row, unsigned /*item_state_flags*/ state) const = 0;
			virtual void draw_subitem(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, unsigned layer, index_type row, unsigned /*item_state_flags*/ state,
				columns_model::index_type column) const = 0;

			void invalidate_();
			void toggle_selection(index_type item);
			void precache_model();
			agge::real_t get_visible_count() const;
			std::pair<index_type, index_type> get_visible_range() const;
			std::pair<columns_model::index_type, columns_model::index_type> update_horizontal_visible_range() const;
			index_type first_partially_visible() const;
			index_type last_partially_visible() const;
			index_type get_item(int y) const;
			bool is_selected(index_type item) const;
			bool is_visible(index_type item) const;

		private:
			std::shared_ptr<columns_model> _cmodel;
			std::shared_ptr<table_model_base> _model;
			std::pair<string_table_model::index_type, string_table_model::index_type> _precached_range;
			string_table_model::index_type _item_count;
			std::shared_ptr<vertical_scroll_model> _vsmodel;
			std::shared_ptr<horizontal_scroll_model> _hsmodel;
			slot_connection _model_invalidation, _cmodel_invalidation;
			agge::agge_vector<double> _offset;
			mutable agge::real_t _total_width;
			mutable std::vector< std::pair<agge::real_t /*x1*/, agge::real_t /*x2*/> > _subitem_positions;

			trackable_ptr _focused;
			trackables _selected;

			bool _state_vscrolling : 1;
			bool _state_keep_focus_visible : 1;
		};
	}
}
