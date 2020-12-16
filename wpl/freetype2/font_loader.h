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

#include "../visual.h"

#include <agge.text/text_engine.h>

typedef struct FT_FaceRec_ *FT_Face;
typedef struct FT_LibraryRec_ *FT_Library;

namespace wpl
{
	class font_loader : public gcontext::text_engine_type::loader, noncopyable
	{
	public:
		font_loader();

		virtual agge::font::accessor_ptr load(const wchar_t *family_filename, int height, bool bold, bool italic,
			agge::font::key::grid_fit grid_fit) override;

	private:
		std::shared_ptr<FT_Library> _freetype;
	};

	class font_accessor : public agge::font::accessor
	{
	public:
		font_accessor(std::shared_ptr<FT_Library> freetype_, const wchar_t *font_family, int height, bool bold,
			bool italic, agge::font::key::grid_fit grid_fit);

	private:
		virtual agge::font::metrics get_metrics() const override;
		virtual agge::uint16_t get_glyph_index(wchar_t character) const override;
		virtual agge::glyph::outline_ptr load_glyph(agge::uint16_t index, agge::glyph::glyph_metrics &m) const override;

		agge::real_t scale_x(int value) const;
		static agge::real_t scale_y(int value);

	private:
		std::shared_ptr<FT_Face> _face;
		agge::real_t _overscale;
		bool _hint;
	};
}
