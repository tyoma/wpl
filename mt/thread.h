//	Copyright (C) 2011-2012 by Artem A. Gevorkyan (gevorkyan.org)
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

namespace std
{
	using tr1::function;
	using tr1::shared_ptr;
}

namespace wpl
{
	namespace mt
	{
		class thread
		{
		public:
			typedef std::function<void()> action;
			typedef unsigned int id;

		public:
			explicit thread(const action &job);
			virtual ~thread() throw();

			void join();
			void detach();

			static std::auto_ptr<thread> run(const action &initializer, const action &job);

			id get_id() const throw();

		private:
			id _id;
			std::shared_ptr<void> _thread;
		};


		class tls_base
		{
			unsigned int _index;

			tls_base(const tls_base &other);
			const tls_base &operator =(const tls_base &rhs);

		protected:
			tls_base();
			~tls_base();

			void *get() const;
			void set(const void *value);
		};

		template <typename T>
		struct tls : private tls_base
		{
			T *get() const;
			void set(T *value);
		};


		// thread - inline definitions
		inline thread::id thread::get_id() const throw()
		{	return _id;	}


		// tls<T> - inline definitions
		template <typename T>
		inline T *tls<T>::get() const
		{	return static_cast<T *>(tls_base::get());	}

		template <typename T>
		inline void tls<T>::set(T *value)
		{	tls_base::set(value);	}
	}
}
