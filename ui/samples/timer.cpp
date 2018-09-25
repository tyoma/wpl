#include "timer.h"

#include <tchar.h>
#include <windows.h>

using namespace std;

namespace wpl
{
	namespace
	{
		class timer
		{
		public:
			timer(unsigned timeout, const function<void(unsigned elapsed)> &callback)
				: _callback(callback), _start_time(::GetTickCount()),
					_window(::CreateWindow(_T("static"), 0, WS_CHILD, 0, 0, 1, 1, HWND_MESSAGE, 0, 0, 0), &::DestroyWindow)					
			{	::SetTimer(static_cast<HWND>(_window.get()), reinterpret_cast<UINT_PTR>(this), timeout, &timer_proc);	}

			~timer()
			{	::KillTimer(static_cast<HWND>(_window.get()), reinterpret_cast<UINT_PTR>(this));	}

		private:
			static void CALLBACK timer_proc(HWND /*hwnd*/, UINT /*message*/, UINT_PTR self_, DWORD elapsed)
			{
				timer *self = reinterpret_cast<timer *>(self_);

				self->_callback(elapsed - self->_start_time);
			}

		private:
			function<void(unsigned elapsed)> _callback;
			unsigned _start_time;
			shared_ptr<void> _window;
		};
	}

	shared_ptr<void> create_timer(unsigned timeout, const function<void(unsigned elapsed)> &callback)
	{	return shared_ptr<void>(new timer(timeout, callback));	}
}
