//	Copyright (c) 2011-2022 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <agge/path.h>
#include <agge/pod_vector.h>
#include <utility>

namespace wpl
{
	typedef agge::pod_vector< std::pair<agge::point_r, unsigned> > path_storage_t;

	template <typename IteratorT>
	class path_offset
	{
	public:
		path_offset(IteratorT &src, agge::real_t dx, agge::real_t dy)
			: _src(src), _dx(dx), _dy(dy)
		{	}

		void rewind(int index)
		{	_src.rewind(index);	}

		int vertex(agge::real_t *x, agge::real_t *y)
		{
			const auto cmd = _src.vertex(x, y);

			return *x += _dx, *y += _dy, cmd;
		}

	private:
		void operator =(const path_offset &rhs);

	private:
		IteratorT &_src;
		agge::real_t _dx, _dy;
	};

	class glyph : path_storage_t
	{
	public:
		glyph(const glyph &other)
			: _points(other._points), _bounds(other._bounds), _iterator(_points.begin())
		{	}

		glyph(const path_storage_t &from, const agge::rect_r &bounds)
			: _points(from), _bounds(bounds), _iterator(_points.begin())
		{	}

		void rewind(int /*index*/)
		{	_iterator = _points.begin();	}

		int vertex(agge::real_t *x, agge::real_t *y)
		{
			if (_points.end() == _iterator)
				return agge::path_command_stop;
			*x = _iterator->first.x, *y = _iterator->first.y;
			return _iterator++->second;
		}

		agge::rect_r bounds() const
		{	return _bounds;	}

	private:
		path_storage_t _points;
		agge::rect_r _bounds;
		path_storage_t::const_iterator _iterator;
	};

	struct glyphs
	{
		static glyph up(int font_height);
		static glyph down(int font_height);
	};


	template <typename IteratorT>
	inline path_offset<IteratorT> offset(IteratorT &src, agge::real_t dx, agge::real_t dy)
	{	return path_offset<IteratorT>(src, dx, dy);	}
}
