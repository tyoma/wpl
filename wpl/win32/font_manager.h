//	Copyright (c) 2011-2022 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <agge.text/font.h>
#include <map>
#include <memory>

namespace wpl
{
	namespace win32
	{
		class font_manager : noncopyable
		{
		public:
			font_manager();

			std::shared_ptr<void> /*HFONT*/ get_font(const agge::font_descriptor &font_key);

		private:
			struct key_less
			{
				bool operator ()(const agge::font_descriptor &lhs, const agge::font_descriptor &rhs) const;
			};

			typedef std::map<agge::font_descriptor, std::weak_ptr<void> /*HFONT*/, key_less> font_cache;

		private:
			std::shared_ptr<font_cache> _font_cache;
		};
	}
}
