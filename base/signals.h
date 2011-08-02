//	Copyright (C) 2011 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <functional>
#include <vector>

namespace std
{
	using namespace tr1;
}

namespace wpl
{
	template <typename F>
	class signal
	{
	};

	template <typename R>
	class signal<R ()> : signal< std::function<void ()> >
	{
	public:
		void operator ()() const
		{	}
	};

	template <typename R, typename T1>
	class signal<R (T1)> : signal< std::function<void (T1)> >
	{
	public:
		void operator ()(T1 /*arg1*/) const
		{	}
	};

	template <typename R, typename T1, typename T2>
	class signal<R (T1, T2)> : signal< std::function<void (T1, T2)> >
	{
	public:
		void operator ()(T1 /*arg1*/, T2 /*arg2*/) const
		{	}
	};

	template <typename R, typename T1, typename T2, typename T3>
	class signal<R (T1, T2, T3)> : signal< std::function<void (T1, T2, T3)> >
	{
	public:
		void operator ()(T1 /*arg1*/, T2 /*arg2*/, T3 /*arg3*/) const
		{	}
	};

	template <typename R, typename T1, typename T2, typename T3, typename T4>
	class signal<R (T1, T2, T3, T4)> : signal< std::function<void (T1, T2, T3, T4)> >
	{
	public:
		void operator ()(T1 /*arg1*/, T2 /*arg2*/, T3 /*arg3*/, T4 /*arg4*/) const
		{	}
	};

	template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
	class signal<R (T1, T2, T3, T4, T5)> : signal< std::function<void (T1, T2, T3, T4, T5)> >
	{
	public:
		void operator ()(T1 /*arg1*/, T2 /*arg2*/, T3 /*arg3*/, T4 /*arg4*/, T5 /*arg5*/) const
		{	}
	};
}
