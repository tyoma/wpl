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

#include "text_container.h"

#include "../controls.h"
#include <memory>

namespace wpl
{
	struct button;
	struct link;
	struct view_host;

	namespace win32
	{
		class button : public text_container_impl<wpl::button>
		{
		public:
			button();

		private:
			virtual std::shared_ptr<view> get_view();

			virtual HWND materialize(HWND hparent);
			virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
				const window::original_handler_t &handler);
		};


		class link : public text_container_impl<wpl::link>
		{
		public:
			link();

		private:
			virtual std::shared_ptr<view> get_view();

			virtual HWND materialize(HWND hparent);
			virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
				const window::original_handler_t &handler);

			virtual void set_align(halign value);
		};
	}
}
