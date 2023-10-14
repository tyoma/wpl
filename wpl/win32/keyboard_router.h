#pragma once

#include "../keyboard_router.h"

#include <windows.h>

namespace wpl
{
	namespace win32
	{
		class keyboard_router : wpl::keyboard_router
		{
		public:
			keyboard_router(const std::vector<placed_view> &views, keyboard_router_host &host);

			using wpl::keyboard_router::reload_views;
			using wpl::keyboard_router::set_focus;
			bool handle_message(LRESULT &result, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

		private:
			void dispatch_key(UINT message, WPARAM wparam, LPARAM /*lparam*/);
			bool update_modifier(UINT message, unsigned code);

		private:
			unsigned _input_modifiers;
		};
	}
}
