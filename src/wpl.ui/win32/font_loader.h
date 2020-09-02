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

#include <agge.text/text_engine.h>

struct HFONT__;
typedef struct HFONT__ *HFONT;

namespace wpl
{
	namespace win32
	{
		class font_accessor : public agge::font::accessor
		{
		public:
			font_accessor(int height, const wchar_t *typeface, bool bold, bool italic,
				agge::font::key::grid_fit grid_fit);

			HFONT native() const;

		private:
			virtual agge::font::metrics get_metrics() const;
			virtual agge::uint16_t get_glyph_index(wchar_t character) const;
			virtual agge::glyph::outline_ptr load_glyph(agge::uint16_t index, agge::glyph::glyph_metrics &m) const;

		private:
			agge::shared_ptr<void> _native;
			agge::font::key::grid_fit _grid_fit;
		};

		class font_loader : public agge::text_engine_base::loader
		{
			virtual agge::font::accessor_ptr load(const wchar_t *typeface, int height, bool bold, bool italic,
				agge::font::key::grid_fit grid_fit);
		};
	}
}
