#include "MockupsNative.h"

#include <tchar.h>
#include <windows.h>
#include <wpl/win32/native_view.h>
#include <ut/assert.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			native_view_window::native_view_window()
				: native_view(""), _hparent(0), _hwnd(0)
			{	}

			native_view_window::~native_view_window()
			{	::DestroyWindow(_hwnd);	}

			HWND native_view_window::materialize(HWND hparent_for)
			{
				if (hparent_for != _hparent)
				{
					::DestroyWindow(_hwnd);
					return _hwnd = ::CreateWindow(_T("static"), NULL, WS_CHILD, 0, 0, 0, 0, hparent_for, NULL, NULL, 0);
				}
				return _hwnd;
			}

			LRESULT native_view_window::on_message(UINT message, WPARAM wparam, LPARAM lparam,
				const win32::window::original_handler_t &handler)
			{	return handler(message, wparam, lparam);	}
		}
	}
}
