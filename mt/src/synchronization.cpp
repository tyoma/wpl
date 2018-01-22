//	Copyright (c) 2011-2018 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../synchronization.h"

#include <windows.h>

namespace wpl
{
	namespace mt
	{
		event_flag::event_flag(bool raised, bool auto_reset)
			: _handle(::CreateEvent(NULL, auto_reset ? FALSE : TRUE, raised ? TRUE : FALSE, NULL))
		{	}

		event_flag::~event_flag()
		{	::CloseHandle(static_cast<HANDLE>(_handle));	}

		void event_flag::raise()
		{	::SetEvent(static_cast<HANDLE>(_handle));	}

		void event_flag::lower()
		{	::ResetEvent(static_cast<HANDLE>(_handle));	}

		event_flag::wait_status event_flag::wait(unsigned int to) volatile
		{	return WAIT_OBJECT_0 == ::WaitForSingleObject(static_cast<HANDLE>(_handle), to) ? satisfied : timeout;	}
	}
}
