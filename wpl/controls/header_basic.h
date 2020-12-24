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

#include <wpl/controls/header_core.h>

#include <agge/color.h>
#include <agge.text/font.h>
#include <memory>

namespace wpl
{
	class glyph;
	struct stylesheet;

	namespace controls
	{
		class header_basic : public header_core
		{
		public:
			header_basic(std::shared_ptr<cursor_manager> cursor_manager_);
			~header_basic();

			void apply_styles(const stylesheet &stylesheet_);

		private:
			// visual methods
			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const override;

			// header_core methods
			virtual void draw_item_background(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned /*item_state_flags*/ state) const override;
			virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &ras, const agge::rect_r &b,
				index_type item, unsigned /*item_state_flags*/ state, const std::wstring &text) const override;

		private:
			agge::real_t _padding, _baseline_offset, _separator_width;
			agge::font::ptr _font;
			agge::color _bg, _bg_sorted, _fg_normal, _fg_sorted, _fg_separator, _fg_indicator;
			std::unique_ptr<glyph> _up, _down;
		};
	}
}
