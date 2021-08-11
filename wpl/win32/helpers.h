//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../concepts.h"
#include "../types.h"

#include <vector>
#include <wpl/win32/utf8.h>
#include <windows.h>

namespace wpl
{
	namespace win32
	{
		struct helpers
		{
			static void client_to_screen(rect_i &value, HWND hwnd);
			static void screen_to_client(point_i &value, HWND hwnd);

			template <typename ValueT, typename FlagT>
			static ValueT update_flag(ValueT in, bool enable, FlagT flag);

			class defer_window_pos : noncopyable
			{
			public:
				explicit defer_window_pos(size_t count);
				~defer_window_pos();

				void update_location(HWND hwnd, const rect_i &location);

			private:
				HDWP _hdwp;
			};

			class paint_sequence : public PAINTSTRUCT, noncopyable
			{
			public:
				explicit paint_sequence(HWND hwnd);
				~paint_sequence() throw();

				agge::count_t width() const throw();
				agge::count_t height() const throw();

			private:
				HWND _hwnd;
			};

			class window_handle : noncopyable
			{
			public:
				explicit window_handle(HWND hwnd) throw();
				~window_handle() throw();

				void reset(HWND hwnd) throw();
				HWND release() throw();
				operator HWND() const throw();

			private:
				HWND _hwnd;
			};

			class window_text
			{
			public:
				void get(std::string &value, HWND hwnd) const;
				void set(HWND hwnd, const std::string &value) const;
				const wchar_t *convert(const std::string &value) const;

			private:
				mutable std::vector<wchar_t> _buffer;
				mutable utf_converter _converter;
			};
		};



		template <typename ValueT, typename FlagT>
		inline ValueT helpers::update_flag(ValueT in, bool enable, FlagT flag)
		{	return enable ? (in | flag) : (in & ~flag);	}
	}
}
