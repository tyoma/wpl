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

#include "static_visitor.h"

#include <algorithm>

namespace wpl
{
	template <typename T>
	class fixed_size_calculator : public unit_visitor<int>
	{
	public:
		fixed_size_calculator(T min_size_func)
			: _min_size_func(min_size_func)
		{	}

		int visit_pixel(double value) const {	return (std::max)(_min_size_func(), static_cast<int>(value));	}
		int visit_percent(double /*value*/) const {	return 0;	}

	private:
		void operator =(const fixed_size_calculator &rhs);

	private:
		T _min_size_func;
	};

	template <typename T>
	class size_calculator : public unit_visitor<int>
	{
	public:
		size_calculator(double remainder, double &correction, T min_size_func)
			: _remainder(remainder), _correction(correction), _min_size_func(min_size_func)
		{	}

		int visit_pixel(double value) const
		{	return (std::max)(_min_size_func(), agge::iround(static_cast<agge::real_t>(value)));	}

		int visit_percent(double value) const
		{
			const auto fsize = agge::agge_max(0.01 * value * _remainder, 0.0);
			const auto ivalue = agge::iround(static_cast<agge::real_t>(fsize + _correction));

			_correction += fsize - ivalue;
			return ivalue;
		}

	private:
		void operator =(const size_calculator &rhs);

	private:
		const double _remainder;
		double &_correction;
		T _min_size_func;
	};


	template <typename T>
	inline fixed_size_calculator<T> fixed_size(T min_size_func)
	{	return fixed_size_calculator<T>(min_size_func);	}

	template <typename T>
	inline size_calculator<T> calculate_size(double remainder, double &correction, T min_size_func)
	{	return size_calculator<T>(remainder, correction, min_size_func);	}

	template <typename CallbackT, typename ContainerT, typename GetSizeT, typename MinSizeT>
	inline int enumerate_sizes(CallbackT size_callback, const ContainerT &entries, int shared_size,
		GetSizeT get_size, MinSizeT min_size)
	{
		auto correction = 0.0;

		for (auto i = entries.begin(); i != entries.end(); ++i)
			shared_size -= get_size(*i).apply(fixed_size([min_size, i] {	return min_size(*i);	}));
		for (auto j = entries.begin(); j != entries.end(); )
		{
			const auto i = j++;
			const auto item_size = get_size(*i).apply(calculate_size(shared_size, correction, [min_size, i] {
				return min_size(*i);
			}));

			size_callback(*i, item_size, j != entries.end() ? &*j : nullptr);
		}
		return shared_size;
	}
}
