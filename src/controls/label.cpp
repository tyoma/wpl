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

#include <wpl/controls/label.h>

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/filling_rules.h>
#include <agge.text/text_engine.h>
#include <wpl/stylesheet.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace
	{
		const font_style_annotation c_base_annotation = {	font_descriptor::create("Arial", 10),	};
		typedef blender_solid_color<simd::blender_solid_color, platform_pixel_order> blender;
	}

	namespace controls
	{
		label::label(shared_ptr<gcontext::text_engine_type> text_services)
			: _text_services(text_services), _text_buffer(c_base_annotation), _text(style_modifier::empty),
				_layout(*_text_services), _halign(align_near), _valign(align_center)
		{	}

		void label::apply_styles(const stylesheet &stylesheet_)
		{
			font_style_annotation a = {	stylesheet_.get_font("text.label")->get_key(),	};

			_text_buffer.set_base_annotation(a);
			_text_buffer << _text;
			_color = stylesheet_.get_color("text.label");
			layout_changed(false);
		}

		int label::min_height(int for_width) const
		{
			_layout.set_width_limit(static_cast<real_t>(for_width));
			_layout.process(_text_buffer);
			return static_cast<int>(_layout.get_box().h + 1.0);
		}

		int label::min_width(int /*for_width*/) const
		{	return static_cast<int>(_text_services->measure(_text_buffer).w + 1.0);	}

		void label::set_text(const richtext_modifier_t &text)
		{
			_text_buffer.clear();
			_text = text;
			_text_buffer << text;
			layout_changed(false);
		}

		void label::set_halign(text_alignment value)
		{	_halign = value, invalidate(nullptr);	}

		void label::set_valign(text_alignment value)
		{	_valign = value, invalidate(nullptr);	}

		void label::draw(gcontext &context, gcontext::rasterizer_ptr &rasterizer_) const
		{
			context.text_engine.render(*rasterizer_, _text_buffer, _halign, _valign,
				create_rect(0.0f, 0.0f, static_cast<real_t>(get_last_size().w), static_cast<real_t>(get_last_size().h)));
			context(rasterizer_, blender(_color), winding<>());
		}
	}
}
