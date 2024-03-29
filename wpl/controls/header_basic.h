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

#include "header_core.h"

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
			header_basic(std::shared_ptr<gcontext::text_engine_type> text_services,
				std::shared_ptr<cursor_manager> cursor_manager_);
			~header_basic();

			void apply_styles(const stylesheet &stylesheet_);

			// header methods
			virtual void set_model(std::shared_ptr<headers_model> model) override;

		private:
			// visual methods
			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const override;

			// header_core methods
			virtual agge::box<int> measure_item(const headers_model &model, index_type index) const override;
			virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &ras, const agge::rect_r &b,
				const headers_model &model, index_type item, unsigned /*item_state_flags*/ state) const override;

		private:
			std::shared_ptr<gcontext::text_engine_type> _text_services;
			std::shared_ptr<headers_model> _model;
			agge::real_t _padding, _separator_width;
			agge::color _bg, _bg_sorted, _fg_normal, _fg_sorted, _fg_separator, _fg_indicator;
			std::unique_ptr<glyph> _up, _down;
			mutable agge::richtext_t _caption_buffer;
		};
	}
}
