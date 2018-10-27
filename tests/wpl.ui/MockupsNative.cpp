#include "MockupsNative.h"

#include <tchar.h>
#include <windows.h>
#include <wpl/ui/win32/native_view.h>

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace mocks
			{
				native_control::native_control()
					: _hwnd(::CreateWindow(_T("static"), NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, NULL, NULL))
				{	}

				native_control::~native_control()
				{	::DestroyWindow(_hwnd);	}

				void native_control::resize(unsigned cx, unsigned cy, positioned_native_views &native_views)
				{
					view_location l = { 0, 0, cx, cy };

					native_views.push_back(positioned_native_view(*this, l));
				}
				
				HWND native_control::get_window() const throw()
				{	return _hwnd;	}
			}
		}
	}
}
