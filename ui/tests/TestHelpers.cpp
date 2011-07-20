#include "TestHelpers.h"

#include <windows.h>
#include <vcclr.h>

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
	{	return create_window(_T("static"));	}

	void *WindowTestsBase::create_window(const TCHAR *class_name)
	{
		HWND hwnd = ::CreateWindow(class_name, NULL, WS_POPUP, 0, 0, 1, 1, NULL, NULL, NULL, NULL);

		_windows->push_back(hwnd);
		return hwnd;
	}

	void *WindowTestsBase::create_tree()
	{
		HWND hparent = ::CreateWindow(_T("static"), NULL, WS_POPUP, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
		HWND htree = ::CreateWindow(_T("systreeview32"), NULL, WS_CHILD | WS_VISIBLE, 0, 0, 1, 1, hparent, NULL, NULL, NULL);

		_windows->push_back(htree);
		_windows->push_back(hparent);
		return htree;
	}

	void WindowTestsBase::cleanup()
	{
		for (vector<void *>::const_iterator i = _windows->begin(); i != _windows->end(); ++i)
			if (::IsWindow(reinterpret_cast<HWND>(*i)))
				::DestroyWindow(reinterpret_cast<HWND>(*i));
	}
}
