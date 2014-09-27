#include "TestHelpers.h"

#include <wpl/ui/win32/window.h>

#include <algorithm>
#include <commctrl.h>
#include <olectl.h>
#include <tchar.h>
#include <vcclr.h>
#include <windows.h>

using namespace std;
using namespace System;
using namespace System::IO;
using namespace System::Globalization;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace ut
{
	namespace
	{
		typedef basic_string<TCHAR> tstring;
#ifdef UNICODE
		wstring t2w(const wstring &text)
		{	return text;	}

		wstring w2t(const wstring &text)
		{	return text;	}
#else 
		wstring t2w(const string &text);
		{
			vector<wchar_t> wtext(mbstowcs(NULL, text.c_str(), text.size()) + 1);

			mbstowcs(&wtext[0], text.c_str(), text.size());
			return &wtext[0];
		}

		string w2t(const wstring &text)
		{
			vector<char> mbtext(wcstombs(NULL, text.c_str(), text.size()) + 1);

			wcstombs(&mbtext[0], text.c_str(), text.size());
			return string(mbtext.begin(), mbtext.end());
		}
#endif

		LRESULT reflection_wndproc(UINT message, WPARAM wparam, LPARAM lparam, const wpl::ui::window::original_handler_t &previous)
		{
			return WM_NOTIFY == message ?
				::SendMessage(reinterpret_cast<NMHDR *>(lparam)->hwndFrom, OCM_NOTIFY, wparam, lparam)
				: previous(message, wparam, lparam);
		}
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

	RECT rect(int left, int top, int width, int height)
	{
		RECT rc = { left, top, left + width, top + height };

		return rc;
	}

	wstring get_window_text(HWND hwnd)
	{
		vector<TCHAR> text(::GetWindowTextLength(hwnd) + 1);

		::GetWindowText(hwnd, &text[0], text.size());
		return t2w(tstring(&text[0]));
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

	HWND WindowTestsBase::create_window(const wstring &class_name)
	{	return create_window(class_name, 0, WS_POPUP, 0);	}

	HWND WindowTestsBase::create_window(const wstring &class_name, HWND parent, unsigned int style, unsigned int exstyle)
	{
		HWND hwnd = ::CreateWindowEx(exstyle, w2t(class_name).c_str(), NULL, style, 0, 0, 50, 50, parent, NULL, NULL, NULL);

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


	window_tracker::window_tracker(const wstring &allow, const wstring &prohibit)
		: _allow(allow), _prohibit(prohibit)
	{
		checkpoint();
		created.clear();
	}

	void window_tracker::checkpoint()
	{
		struct WindowEnumData
		{
			basic_string<TCHAR> allowed, prohibited;
			set<HWND> windows;

			static BOOL CALLBACK enum_windows_callback(HWND hwnd, LPARAM lParam)
			{
				TCHAR classname[100] = { 0 };
				WindowEnumData &data = *reinterpret_cast<WindowEnumData *>(lParam);

				::GetClassName(hwnd, classname, _countof(classname));
				if ((data.allowed.empty() || 0 == _tcsicmp(data.allowed.c_str(), classname))
						&& (data.prohibited.empty() || 0 != _tcsicmp(data.prohibited.c_str(), classname)))
					data.windows.insert(hwnd);
				::EnumChildWindows(hwnd, &enum_windows_callback, lParam);
				return TRUE;
			}
		} data = { w2t(_allow), w2t(_prohibit) };

		::EnumThreadWindows(::GetCurrentThreadId(), &WindowEnumData::enum_windows_callback, reinterpret_cast<LPARAM>(&data));
		set_difference(data.windows.begin(), data.windows.end(), _windows.begin(), _windows.end(), back_inserter(created));
		set_difference(_windows.begin(), _windows.end(), data.windows.begin(), data.windows.end(), back_inserter(destroyed));
		_windows = data.windows;
	}
}

bool operator ==(const RECT &lhs, const RECT &rhs)
{
	return lhs.left == rhs.left && lhs.top == rhs.top && lhs.right == rhs.right && lhs.bottom == rhs.bottom;
}
