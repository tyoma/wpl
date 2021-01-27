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
#include "integrated.h"

#include <agge/color.h>
#include <agge.text/layout.h>

namespace wpl
{
	struct stylesheet;

	namespace controls
	{
		class label : public integrated_control<wpl::label>
		{
		public:
			label(std::shared_ptr<gcontext::text_engine_type> text_services);

			void apply_styles(const stylesheet &stylesheet_);

		private:
			// control methods
			virtual int min_height(int for_width) const override;
			virtual int min_width(int for_width) const override;

			// text_container methods
			virtual void set_text(const agge::richtext_modifier_t &text) override;
			virtual void set_halign(agge::text_alignment value) override;
			virtual void set_valign(agge::text_alignment value) override;

			// visual methods
			virtual void draw(gcontext &context, gcontext::rasterizer_ptr &rasterizer) const override;

		private:
			const std::shared_ptr<gcontext::text_engine_type> _text_services;
			agge::richtext_t _text;
			agge::color _color;
			mutable agge::layout _layout;
			agge::text_alignment _halign, _valign;
		};
	}
}
