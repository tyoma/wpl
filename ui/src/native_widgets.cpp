#include "native_widgets.h"

#include <olectl.h>

using namespace std;
using namespace std::tr1::placeholders;

namespace wpl
{
	namespace ui
	{
		native_button::native_button(HWND hwnd)
			: _window(window::attach(hwnd))
		{	_advisory = _window->advise(bind(&native_button::wndproc, this, _1, _2, _3, _4));	}

		void native_button::set_text(const wstring &text)
		{	::SetWindowTextW(_window->hwnd(), text.c_str());	}

		LRESULT native_button::wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
		{
			if (OCM_COMMAND == message)
			{
				clicked();
			}
			return previous(message, wparam, lparam);
		}
	}
}
