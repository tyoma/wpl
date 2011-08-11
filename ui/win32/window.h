//	Copyright (C) 2011 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#pragma once

#include "../../base/concepts.h"

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
			typedef std::function<LRESULT (UINT, WPARAM, LPARAM)> _original_handler_t;
			typedef std::function<LRESULT (UINT, WPARAM, LPARAM, const _original_handler_t &)> _user_handler_t;

			HWND _window;
			WNDPROC _original_wndproc;
			_user_handler_t _user_handler;
			std::shared_ptr<window_wrapper> _this;

			window_wrapper(HWND hwnd);

			static LRESULT CALLBACK windowproc_proxy(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
			static std::shared_ptr<window_wrapper> extract(HWND hwnd);

		public:
			typedef _original_handler_t original_handler_t;
			typedef _user_handler_t user_handler_t;

		public:
			static std::shared_ptr<window_wrapper> attach(HWND hwnd);
			bool detach() throw();

			std::shared_ptr<destructible> advise(const user_handler_t &user_handler);
			void unadvise() throw();

			HWND hwnd() const throw();
		};
	}
}
