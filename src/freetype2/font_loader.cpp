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

#include <wpl/freetype2/font_loader.h>

#include <agge/curves.h>
#include <algorithm>
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


		struct text_engine_composite
		{
			text_engine_composite()
				: text_engine(loader)
			{	}

			font_loader loader;
			gcontext::text_engine_type text_engine;
		};

		struct nc_compare
		{
			bool operator ()(char lhs, char rhs) const
			{	return toupper(lhs) < toupper(rhs);	}
		};

		struct nc_equal
		{
			bool operator ()(char lhs, char rhs) const
			{	return toupper(lhs) == toupper(rhs);	}
		};



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

				cbezier::calculate(&x, &y, x1, y1, x2, y2, x3, y3, x4, y4, t);
				outline.push_back(path_point(path_command_line_to, x, y));
			}
			outline.push_back(path_point(path_command_line_to, x3, y3));
		}

		void close_polygon(glyph::outline_storage &outline)
		{	outline.push_back(path_point(path_flag_close, 0.0f, 0.0f));	}

		shared_ptr<FT_LibraryRec_> create_freetype()
		{
			FT_Library freetype;

			if (FT_Error error = FT_Init_FreeType(&freetype))
				throw runtime_error("Cannot initialize freetype!");
			return shared_ptr<iterator_traits<FT_Library>::value_type>(freetype, [] (FT_Library p) {
				FT_Done_FreeType(p);
			});
		}

		shared_ptr<FT_FaceRec_> open_face(FT_Library freetype, const string &path, unsigned index)
		{
			FT_Face face;

			if (FT_Error error = FT_New_Face(freetype, path.c_str(), index, &face))
				throw runtime_error("Cannot load font-face '" + path +"'!");
			return shared_ptr<iterator_traits<FT_Face>::value_type>(face, [] (FT_Face p) {
				FT_Done_Face(p);
			});
		}

		font_weight find_weight(string &styles)
		{
			for_each(styles.begin(), styles.end(), [] (char &c) {	c = static_cast<char>(toupper(c));	});

			if (string::npos != styles.find("REGULAR")) return regular;
			if (string::npos != styles.find("SEMIBOLD") || string::npos != styles.find("DEMIBOLD")) return semi_bold;
			if (string::npos != styles.find("BOLD")) return bold;
			if (string::npos != styles.find("EXTRALIGHT")) return extra_light;
			if (string::npos != styles.find("LIGHT")) return light;
			if (string::npos != styles.find("MEDIUM")) return medium;
			if (string::npos != styles.find("EXTRABLACK")) return extra_black;
			if (string::npos != styles.find("BLACK")) return black;
			return regular;
		}
	}


	bool font_loader::key_less::operator ()(const agge::font_descriptor &lhs, const agge::font_descriptor &rhs) const
	{
		const auto compare = [] (const string &l, const string &r) -> bool {
			return lexicographical_compare(l.begin(), l.end(), r.begin(), r.end(), nc_compare());
		};

		return compare(lhs.family, rhs.family) ? true : compare(rhs.family, lhs.family) ? false :
			lhs.italic < rhs.italic ? true : rhs.italic < lhs.italic ? false :
			lhs.weight < rhs.weight;
	}


	font_accessor::font_accessor(FT_Library freetype_, const string &path, unsigned index, int height,
			font_hinting hinting)
		: _face(open_face(freetype_, path, index)),
			_overscale(hint_vertical == hinting ? 1.0f / static_cast<real_t>(c_overscale) : 1.0f), _height(height),
			_hinting(hinting)
	{	FT_Set_Pixel_Sizes(_face.get(), static_cast<int>(height / _overscale), height);	}

	font_descriptor font_accessor::get_descriptor() const
	{
		return font_descriptor::create(_face->family_name, _height,
			(FT_STYLE_FLAG_BOLD & _face->style_flags) ? bold : regular, !!(FT_STYLE_FLAG_ITALIC & _face->style_flags),
			_hinting);
	}

	font_metrics font_accessor::get_metrics() const
	{
		font_metrics m = {
			-scale_y(_face->size->metrics.ascender),
			scale_y(_face->size->metrics.descender),
			0.0f,
		};

		return m;
	}

	agge::uint16_t font_accessor::get_glyph_index(wchar_t character) const
	{	return static_cast<agge::uint16_t>(FT_Get_Char_Index(_face.get(), character));	}

	glyph::outline_ptr font_accessor::load_glyph(agge::uint16_t index, glyph::glyph_metrics &m) const
	{
		glyph::outline_ptr path(new glyph::outline_storage);

		if (FT_Error error = FT_Load_Glyph(_face.get(), index, _hinting ? FT_LOAD_DEFAULT : FT_LOAD_NO_HINTING))
			return path;
		m.advance_x = scale_x(_face->glyph->advance.x);
		m.advance_y = scale_y(_face->glyph->advance.y);

		FT_Outline outline = _face->glyph->outline;
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
		: _freetype(create_freetype())
	{	build_index();	}

	font::accessor_ptr font_loader::load(const agge::font_descriptor &descriptor)
	{
		auto m = _mapping.lower_bound(descriptor);

		if (m != _mapping.end())
		{
			if (m != _mapping.begin() && (m->first.family.size() != descriptor.family.size()
				|| !equal(m->first.family.begin(), m->first.family.end(), descriptor.family.begin(), nc_equal())))
			{
				--m;
			}
			return make_shared<wpl::font_accessor>(_freetype.get(), m->second.first.c_str(), m->second.second,
				descriptor.height, descriptor.hinting);
		}
		return nullptr;
	}

	void font_loader::build_index()
	{
		string family, path, styles;
		font_descriptor key = {};

		for (const auto e = create_fonts_enumerator(); e(path); )
		{
			auto index = 0;
			auto faces = 0;

			do
			{
				const auto face = open_face(_freetype.get(), path, index);

				faces = face->num_faces;
				key.family = face->family_name;
				key.weight = (styles = face->style_name, find_weight(styles));
				key.italic = !!(FT_STYLE_FLAG_ITALIC & face->style_flags);
				if (string::npos == styles.find("NARROW"))
					_mapping[key] = make_pair(path, index);
			} while (++index < faces);
		}
	}


	shared_ptr<gcontext::text_engine_type> create_text_engine()
	{
		auto tec = make_shared<text_engine_composite>();

		return shared_ptr<gcontext::text_engine_type>(tec, &tec->text_engine);
	}
}
