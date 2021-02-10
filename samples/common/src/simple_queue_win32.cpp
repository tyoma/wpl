#include "../simple_queue.h"

#include <tchar.h>
#include <windows.h>
#include <wpl/win32/helpers.h>

using namespace std;

namespace wpl
{
	struct simple_queue::impl
	{
		impl()
			: hwnd(::CreateWindowW(L"static", 0, WS_CHILD, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0))
		{	::SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&window_proc));	}

		static LRESULT CALLBACK window_proc(HWND hwnd_, UINT message, WPARAM wparam, LPARAM /*lparam*/)
		{
			switch (message)
			{
			case WM_TIMER:
				::KillTimer(hwnd_, wparam);

			case WM_USER:
				const unique_ptr<queue_task> task(reinterpret_cast<queue_task *>(wparam));

				(*task)();
			}
			return 0;
		}

		win32::helpers::window_handle hwnd;
	};

	simple_queue::simple_queue()
		: _impl(new impl)
	{	}

	simple_queue::~simple_queue()
	{	}

	bool simple_queue::schedule(const queue_task &task, timespan defer_by)
	{
		unique_ptr<queue_task> t(new queue_task(task));

		return (defer_by
			? ::SetTimer(_impl->hwnd, reinterpret_cast<UINT_PTR>(t.get()), static_cast<UINT>(defer_by), NULL)
			: ::PostMessage(_impl->hwnd, WM_USER, reinterpret_cast<WPARAM>(t.get()), 0)
		) ? t.release(), true : false;
	}
}
