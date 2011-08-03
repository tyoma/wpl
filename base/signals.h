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
#include <memory>
#include <vector>

namespace std
{
	using tr1::function;
	using tr1::shared_ptr;
}

namespace wpl
{
	template <typename F>
	class signal
	{
	protected:
		typedef std::vector<F> _slots_t;
		typedef std::shared_ptr< _slots_t > _slots_ptr_t;
		typedef typename _slots_t::const_iterator _slots_iterator_t;

		const _slots_ptr_t _slots;

		signal();

	public:
		void operator +=(const F &slot);
	};

	template <typename R>
	struct signal<R ()> : signal< std::function<void ()> >
	{
		void operator ()() const;
	};

	template <typename R, typename T1>
	struct signal<R (T1)> : signal< std::function<void (T1)> >
	{
		void operator ()(T1 arg1) const;
	};

	template <typename R, typename T1, typename T2>
	struct signal<R (T1, T2)> : signal< std::function<void (T1, T2)> >
	{
		void operator ()(T1 arg1, T2 arg2) const;
	};

	template <typename R, typename T1, typename T2, typename T3>
	struct signal<R (T1, T2, T3)> : signal< std::function<void (T1, T2, T3)> >
	{
		void operator ()(T1 arg1, T2 arg2, T3 arg3) const;
	};

	template <typename R, typename T1, typename T2, typename T3, typename T4>
	struct signal<R (T1, T2, T3, T4)> : signal< std::function<void (T1, T2, T3, T4)> >
	{
		void operator ()(T1 arg1, T2 arg2, T3 arg3, T4 arg4) const;
	};

	template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
	struct signal<R (T1, T2, T3, T4, T5)> : signal< std::function<void (T1, T2, T3, T4, T5)> >
	{
		void operator ()(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5) const;
	};


	template <typename F>
	inline signal<F>::signal()
		: _slots(new _slots_t())
	{	}

	template <typename F>
	inline void signal<F>::operator +=(const F &slot)
	{	_slots->push_back(slot);	}


	template <typename R>
	inline void signal<R ()>::operator ()() const
	{	for (_slots_iterator_t i = _slots->begin(); i != _slots->end(); ++i)	(*i)();	}

	template <typename R, typename T1>
	inline void signal<R (T1)>::operator ()(T1 arg1) const
	{	for (_slots_iterator_t i = _slots->begin(); i != _slots->end(); ++i)	(*i)(arg1);	}

	template <typename R, typename T1, typename T2>
	inline void signal<R (T1, T2)>::operator ()(T1 arg1, T2 arg2) const
	{	for (_slots_iterator_t i = _slots->begin(); i != _slots->end(); ++i)	(*i)(arg1, arg2);	}

	template <typename R, typename T1, typename T2, typename T3>
	inline void signal<R (T1, T2, T3)>::operator ()(T1 arg1, T2 arg2, T3 arg3) const
	{	for (_slots_iterator_t i = _slots->begin(); i != _slots->end(); ++i)	(*i)(arg1, arg2, arg3);	}

	template <typename R, typename T1, typename T2, typename T3, typename T4>
	inline void signal<R (T1, T2, T3, T4)>::operator ()(T1 arg1, T2 arg2, T3 arg3, T4 arg4) const
	{	for (_slots_iterator_t i = _slots->begin(); i != _slots->end(); ++i)	(*i)(arg1, arg2, arg3, arg4);	}

	template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
	inline void signal<R (T1, T2, T3, T4, T5)>::operator ()(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5) const
	{	for (_slots_iterator_t i = _slots->begin(); i != _slots->end(); ++i)	(*i)(arg1, arg2, arg3, arg4, arg5);	}
}
