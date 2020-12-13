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

#include "concepts.h"

#include <functional>
#include <list>
#include <memory>

namespace wpl
{
	typedef std::shared_ptr<void> slot_connection;

	template <typename F>
	class signal
	{
	public:
		signal();
		signal(const signal &other);

		signal &operator =(const signal &rhs);
		slot_connection operator +=(const F &slot);

	protected:
		template <typename InvokerT>
		void for_each_invoke(const InvokerT &invoker) const;

	private:
		struct cleanup_lock;
		struct function_list;
		typedef F function_t;

	private:
		std::shared_ptr<function_list> _function_list;
	};

	template <typename F>
	struct signal<F>::cleanup_lock
	{
		cleanup_lock(const std::shared_ptr<function_list> &list_);
		~cleanup_lock();

		std::shared_ptr<function_list> list;
	};

	template <typename F>
	struct signal<F>::function_list
	{
		function_list();

		std::list<F> functions;
		bool traversing, require_cleanup;
	};



	template <typename F>
	inline signal<F>::signal()
		: _function_list(std::make_shared<function_list>())
	{	}

	template <typename F>
	inline signal<F>::signal(const signal &/*other*/)
		: _function_list(std::make_shared<function_list>())
	{	}

	template <typename F>
	inline signal<F> &signal<F>::operator =(const signal &/*rhs*/)
	{	return *this;	}

	template <typename F>
	inline slot_connection signal<F>::operator +=(const F &slot)
	{
		const auto l = _function_list;
		const auto i = l->functions.insert(l->functions.end(), slot);

		return slot_connection(&*i, [l, i] (void *) {
			if (l->traversing)
				*i = F(), l->require_cleanup = true;
			else
				l->functions.erase(i);
		});
	}

	template <typename F>
	template <typename InvokerT>
	inline void signal<F>::for_each_invoke(const InvokerT &invoker) const
	{
		cleanup_lock l(_function_list);

		for (auto i = l.list->functions.begin(); i != l.list->functions.end(); ++i)
		{
			if (*i)
				invoker(*i);
		}
	}


	template <typename F>
	inline signal<F>::cleanup_lock::cleanup_lock(const std::shared_ptr<function_list> &list_)
		: list(list_)
	{	list->traversing = true;	}

	template <typename F>
	inline signal<F>::cleanup_lock::~cleanup_lock()
	{
		if (list->require_cleanup)
		{
			list->functions.remove_if([] (const function_t &f) {	return !!f;	});
			list->require_cleanup = false;
		}
		list->traversing = false;
	}


	template <typename F>
	inline signal<F>::function_list::function_list()
		: traversing(false), require_cleanup(false)
	{	}



	template <>
	struct signal<void ()> : signal< std::function<void ()> >
	{
		void operator ()() const
		{	this->for_each_invoke([] (const std::function<void ()> &f) {	f();	});	}
	};

	template <typename T1>
	struct signal<void (T1)> : signal< std::function<void (T1)> >
	{
		void operator ()(T1 arg1) const
		{	this->for_each_invoke([&] (const std::function<void (T1)> &f) {	f(arg1);	});	}
	};

	template <typename T1, typename T2>
	struct signal<void (T1, T2)> : signal< std::function<void (T1, T2)> >
	{
		void operator ()(T1 arg1, T2 arg2) const
		{	this->for_each_invoke([&] (const std::function<void (T1, T2)> &f) {	f(arg1, arg2);	});	}
	};

	template <typename T1, typename T2, typename T3>
	struct signal<void (T1, T2, T3)> : signal< std::function<void (T1, T2, T3)> >
	{
		void operator ()(T1 arg1, T2 arg2, T3 arg3) const
		{	this->for_each_invoke([&] (const std::function<void (T1, T2, T3)> &f) {	f(arg1, arg2, arg3);	});	}
	};

	template <typename T1, typename T2, typename T3, typename T4>
	struct signal<void (T1, T2, T3, T4)> : signal< std::function<void (T1, T2, T3, T4)> >
	{
		void operator ()(T1 arg1, T2 arg2, T3 arg3, T4 arg4) const
		{	this->for_each_invoke([&] (const std::function<void (T1, T2, T3, T4)> &f) {	f(arg1, arg2, arg3, arg4);	});	}
	};

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	struct signal<void (T1, T2, T3, T4, T5)> : signal< std::function<void (T1, T2, T3, T4, T5)> >
	{
		void operator ()(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5) const
		{	this->for_each_invoke([&] (const std::function<void (T1, T2, T3, T4, T5)> &f) {	f(arg1, arg2, arg3, arg4, arg5);	});	}
	};
}
