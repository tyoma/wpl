//	Copyright (c) 2011-2018 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../../mt/thread.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <windows.h>

namespace wpl
{
	namespace ui
	{
		class window
		{
		public:
			typedef window original_handler_t;
			typedef std::function<LRESULT (UINT message, WPARAM wparam, LPARAM lparam, const original_handler_t &handler)> user_handler_t;

		public:
			static std::shared_ptr<window> attach(HWND hwnd, const user_handler_t &user_handler);

			HWND hwnd() const throw();

			LRESULT operator ()(UINT message, WPARAM wparam, LPARAM lparam) const;

		private:
			struct hwnd_hash { size_t operator ()(HWND hwnd) const; };
			typedef std::unordered_map<HWND, window *, hwnd_hash> windows_map;

		private:
			window(HWND hwnd, const user_handler_t &user_handler);

			const window &operator =(const window &rhs);

			static LRESULT CALLBACK windowproc_proxy(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
			static void map(HWND hwnd, window *w);
			static window *get_window(HWND hwnd) throw();
			static bool unmap(HWND hwnd, bool force) throw();
			static void detach(window *w);

		private:
			static mt::tls<windows_map> _windows;
			HWND _hwnd;
			LONG_PTR _wndproc;
			user_handler_t _user_handler;
		};
	}
}
