#include <synchronization.h>

#include <windows.h>

namespace wpl
{
	namespace mt
	{
		event_flag::event_flag(bool raised, bool auto_reset)
			: _handle(::CreateEvent(NULL, auto_reset ? FALSE : TRUE, raised ? TRUE : FALSE, NULL))
		{	}

		event_flag::~event_flag()
		{	::CloseHandle(reinterpret_cast<HANDLE>(_handle));	}

		void event_flag::raise()
		{	::SetEvent(reinterpret_cast<HANDLE>(_handle));	}

		void event_flag::lower()
		{	::ResetEvent(reinterpret_cast<HANDLE>(_handle));	}

		event_flag::wait_status event_flag::wait(unsigned int to) volatile
		{	return WAIT_OBJECT_0 == ::WaitForSingleObject(reinterpret_cast<HANDLE>(_handle), to) ? satisfied : timeout;	}
	}
}
