#include <wpl/win32/keyboard_router.h>

#include <wpl/input.h>
#include <wpl/win32/native_view.h>
#include <wpl/win32/window.h>

using namespace std;

namespace wpl
{
	namespace win32
	{
		keyboard_router::keyboard_router(const vector<placed_view> &views, keyboard_router_host &host)
			: wpl::keyboard_router(views, host)
		{	}

		bool keyboard_router::handle_message(LRESULT &result, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
		{
			switch (message)
			{
			case WM_SETFOCUS:
				got_focus();
				return true;

			case WM_KILLFOCUS:
				lost_focus();
				return true;

			case WM_KEYDOWN:
			case WM_KEYUP:
				dispatch_key(message, wparam, lparam);
				return true;

			default:
				return false;

			case window::WM_FORWARDED:
				const auto &m = *reinterpret_cast<const MSG *>(wparam);
				auto &inhibit = *reinterpret_cast<bool *>(lparam);

				switch (m.message)
				{
				case WM_SETFOCUS:
					got_native_focus([&m] (native_view& nv) {	return m.hwnd == nv.get_window();	});
					break;
				}
				return true;
			}
		}

		void keyboard_router::dispatch_key(UINT message, WPARAM wparam, LPARAM /*lparam*/)
		{
			const auto modifiers = (::GetKeyState(VK_SHIFT) & 0x8000 ? keyboard_input::shift : 0)
				| (::GetKeyState(VK_CONTROL) & 0x8000 ? keyboard_input::control : 0);

			// In here we rely that VK_* key mappings are in accordance with keyboard_input::special_keys.
			switch (message)
			{
			case WM_KEYDOWN: key_down(static_cast<int>(wparam), modifiers); break;
			case WM_KEYUP: key_up(static_cast<int>(wparam), modifiers); break;
			}
		}
	}
}
