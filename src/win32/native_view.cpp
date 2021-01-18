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

#include <wpl/win32/native_view.h>

#include <wpl/control.h>
#include <wpl/stylesheet.h>
#include <wpl/win32/font_manager.h>

using namespace std;
using namespace placeholders;

namespace wpl
{
	native_view::native_view(const string &text_style_name)
		: _text_style_name(text_style_name), _hwnd(NULL)
	{	}

	void native_view::layout(const placed_view_appender &append_view, const agge::box<int> &box)
	{
		placed_view v = {
			shared_ptr<view>(),
			shared_from_this(),
			{ 0, 0, box.w, box.h },
			1,
			false,
		};

		append_view(v);
	}

	HWND native_view::get_window() const throw()
	{	return _hwnd;	}

	HWND native_view::get_window(HWND hparent_for)
	{
		if (!_hwnd || hparent_for != ::GetParent(_hwnd))
		{
			_hwnd.reset(materialize(hparent_for));
			::SendMessage(_hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(_font.get()), 0);
			_window = win32::window::attach(_hwnd, bind(&native_view::on_message, this, _1, _2, _3, _4));
		}
		return _hwnd;
	}

	void native_view::apply_styles(const stylesheet &stylesheet_, win32::font_manager &font_manager)
	{
		auto new_font = font_manager.get_font(stylesheet_.get_font(_text_style_name.c_str())->get_key());

		if (_hwnd)
			::SendMessage(_hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(new_font.get()), 0);
		_font = new_font;
	}
}
