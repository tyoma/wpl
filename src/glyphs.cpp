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

#include "glyphs.h"

#include <agge/vertex_sequence.h>
#include <agge/stroke.h>
#include <agge/stroke_features.h>
#include <wpl/concepts.h>
#include <wpl/helpers.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace
	{
		template <typename T>
		T absolute(T value)
		{	return value < T() ? -value : value;	}

		pair<point_r, int> make_vertex(real_t x, real_t y, int command)
		{
			point_r p = {	x, y	};
			return make_pair(p, command);
		}

		class glyph_writer : noncopyable
		{
		public:
			glyph_writer(path_storage_t &storage, rect_r &bounds)
				: _storage(storage), _bounds(bounds)
			{	}

			void move_to(real_t x, real_t y)
			{
				_storage.push_back(make_vertex(x, y, path_command_move_to));
				extend_bounds(x, y);
			}

			void line_to(real_t x, real_t y)
			{
				_storage.push_back(make_vertex(x, y, path_command_line_to));
				extend_bounds(x, y);
			}

			void close_polygon()
			{	_storage.push_back(make_vertex(0.0f, 0.0f, path_flag_close));	}

		private:
			void extend_bounds(real_t x, real_t y)
			{
				if (x < _bounds.x1) _bounds.x1 = x;
				if (x > _bounds.x2) _bounds.x2 = x;
				if (y < _bounds.y1) _bounds.y1 = y;
				if (y > _bounds.y2) _bounds.y2 = y;
			}

		private:
			path_storage_t &_storage;
			rect_r &_bounds;
		};

		glyph build_glyph(const path_storage_t &from, real_t stroke_width)
		{
			stroke s;
			auto r = create_rect<real_t>(30000.0f, 30000.0f, -30000.0f, -30000.0f);
			glyph src(from, r);
			path_storage_t dest;
			glyph_writer w(dest, r);

			s.width(stroke_width);
			s.set_cap(caps::round());
			s.set_join(joins::bevel());
			add_path(w, assist(src, s));
			return glyph(dest, r);
		}
	}

	glyph glyphs::up(int font_height_)
	{
		const auto font_height = static_cast<real_t>(absolute(font_height_));

		path_storage_t points;
		const auto dx = 0.2f * font_height;
		const auto dy = 0.6f * dx;

		points.push_back(make_vertex(-dx, dy, path_command_move_to));
		points.push_back(make_vertex(0.0f, -dy, path_command_line_to));
		points.push_back(make_vertex(dx, dy, path_command_line_to));
		return build_glyph(points, 0.15f * font_height);
	}

	glyph glyphs::down(int font_height_)
	{
		const auto font_height = static_cast<real_t>(absolute(font_height_));

		path_storage_t points;
		const auto dx = 0.2f * font_height;
		const auto dy = 0.6f * dx;

		points.push_back(make_vertex(-dx, -dy, path_command_move_to));
		points.push_back(make_vertex(0.0f, dy, path_command_line_to));
		points.push_back(make_vertex(dx, -dy, path_command_line_to));
		return build_glyph(points, 0.15f * font_height);
	}
}
