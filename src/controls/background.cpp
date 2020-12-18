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

#include <wpl/controls/background.h>

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/figures.h>
#include <agge/filling_rules.h>
#include <agge/path.h>
#include <wpl/stylesheet.h>

using namespace agge;

namespace wpl
{
	namespace controls
	{
		namespace
		{
			typedef blender_solid_color<simd::blender_solid_color, platform_pixel_order> blender;
		}

		void solid_background::apply_styles(const stylesheet &stylesheet_)
		{
			_color = stylesheet_.get_color("background");
			invalidate(nullptr);
		}

		void solid_background::draw(gcontext &context, gcontext::rasterizer_ptr &rasterizer_) const
		{
			const auto r = context.update_area();

			add_path(*rasterizer_, rectangle(static_cast<real_t>(r.x1), static_cast<real_t>(r.y1),
				static_cast<real_t>(r.x2), static_cast<real_t>(r.y2)));
			context(rasterizer_, blender(_color), winding<>());
		}
	}
}
