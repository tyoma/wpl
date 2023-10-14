#include <wpl/win32/keyboard_router.h>

#include <wpl/input.h>

using namespace std;

namespace wpl
{
	namespace win32
	{
		keyboard_router::keyboard_router(const vector<placed_view> &views, keyboard_router_host &host)
			: wpl::keyboard_router(views, host), _input_modifiers(0)
		{	}

		bool keyboard_router::handle_message(LRESULT &result, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
		{
			switch (message)
			{
			case WM_KILLFOCUS:
				set_focus(nullptr);
				return true;

			case WM_KEYDOWN:
			case WM_KEYUP:
				dispatch_key(message, wparam, lparam);
				return true;

			default:
				return false;
			}
		}

		void keyboard_router::dispatch_key(UINT message, WPARAM wparam, LPARAM /*lparam*/)
		{
			if (update_modifier(message, static_cast<int>(wparam)))
				return;

			// In here we rely that VK_* key mappings are in accordance with keyboard_input::special_keys.
			switch (message)
			{
			case WM_KEYDOWN: key_down(static_cast<int>(wparam), _input_modifiers); break;
			case WM_KEYUP: key_up(static_cast<int>(wparam), _input_modifiers); break;
			}
		}

		bool keyboard_router::update_modifier(UINT message, unsigned code)
		{
			auto modifier_flag = 0u;

			switch (code)
			{
			case VK_SHIFT:	modifier_flag = keyboard_input::shift;	break;
			case VK_CONTROL:	modifier_flag = keyboard_input::control;	break;
			default:	return false;
			}
			if (WM_KEYDOWN == message)
				_input_modifiers |= modifier_flag;
			else if (WM_KEYUP == message)
				_input_modifiers &= ~modifier_flag;
			return true;
		}
	}
}
