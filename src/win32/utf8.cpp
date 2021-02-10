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

#include <wpl/win32/utf8.h>

#include <agge.text/utf8.h>

namespace wpl
{
	namespace win32
	{
		namespace
		{
			template <typename CharT>
			CharT *end(CharT *str)
			{
				while (*str)
					str++;
				return str;
			}
		}

		const char *utf_converter::operator ()(const wchar_t *from)
		{
			_utf8_buffer.clear();

			for (const wchar_t *i = from, *e = end(from); i != e; ++i)
			{
				if (*i <= 0x007F)
				{
					// Plain single-byte ASCII.
					_utf8_buffer.push_back(static_cast<char>(*i));
				}
				else if (*i <= 0x07FF)
				{
					// Two bytes.
					_utf8_buffer.push_back(static_cast<char>(0xC0 | (*i >> 6)));
					_utf8_buffer.push_back(static_cast<char>(0x80 | ((*i >> 0) & 0x3F)));
				}
				else
				{
					// Three bytes.
					_utf8_buffer.push_back(static_cast<char>(0xE0 | (*i >> 12)));
					_utf8_buffer.push_back(static_cast<char>(0x80 | ((*i >> 6) & 0x3F)));
					_utf8_buffer.push_back(static_cast<char>(0x80 | ((*i >> 0) & 0x3F)));
				}
			}
			return _utf8_buffer.c_str();
		}

		const wchar_t *utf_converter::operator ()(const char *from)
		{
			_utf16_buffer.clear();
			for (const char *i = from, *e = end(from); i != e; )
				_utf16_buffer.push_back(static_cast<wchar_t>(agge::utf8::next(i, e)));
			return _utf16_buffer.c_str();
		}
	}
}
