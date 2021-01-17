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

#pragma once

#include <agge/types.h>

namespace wpl
{
	typedef agge::point<int> point_i;
	typedef agge::point<agge::real_t> point_r;
	typedef agge::rect<int> rect_i;
	typedef agge::rect<agge::real_t> rect_r;

	class display_unit
	{
	public:
		enum unit {	px, percent, em,	};

	public:
		display_unit();
		display_unit(double value, unit unit_);

		template <typename VisitorT>
		void apply(VisitorT &visitor) const;
		template <typename VisitorT>
		void apply(VisitorT &visitor);

	private:
		double _value;
		unit _unit;
	};



	inline display_unit::display_unit(double value, unit unit_)
		: _value(value), _unit(unit_)
	{	}

	template <typename VisitorT>
	inline void display_unit::apply(VisitorT &visitor) const
	{
		switch (_unit)
		{
		case px:	visitor.visit_pixel(_value);	break;
		case percent:	visitor.visit_percent(_value);	break;
		case em:	visitor.visit_em(_value);	break;
		}
	}

	template <typename VisitorT>
	inline void display_unit::apply(VisitorT &visitor)
	{
		switch (_unit)
		{
		case px:	visitor.visit_pixel(_value);	break;
		case percent:	visitor.visit_percent(_value);	break;
		case em:	visitor.visit_em(_value);	break;
		}
	}


	inline display_unit pixels(double value)
	{	return display_unit(value, display_unit::px);	}

	inline display_unit percents(double value)
	{	return display_unit(value, display_unit::percent);	}
}
