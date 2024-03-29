#include "helpers-win32.h"

#include <tests/common/Mockups.h>

#include <wpl/control.h>
#include <wpl/win32/font_loader.h>
#include <wpl/win32/native_view.h>
#include <wpl/win32/utf8.h>
#include <wpl/win32/window.h>

#include <algorithm>
#include <commctrl.h>
#include <olectl.h>
#include <tchar.h>
#include <windows.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
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

		HWND get_window_and_resize(shared_ptr<control> control_, HWND hparent, int cx, int cy)
		{
			vector<placed_view> v;
			agge::box<int> b = { cx, cy };

			control_->layout(make_appender(v), b);
			assert_equal(1u, v.size());
			assert_not_null(v[0].native);
			assert_equal(create_rect(0, 0, cx, cy), v[0].location);
			assert_is_false(v[0].overlay);

			HWND hwnd = v[0].native->get_window(hparent);

			::MoveWindow(hwnd, 0, 0, cx, cy, TRUE);
			return hwnd;
		}

		bool provides_tabstoppable_native_view(std::shared_ptr<control> control_)
		{
			vector<placed_view> v;
			agge::box<int> b = { 1, 1 };

			control_->layout(make_appender(v), b);
			assert_equal(1u, v.size());
			assert_is_true(1 == v[0].tab_order || 0 == v[0].tab_order);
			return !!v[0].tab_order;
		}

		RECT get_window_rect(HWND hwnd)
		{
			RECT rc = { };
			HWND hparent = ::GetParent(hwnd);

			::GetWindowRect(hwnd, &rc);
			if ((WS_CHILD & ::GetWindowLong(hwnd, GWL_STYLE)) && hparent)
				::MapWindowPoints(NULL, hparent, reinterpret_cast<POINT *>(&rc), 2);
			return rc;
		}

		agge::box<int> get_client_size(HWND hwnd)
		{
			RECT rc;

			::GetClientRect(hwnd, &rc);
			return agge::create_box<int>(rc.right, rc.bottom);
		}

		rect_i get_update_rect(HWND hwnd)
		{
			RECT rc;

			assert_is_true(!!::GetUpdateRect(hwnd, &rc, FALSE));
			return create_rect<int>(rc.left, rc.top, rc.right, rc.bottom);
		}

		RECT rect(int left, int top, int width, int height)
		{
			RECT rc = { left, top, left + width, top + height };

			return rc;
		}

		wstring get_window_text(HWND hwnd)
		{
			vector<wchar_t> text(::GetWindowTextLength(hwnd) + 1);

			::GetWindowTextW(hwnd, &text[0], static_cast<int>(text.size()));
			return text.data();
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

		shared_ptr<gcontext::text_engine_type> create_text_engine()
		{
			struct text_engine_composite
			{
				text_engine_composite()
					: text_engine(loader)
				{	}

				win32::font_loader loader;
				gcontext::text_engine_type text_engine;
			};

			shared_ptr<text_engine_composite> tec(new text_engine_composite);

			return shared_ptr<gcontext::text_engine_type>(tec, &tec->text_engine);
		}


		window_manager::~window_manager()
		{
			Cleanup();
		}

		HWND window_manager::create_window()
		{	return create_window(L"static", 0, WS_POPUP, 0);	}

		HWND window_manager::create_visible_window()
		{	return create_window(L"static", 0, WS_VISIBLE, 0);	}

		HWND window_manager::create_window(const wstring &class_name)
		{	return create_window(class_name, 0, WS_POPUP, 0);	}

		HWND window_manager::create_window(const wstring &class_name, HWND parent, unsigned int style, unsigned int exstyle)
		{
			HWND hwnd = ::CreateWindowExW(exstyle, class_name.c_str(), NULL, style, 0, 0, 200, 150, parent, NULL, NULL, NULL);

			_windows.push_back(hwnd);
			return hwnd;
		}

		void window_manager::enable_reflection(HWND hwnd)
		{	_connections.push_back(wpl::win32::window::attach(hwnd, &reflection_wndproc));	}

		void window_manager::destroy_window(HWND hwnd)
		{
			_windows.erase(remove(_windows.begin(), _windows.end(), hwnd), _windows.end());
			::DestroyWindow(hwnd);
		}

		void window_manager::Cleanup()
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
			} data = { _allow, _prohibit };

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

		unsigned int pack_screen_coordinates(HWND hwnd, int x, int y)
		{
			POINT pt = {	x, y	};

			::ClientToScreen(hwnd, &pt);
			return pack_coordinates(pt.x, pt.y);
		}

		unsigned int pack_wheel(int delta, int modifiers)
		{	return (unsigned short)modifiers | ((unsigned int )(unsigned short)(delta * WHEEL_DELTA) << 16);	}
	}
}

bool operator ==(const RECT &lhs, const RECT &rhs)
{	return lhs.left == rhs.left && lhs.top == rhs.top && lhs.right == rhs.right && lhs.bottom == rhs.bottom;	}

bool operator ==(const agge::rect_i &lhs, const RECT &rhs)
{	return lhs.x1 == rhs.left && lhs.y1 == rhs.top && lhs.x2 == rhs.right && lhs.y2 == rhs.bottom;	}
