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

#include <wpl/win32/font_manager.h>

#include <windows.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace win32
	{
		namespace
		{
			HFONT create_font(const font_descriptor &key)
			{
				return ::CreateFontA(-key.height, 0, 0, 0, key.weight >= bold ? FW_BOLD : FW_NORMAL, key.italic, FALSE, FALSE,
					DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
					key.family.c_str());
			}
		}


		bool font_manager::key_less::operator ()(const agge::font_descriptor &lhs, const agge::font_descriptor &rhs) const
		{
			return lhs.height < rhs.height ? true : rhs.height < lhs.height ? false :
				lhs.weight < rhs.weight ? true : rhs.weight < lhs.weight ? false :
				lhs.italic < rhs.italic ? true : rhs.italic < lhs.italic ? false :
				lhs.family < rhs.family;

		}

		font_manager::font_manager()
			: _font_cache(make_shared<font_cache>())
		{	}

		shared_ptr<void> font_manager::get_font(const font_descriptor &key)
		{
			auto i = _font_cache->find(key);

			if (_font_cache->end() != i)
				return i->second.lock();

			const auto m = _font_cache;

			i = _font_cache->insert(i, make_pair(key, shared_ptr<void>()));

			shared_ptr<void> hfont(create_font(key), [i, m] (HFONT h) {
				::DeleteObject(h);
				m->erase(i);
			});

			i->second = hfont;
			return hfont;
		}
	}
}
