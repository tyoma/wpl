//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../controls.h"
#include "native_view.h"

#include <memory>

namespace wpl
{
	namespace win32
	{
		template <typename BaseT>
		class text_container_impl : public BaseT, public native_view
		{
		protected:
			typedef text_container_impl text_container;

		protected:
			text_container_impl(const std::string &text_style_name);

			virtual void set_align(wpl::text_container::halign value);
			virtual void set_text(const std::wstring &text);

		protected:
			std::wstring _text;
			wpl::text_container::halign _halign;
		};



		template <typename BaseT>
		inline text_container_impl<BaseT>::text_container_impl(const std::string &text_style_name)
			: native_view(text_style_name)
		{	}

		template <typename BaseT>
		inline void text_container_impl<BaseT>::set_align(wpl::text_container::halign value)
		{	_halign = value;	}

		template <typename BaseT>
		inline void text_container_impl<BaseT>::set_text(const std::wstring &text)
		{
			_text = text;
			::SetWindowTextW(get_window(), _text.c_str());
		}
	}
}
