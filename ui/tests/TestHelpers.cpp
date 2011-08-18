#include "TestHelpers.h"

#include <windows.h>
#include <commctrl.h>
#include <vcclr.h>
#include <algorithm>

using namespace std;
using namespace System;
using namespace System::IO;
using namespace System::Globalization;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace ut
{
	wstring make_native(String ^managed_string)
	{
		pin_ptr<const wchar_t> buffer(PtrToStringChars(managed_string));

		return wstring(buffer);
	}

	String ^make_managed(const wstring &native_string)
	{	return gcnew String(native_string.c_str());	}

	WindowTestsBase::WindowTestsBase()
		: _windows(new vector<void *>())
	{	}

	WindowTestsBase::~WindowTestsBase()
	{	delete _windows;	}

	void *WindowTestsBase::create_window()
	{	return create_window(_T("static"), 0, WS_POPUP | WS_VISIBLE, 0);	}

	void *WindowTestsBase::create_window(const TCHAR *class_name)
	{	return create_window(class_name, 0, WS_POPUP, 0);	}

	void *WindowTestsBase::create_window(const TCHAR *class_name, void *parent, unsigned int style, unsigned int exstyle)
	{
		HWND hwnd = ::CreateWindowEx(exstyle, class_name, NULL, style, 0, 0, 50, 50, reinterpret_cast<HWND>(parent), NULL, NULL, NULL);

		_windows->push_back(hwnd);
		return hwnd;
	}

	void WindowTestsBase::destroy_window(void *hwnd)
	{
		_windows->erase(remove(_windows->begin(), _windows->end(), hwnd), _windows->end());
		::DestroyWindow(reinterpret_cast<HWND>(hwnd));
	}

	void WindowTestsBase::cleanup()
	{
		for (vector<void *>::const_iterator i = _windows->begin(); i != _windows->end(); ++i)
			if (::IsWindow(reinterpret_cast<HWND>(*i)))
				::DestroyWindow(reinterpret_cast<HWND>(*i));
	}

	void WindowTestsBase::init_commctrl(TestContext ^/*context*/)
	{
		INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };

		::InitCommonControlsEx(&icc);
	}
}
