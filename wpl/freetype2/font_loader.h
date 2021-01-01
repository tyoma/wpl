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
#include <unordered_map>

typedef struct FT_FaceRec_ FT_FaceRec;
typedef struct FT_LibraryRec_ FT_LibraryRec;
typedef struct FT_LibraryRec_ *FT_Library;

namespace wpl
{
	std::shared_ptr<gcontext::text_engine_type> create_text_engine();

	class font_loader : public gcontext::text_engine_type::loader, noncopyable
	{
	public:
		struct font_key_hasher
		{
			size_t operator ()(const agge::font_descriptor &key) const;
		};

		typedef std::unordered_map<agge::font_descriptor, std::pair<std::string, unsigned>, font_key_hasher> key_to_file_t;

	public:
		font_loader();

		virtual agge::font::accessor_ptr load(const agge::font_descriptor &descriptor) override;

	private:
		typedef std::function<bool (std::string &path)> enum_font_files_cb;

	private:
		static enum_font_files_cb create_fonts_enumerator();
		void build_index();

	private:
		std::shared_ptr<FT_LibraryRec_> _freetype;
		key_to_file_t _mapping;
	};

	class font_accessor : public agge::font::accessor
	{
	public:
		font_accessor(FT_Library freetype_, const std::string &path, unsigned index, int height,
			agge::font_hinting hinting);

	private:
		virtual agge::font_metrics get_metrics() const override;
		virtual agge::uint16_t get_glyph_index(wchar_t character) const override;
		virtual agge::glyph::outline_ptr load_glyph(agge::uint16_t index, agge::glyph::glyph_metrics &m) const override;

		agge::real_t scale_x(int value) const;
		static agge::real_t scale_y(int value);

	private:
		std::shared_ptr<FT_FaceRec> _face;
		agge::real_t _overscale;
		bool _hint;
	};
}
