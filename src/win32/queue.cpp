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

#include <wpl/win32/queue.h>

#include <tchar.h>
#include <windows.h>

using namespace std;

namespace wpl
{
	namespace win32
	{
		LARGE_INTEGER g_pq_frequency;
		const long long c_pq_frequency = (::QueryPerformanceFrequency(&g_pq_frequency), g_pq_frequency.QuadPart);

		namespace
		{
			HWND hndl(const shared_ptr<void> &h)
			{	return static_cast<HWND>(h.get());	}

			LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
			{
				if (WM_USER == message)
					(*unique_ptr<queue_task>(reinterpret_cast<queue_task *>(lparam)))();
				return ::DefWindowProc(hwnd, message, wparam, lparam);
			}

			void CALLBACK timer_proc(HWND hwnd, UINT, UINT_PTR id, DWORD /*dwtime*/)
			{
				::KillTimer(hwnd, id);
				(*unique_ptr<queue_task>(reinterpret_cast<queue_task *>(id)))();
			}
		}

		timestamp clock()
		{
			LARGE_INTEGER v;
			::QueryPerformanceCounter(&v);
			return 1000 * v.QuadPart / c_pq_frequency;
		}


		queue::queue()
			: _hwnd(::CreateWindow(_T("static"), 0, WS_CHILD, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0), ::DestroyWindow)
		{	::SetWindowLongPtr(hndl(_hwnd), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&wnd_proc));	}

		bool queue::operator ()(const queue_task &task, timespan defer_by)
		{
			unique_ptr<queue_task> t(new queue_task(task));

			return (defer_by
				? ::SetTimer(hndl(_hwnd), reinterpret_cast<LPARAM>(t.get()), static_cast<UINT>(defer_by), &timer_proc)
				: ::PostMessageA(hndl(_hwnd), WM_USER, 0, reinterpret_cast<LPARAM>(t.get()))
			) ? t.release(), true : false;
		}
	}
}
