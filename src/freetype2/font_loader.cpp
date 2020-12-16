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

#include <wpl/freetype2/font_loader.h>

#include <agge/curves.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdexcept>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace
	{
		const int c_overscale = 50;
		const int c_26x6_1 = 64;
		const real_t c_26x6_unit = 1.0f / static_cast<real_t>(c_26x6_1);

		glyph::path_point path_point(int command, real_t x, real_t y)
		{
			glyph::path_point p = { command, x, y };
			return p;
		}

		void move_to(glyph::outline_storage &outline, real_t x, real_t y)
		{	outline.push_back(path_point(path_command_move_to, x, y));	}

		void line_to(glyph::outline_storage &outline, real_t x, real_t y)
		{	outline.push_back(path_point(path_command_line_to, x, y));	}

		void curve3(glyph::outline_storage &outline, real_t x2, real_t y2, real_t x3, real_t y3, real_t d = 0.03f)
		{
			const real_t x1 = (outline.end() - 1)->x, y1 = (outline.end() - 1)->y;
			
			for (real_t t = d; t < 1.0f; t += d)
			{
				real_t t_ = 1.0f - t;
				
				outline.push_back(path_point(path_command_line_to,
					t_ * t_ * x1 + 2.0f * t_ * t * x2 + t * t * x3,
					t_ * t_ * y1 + 2.0f * t_ * t * y2 + t * t * y3));
			}
			outline.push_back(path_point(path_command_line_to, x3, y3));
		}

		void curve4(glyph::outline_storage &outline, real_t x2, real_t y2, real_t x3, real_t y3, real_t x4, real_t y4, real_t d = 0.03f)
		{
			const real_t x1 = (outline.end() - 1)->x, y1 = (outline.end() - 1)->y;
			
			for (real_t t = d; t < 1.0f; t += d)
			{
				real_t x, y;

				agge::cbezier::calculate(&x, &y, x1, y1, x2, y2, x3, y3, x4, y4, t);
				outline.push_back(path_point(path_command_line_to, x, y));
			}
			outline.push_back(path_point(path_command_line_to, x3, y3));
		}

		void close_polygon(glyph::outline_storage &outline)
		{	outline.push_back(path_point(path_flag_close, 0.0f, 0.0f));	}
	}

	font_accessor::font_accessor(shared_ptr<FT_Library> freetype_, const wchar_t *font_family_, int height,
		bool /*bold*/, bool /*italic*/, font::key::grid_fit grid_fit)
		: _overscale(font::key::gf_vertical == grid_fit ? 1.0f / static_cast<real_t>(c_overscale) : 1.0f),
			_hint(font::key::gf_none != grid_fit)
	{
		unique_ptr<FT_Face> face_(new FT_Face);
		wstring font_family_w(font_family_);
		string font_family(font_family_w.begin(), font_family_w.end());

		if (FT_Error error = FT_New_Face(*freetype_, font_family.c_str(), 0, face_.get()))
			throw runtime_error("Cannot load fontface '" + font_family + "'!");
		_face.reset(face_.release(), [] (FT_Face *p) {
			FT_Done_Face(*p);
			delete p;
		});
		FT_Set_Char_Size(*_face, 0, height * c_26x6_1, static_cast<int>(96 / _overscale), 96);
	}

	font::metrics font_accessor::get_metrics() const
	{
		font::metrics m = {
			-scale_y((*_face)->size->metrics.ascender),
			scale_y((*_face)->size->metrics.descender),
			0.0f,
		};

		return m;
	}

	uint16_t font_accessor::get_glyph_index(wchar_t character) const
	{	return static_cast<uint16_t>(FT_Get_Char_Index(*_face, character));	}

	glyph::outline_ptr font_accessor::load_glyph(uint16_t index, glyph::glyph_metrics &m) const
	{
		glyph::outline_ptr path(new glyph::outline_storage);

		if (FT_Error error = FT_Load_Glyph(*_face, index, _hint ? FT_LOAD_DEFAULT : FT_LOAD_NO_HINTING))
			return path;
		m.advance_x = scale_x((*_face)->glyph->advance.x);
		m.advance_y = scale_y((*_face)->glyph->advance.y);

		FT_Outline outline = (*_face)->glyph->outline;
		FT_Vector v_last;
		FT_Vector v_control;
		FT_Vector v_start;
		FT_Vector *point;
		FT_Vector *limit;
		char *tags;

		int first = 0; // index of first point in contour
		char tag; // current point's state

		for(int n = 0; n < outline.n_contours; n++)
		{
			int last;  // index of last point in contour

			last = outline.contours[n];
			limit = outline.points + last;

			v_start = outline.points[first];
			v_last = outline.points[last];

			v_control = v_start;

			point = outline.points + first;
			tags = outline.tags  + first;
			tag = FT_CURVE_TAG(tags[0]);

			// A contour cannot start with a cubic control point!
			if(tag == FT_CURVE_TAG_CUBIC) return path->clear(), path;

			// check first point to determine origin
			if( tag == FT_CURVE_TAG_CONIC)
			{
				// first point is conic control.  Yes, this happens.
				if(FT_CURVE_TAG(outline.tags[last]) == FT_CURVE_TAG_ON)
				{
					// start at last point if it is on the curve
					v_start = v_last;
					limit--;
				}
				else
				{
					// if both first and last points are conic,
					// start at their middle and record its position
					// for closure
					v_start.x = (v_start.x + v_last.x) / 2;
					v_start.y = (v_start.y + v_last.y) / 2;

					v_last = v_start;
				}
				point--;
				tags--;
			}

			move_to(*path, scale_x(v_start.x), scale_y(v_start.y));

			while(point < limit)
			{
				point++;
				tags++;

				tag = FT_CURVE_TAG(tags[0]);
				switch(tag)
				{
				case FT_CURVE_TAG_ON:  // emit a single line_to
					{
						line_to(*path, scale_x(point->x), scale_y(point->y));
						continue;
					}

				case FT_CURVE_TAG_CONIC:  // consume conic arcs
					{
						v_control.x = point->x;
						v_control.y = point->y;

Do_Conic:
						if(point < limit)
						{
							FT_Vector vec;
							FT_Vector v_middle;

							point++;
							tags++;
							tag = FT_CURVE_TAG(tags[0]);

							vec.x = point->x;
							vec.y = point->y;

							if(tag == FT_CURVE_TAG_ON)
							{
								curve3(*path, scale_x(v_control.x), scale_y(v_control.y), scale_x(vec.x), scale_y(vec.y));
								continue;
							}

							if(tag != FT_CURVE_TAG_CONIC) return path->clear(), path;

							v_middle.x = (v_control.x + vec.x) / 2;
							v_middle.y = (v_control.y + vec.y) / 2;
							curve3(*path, scale_x(v_control.x), scale_y(v_control.y),
								scale_x(v_middle.x), scale_y(v_middle.y));
							v_control = vec;
							goto Do_Conic;
						}

						curve3(*path, scale_x(v_control.x), scale_y(v_control.y), scale_x(v_start.x), scale_y(v_start.y));
						goto Close;
					}

				default: // FT_CURVE_TAG_CUBIC
					{
						FT_Vector vec1, vec2;

						if(point + 1 > limit || FT_CURVE_TAG(tags[1]) != FT_CURVE_TAG_CUBIC)
						{
							return path->clear(), path;
						}

						vec1.x = point[0].x; 
						vec1.y = point[0].y;
						vec2.x = point[1].x; 
						vec2.y = point[1].y;

						point += 2;
						tags  += 2;

						if(point <= limit)
						{
							FT_Vector vec;

							vec.x = point->x;
							vec.y = point->y;

							curve4(*path, scale_x(vec1.x), scale_y(vec1.y), scale_x(vec2.x), scale_y(vec2.y),
								scale_x(vec.x), scale_y(vec.y));
							continue;
						}

						curve4(*path, scale_x(vec1.x), scale_y(vec1.y), scale_x(vec2.x), scale_y(vec2.y),
							scale_x(v_start.x), scale_y(v_start.y));
						goto Close;
					}
				}
			}
			close_polygon(*path);

		Close:
			first = last + 1; 
		}
		return path;
	}

	real_t font_accessor::scale_x(int value) const
	{	return static_cast<real_t>(value) * c_26x6_unit * _overscale;	}

	real_t font_accessor::scale_y(int value)
	{	return static_cast<real_t>(-value) * c_26x6_unit;	}


	font_loader::font_loader()
	{
		unique_ptr<FT_Library> freetype_(new FT_Library);

		if (FT_Error error = FT_Init_FreeType(freetype_.get()))
			throw runtime_error("Cannot initialize freetype2!");
		_freetype.reset(freetype_.release(), [] (FT_Library *p) {
			FT_Done_FreeType(*p);
			delete p;
		});
	}

	font::accessor_ptr font_loader::load(const wchar_t *family_filename, int height, bool bold, bool italic,
		font::key::grid_fit grid_fit)
	{	return make_shared<font_accessor>(_freetype, family_filename, height, bold, italic, grid_fit);	}
}
