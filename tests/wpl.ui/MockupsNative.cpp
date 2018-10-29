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
				native_view_window::native_view_window()
					: _hwnd(::CreateWindow(_T("static"), NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, NULL, NULL))
				{	}

				native_view_window::~native_view_window()
				{	::DestroyWindow(_hwnd);	}

				HWND native_view_window::get_window() const throw()
				{	return _hwnd;	}


				native_view::native_view()
					: container_was_dirty(false)
				{	}

				void native_view::resize(unsigned /*cx*/, unsigned /*cy*/, positioned_native_views &nviews)
				{
					container_was_dirty = container_was_dirty || !nviews.empty();
					for (response_t::const_iterator i = response.begin(); i != response.end(); ++i)
						nviews.push_back(visual::positioned_native_view(*i->first, i->second));
				}
			}
		}
	}
}
