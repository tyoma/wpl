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

#include "../signal.h"
#include "window.h"

#include <agge/types.h>
#include <vector>

namespace wpl
{
	struct placed_view;
	struct stylesheet;

	typedef std::function<void (const placed_view &pv)> placed_view_appender;

	namespace win32
	{
		class font_manager;
	}

	class native_view : public std::enable_shared_from_this<native_view>
	{
	public:
		native_view(const std::string &text_style_name);
		~native_view();

		void layout(const placed_view_appender &append_view, const agge::box<int> &box);
		HWND get_window() const throw();
		HWND get_window(HWND hparent_for);
		void apply_styles(const stylesheet &stylesheet_, win32::font_manager &font_manager);

	public:
		signal<void (bool &handled, LRESULT &result, UINT message, WPARAM wparam, LPARAM lparam)> on_message_hook;

	private:
		virtual HWND materialize(HWND hparent) = 0;
		virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
			const win32::window::original_handler_t &handler) = 0;

	private:
		std::string _text_style_name;
		std::shared_ptr<void> _font;
		std::shared_ptr<win32::window> _window;
	};
}
