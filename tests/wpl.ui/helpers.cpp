#include "helpers.h"

#include <agge/color.h>
#include <wpl/win32/window.h>

#include <algorithm>
#include <commctrl.h>
#include <olectl.h>
#include <tchar.h>
#include <ut/assert.h>
#include <windows.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
#ifdef UNICODE
			wstring t2w(const wstring &text)
			{	return text;	}

			wstring w2t(const wstring &text)
			{	return text;	}
#else 
			wstring t2w(const string &text)
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

			LRESULT reflection_wndproc(UINT message, WPARAM wparam, LPARAM lparam,
				const wpl::win32::window::original_handler_t &previous)
			{
				switch (message)
				{
				case WM_NOTIFY:
					return ::SendMessage(reinterpret_cast<NMHDR *>(lparam)->hwndFrom, OCM_NOTIFY, wparam, lparam);

				case WM_COMMAND:
					return ::SendMessage(reinterpret_cast<HWND>(lparam), OCM_COMMAND, wparam, lparam);

				default:
					return previous(message, wparam, lparam);
				}
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

			::GetWindowText(hwnd, &text[0], static_cast<int>(text.size()));
			return t2w(tstring(&text[0]));
		}

		bool has_style(HWND hwnd, int style)
		{	return style == (style & ::GetWindowLong(hwnd, GWL_STYLE));	}

		bool has_no_style(HWND hwnd, int style)
		{	return !(style & ::GetWindowLong(hwnd, GWL_STYLE));	}

		void emulate_click(HWND hwnd, int x, int y, mouse_input::mouse_buttons button,
			int /*mouse_buttons | modifier_keys*/ /*depressed*/)
		{
			int message_down, message_up;
			MSG msg;

			switch (button)
			{
			case mouse_input::left: message_down = WM_LBUTTONDOWN, message_up = WM_LBUTTONUP; break;
			default: throw 0;
			}
			::PostMessage(hwnd, message_up, 0, pack_coordinates(x, y));
			::SendMessage(hwnd, message_down, 0, pack_coordinates(x, y));
			while (::PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
				::DispatchMessage(&msg);
		}


		WindowManager::~WindowManager()
		{
			Cleanup();
		}

		HWND WindowManager::create_window()
		{	return create_window(L"static", 0, WS_POPUP, 0);	}

		HWND WindowManager::create_visible_window()
		{	return create_window(L"static", 0, WS_VISIBLE, 0);	}

		HWND WindowManager::create_window(const wstring &class_name)
		{	return create_window(class_name, 0, WS_POPUP, 0);	}

		HWND WindowManager::create_window(const wstring &class_name, HWND parent, unsigned int style, unsigned int exstyle)
		{
			HWND hwnd = ::CreateWindowEx(exstyle, w2t(class_name).c_str(), NULL, style, 0, 0, 200, 150, parent, NULL, NULL, NULL);

			_windows.push_back(hwnd);
			return hwnd;
		}

		void WindowManager::enable_reflection(HWND hwnd)
		{	_connections.push_back(wpl::win32::window::attach(hwnd, &reflection_wndproc));	}

		void WindowManager::destroy_window(HWND hwnd)
		{
			_windows.erase(remove(_windows.begin(), _windows.end(), hwnd), _windows.end());
			::DestroyWindow(hwnd);
		}

		void WindowManager::Cleanup()
		{
			for (vector<HWND>::const_iterator i = _windows.begin(); i != _windows.end(); ++i)
				if (::IsWindow(*i))
					::DestroyWindow(*i);
			_connections.clear();
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

		vector<HWND> window_tracker::find_created(const wstring &class_name)
		{
			vector<HWND> result;

			for (vector<HWND>::const_iterator i = created.begin(); i != created.end(); ++i)
			{
				TCHAR tmp[100] = { 0 };

				::GetClassName(*i, tmp, _countof(tmp));
				if (0 == _tcsicmp(class_name.c_str(), tmp))
					result.push_back(*i);
			}
			return result;
		}

		gcontext::pixel_type make_pixel(const agge::color& color)
		{
			gcontext::pixel_type p;

			p.components[order_bgra::R] = color.r;
			p.components[order_bgra::G] = color.g;
			p.components[order_bgra::B] = color.b;
			p.components[order_bgra::A] = color.a;
			return p;
		}
	}
}

namespace agge
{
	bool operator ==(const wpl::gcontext::pixel_type &lhs, const wpl::gcontext::pixel_type &rhs)
	{	return !memcmp(&lhs, &rhs, sizeof(wpl::gcontext::pixel_type));	}
}

bool operator ==(const RECT &lhs, const RECT &rhs)
{
	return lhs.left == rhs.left && lhs.top == rhs.top && lhs.right == rhs.right && lhs.bottom == rhs.bottom;
}

