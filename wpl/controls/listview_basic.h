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

#include "listview_core.h"

#include <agge/color.h>
#include <agge/dash.h>
#include <agge/stroke.h>
#include <agge.text/font.h>
#include <memory>

namespace wpl
{
	struct stylesheet;

	namespace controls
	{
		class listview_basic : public listview_core
		{
		public:
			void apply_styles(const stylesheet &stylesheet_);

		protected:
			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const override;

			virtual agge::real_t get_minimal_item_height() const override;
			virtual void draw_item_background(wpl::gcontext &ctx, wpl::gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned state) const override;
			virtual void draw_item(wpl::gcontext &ctx, wpl::gcontext::rasterizer_ptr &ras, const agge::rect_r &b,
				index_type item, unsigned state) const override;
			virtual void draw_subitem(wpl::gcontext &ctx, wpl::gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned state, wpl::columns_model::index_type subitem,
				const std::string &text) const override;

		private:
			agge::real_t _item_height, _padding, _baseline_offset;
			agge::font::ptr _font;
			agge::color _bg, _bg_even, _bg_odd, _bg_selected, _fg_normal, _fg_selected, _fg_focus, _fg_focus_selected;
			mutable agge::stroke _stroke;
			mutable agge::dash _dash;
		};
	}
}
