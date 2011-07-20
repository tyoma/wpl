#pragma once

#include "../concepts.h"

#include <windows.h>
#include <memory>
#include <functional>

namespace std
{
	using tr1::enable_shared_from_this;
	using tr1::function;
	using tr1::shared_ptr;
}

namespace wpl
{
	namespace ui
	{
		class window_wrapper : public std::enable_shared_from_this<window_wrapper>
		{
		public:
			typedef std::function<LRESULT (UINT, WPARAM, LPARAM)> original_handler_t;
			typedef std::function<LRESULT (UINT, WPARAM, LPARAM, const original_handler_t &)> user_handler_t;

		public:
			static std::shared_ptr<window_wrapper> attach(HWND hwnd);
			bool detach() throw();

			std::shared_ptr<destructible> advise(const user_handler_t &user_handler);
			void unadvise() throw();

			HWND hwnd() const;

		private:
			HWND _window;
			WNDPROC _original_handler;
			std::shared_ptr<user_handler_t> _user_handler;
			std::shared_ptr<window_wrapper> _this;

			window_wrapper(HWND hwnd);

			static LRESULT CALLBACK windowproc_proxy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			static std::shared_ptr<window_wrapper> extract(HWND hwnd);
		};
	}
}