//	Copyright (c) 2011-2014 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../thread.h"
#include "../synchronization.h"

#include <process.h>
#include <windows.h>
#include <stdexcept>
#include <memory>

using namespace std;

namespace wpl
{
	namespace mt
	{
		thread::thread(const action &job)
		{
			void * const invalid_handle_value = reinterpret_cast<void *>(-1);

			struct thread_proxy
			{
				static unsigned int __stdcall thread_function(void *f_)
				{
					auto_ptr<thread::action> f(static_cast<thread::action *>(f_));

					(*f)();
					return 0;
				}
			};

			auto_ptr<action> f(new action(job));

			_thread.reset(reinterpret_cast<HANDLE>(_beginthreadex(0, 0, &thread_proxy::thread_function, f.get(), 0, &_id)),
				::CloseHandle);
			if (invalid_handle_value != _thread.get())
				f.release();
			else
				throw runtime_error("New thread cannot be started!");
		}

		thread::~thread() throw()
		{
			if (_thread)
				join();
		}

		void thread::join()
		{	::WaitForSingleObject(static_cast<HANDLE>(_thread.get()), INFINITE);	}

		void thread::detach()
		{	_thread.reset();	}

		auto_ptr<thread> thread::run(const action &initializer, const action &job)
		{
			struct initialized_run
			{
				const thread::action *initializer, *job;
				event_flag *initialized_flag;

				void operator ()() const
				{
					(*initializer)();
					thread::action job(*this->job);
					initialized_flag->raise();
					job();
				}
			};

			event_flag initialized(false, false);
			initialized_run r = {	&initializer, &job, &initialized	};
			auto_ptr<thread> t(new thread(r));

			initialized.wait();
			return t;
		}


		tls_base::tls_base()
			: _index(::TlsAlloc())
		{	}

		tls_base::~tls_base()
		{	::TlsFree(_index);	}

		void *tls_base::get() const
		{	return ::TlsGetValue(_index);	}

		void tls_base::set(const void *value)
		{	::TlsSetValue(_index, const_cast<void *>(value)); }
	}
}
