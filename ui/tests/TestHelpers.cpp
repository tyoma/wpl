#include "TestHelpers.h"

#include <wpl/ui/win32/window.h>

#include <windows.h>
#include <commctrl.h>
#include <olectl.h>
#include <vcclr.h>
#include <algorithm>

using namespace std;
using namespace System;
using namespace System::IO;
using namespace System::Globalization;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace ut
{
	namespace
	{
		LRESULT reflection_wndproc(UINT message, WPARAM wparam, LPARAM lparam, const wpl::ui::window::original_handler_t &previous)
		{
			if (WM_NOTIFY == message)
				return ::SendMessage(reinterpret_cast<NMHDR *>(lparam)->hwndFrom, OCM_NOTIFY, wparam, lparam);
			else
				return previous(message, wparam, lparam);
		}

		BOOL CALLBACK EnumThreadWndProc(HWND hwnd, LPARAM lParam)
		{
			set<HWND> &s(*reinterpret_cast<set<HWND> *>(lParam));

			s.insert(hwnd);
			return TRUE;
		}
	}

	wstring make_native(String ^managed_string)
	{
		pin_ptr<const wchar_t> buffer(PtrToStringChars(managed_string));

		return wstring(buffer);
	}

	String ^make_managed(const wstring &native_string)
	{	return gcnew String(native_string.c_str());	}

	set<HWND> enum_thread_windows()
	{
		set<HWND> result;

		::EnumThreadWindows(::GetCurrentThreadId(), &EnumThreadWndProc, reinterpret_cast<LPARAM>(&result));
		return result;
	}

	RECT get_window_rect(HWND hwnd)
	{
		RECT rc = { 0 };
		HWND hparent = ::GetParent(hwnd);

		::GetWindowRect(hwnd, &rc);
		if (hparent)
			::MapWindowPoints(NULL, hparent, reinterpret_cast<POINT *>(&rc), 2);
		return rc;
	}



	WindowTestsBase::WindowTestsBase()
		: _windows(new vector<HWND>()), _connections(new vector< shared_ptr<wpl::destructible> >)
	{	}

	WindowTestsBase::~WindowTestsBase()
	{
		delete _connections;
		delete _windows;
	}

	HWND WindowTestsBase::create_window()
	{	return create_window(_T("static"), 0, WS_POPUP, 0);	}

	HWND WindowTestsBase::create_visible_window()
	{	return create_window(_T("static"), 0, WS_POPUP | WS_VISIBLE, 0);	}

	HWND WindowTestsBase::create_window(const TCHAR *class_name)
	{	return create_window(class_name, 0, WS_POPUP, 0);	}

	HWND WindowTestsBase::create_window(const TCHAR *class_name, HWND parent, unsigned int style, unsigned int exstyle)
	{
		HWND hwnd = ::CreateWindowEx(exstyle, class_name, NULL, style, 0, 0, 50, 50, parent, NULL, NULL, NULL);

		_windows->push_back(hwnd);
		return hwnd;
	}

	void WindowTestsBase::enable_reflection(HWND hwnd)
	{	_connections->push_back(wpl::ui::window::attach(hwnd)->advise(&reflection_wndproc));	}

	void WindowTestsBase::destroy_window(HWND hwnd)
	{
		_windows->erase(remove(_windows->begin(), _windows->end(), hwnd), _windows->end());
		::DestroyWindow(hwnd);
	}

	void WindowTestsBase::cleanup()
	{
		for (vector<HWND>::const_iterator i = _windows->begin(); i != _windows->end(); ++i)
			if (::IsWindow(*i))
				::DestroyWindow(*i);
		_connections->clear();
	}

	void WindowTestsBase::init_commctrl(TestContext ^/*context*/)
	{
		INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };

		::InitCommonControlsEx(&icc);
	}
}
