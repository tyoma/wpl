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

#include <wpl/misc/statistics_view.h>

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/color.h>
#include <agge/figures.h>
#include <agge/filling_rules.h>
#include <agge.text/text_engine.h>
#include <wpl/helpers.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace misc
	{
		statistics_view::statistics_view(shared_ptr<gcontext::text_engine_type> text_services)
			: _text_services(text_services)
		{
			font_style_annotation a = {	font_descriptor::create("Arial", 10),	};
			_text.set_base_annotation(a);
		}

		rect_i statistics_view::update()
		{
			wchar_t buffer[100];

			_text.clear();
			for (auto i = _values.begin(); i != _values.end(); ++i)
			{
				_text.append(i->first.begin(), i->first.end());
				_text.append(L": ");
				switch (i->second.current_type)
				{
				case variant::type_float:
					swprintf(buffer, 100, L"%g", static_cast<double>(i->second.value.f));
					_text.append(buffer);
					break;

				case variant::type_int:
					_text.append(to_wstring(static_cast<long long>(i->second.value.i)));
					break;

				case variant::type_box:
					_text.append(to_wstring(static_cast<long long>(i->second.value.b.w)));
					_text.append(L"x");
					_text.append(to_wstring(static_cast<long long>(i->second.value.b.h)));
					break;
				}
				_text.append(i->second.unit.begin(), i->second.unit.end());
				_text.append(L"\n");
			}
			const auto b = _text_services->measure(_text);
			const auto invalid = create_rect(0, 0, static_cast<int>(b.w) + 11, static_cast<int>(b.h) + 11);
			auto full_invalid = invalid;

			unite(full_invalid, _last_invalid);
			_last_invalid = invalid;
			return full_invalid;
		}

		void statistics_view::draw(gcontext &context, gcontext::rasterizer_ptr &rasterizer_) const
		{
			typedef blender_solid_color<simd::blender_solid_color, platform_pixel_order> blender;

			if (!_text.empty())
			{
				auto r = create_rect(static_cast<real_t>(_last_invalid.x1), static_cast<real_t>(_last_invalid.y1),
					static_cast<real_t>(_last_invalid.x2), static_cast<real_t>(_last_invalid.y2));

				add_path(*rasterizer_, rectangle(r.x1, r.y1, r.x2, r.y2));
				context(rasterizer_, blender(color::make(64, 64, 64, 192)), winding<>());
				inflate(r, -5.0f, -5.0f);
				context.text_engine.render(*rasterizer_, _text, align_near, align_near, r);
				context(rasterizer_, blender(color::make(255, 255, 255)), winding<>());
			}
		}
	}
}
