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

#include <exception>

namespace wpl
{
	struct unit_not_supported : std::exception
	{
	};

	template <typename ResultT = void>
	struct unit_visitor
	{
		typedef ResultT result_type;

		template <typename T>
		ResultT visit_pixel(T) const;
		template <typename T>
		ResultT visit_percent(T) const;
		template <typename T>
		ResultT visit_em(T) const;
	};



	template <typename ResultT>
	template <typename T>
	inline ResultT unit_visitor<ResultT>::visit_pixel(T) const
	{	throw unit_not_supported();	}

	template <typename ResultT>
	template <typename T>
	inline ResultT unit_visitor<ResultT>::visit_percent(T) const
	{	throw unit_not_supported();	}

	template <typename ResultT>
	template <typename T>
	inline ResultT unit_visitor<ResultT>::visit_em(T) const
	{	throw unit_not_supported();	}
}
