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

#include <wpl/win32/native_view.h>

#include <wpl/stylesheet.h>
#include <wpl/win32/font_manager.h>

using namespace std;
using namespace placeholders;

namespace wpl
{
	native_view::native_view(const string &text_style_name)
		: _text_style_name(text_style_name), _own(false)
	{	}

	native_view::~native_view()
	{
		if (_own)
			if (HWND hwnd = get_window())
				_window.reset(), ::DestroyWindow(hwnd);
	}

	void native_view::resize(unsigned cx, unsigned cy, visual::positioned_native_views &native_views)
	{
		view_location l = { 0, 0, static_cast<int>(cx), static_cast<int>(cy) };
		native_views.push_back(visual::positioned_native_view(*this, l));
	}

	void native_view::got_focus()
	{	::SetFocus(get_window());	}

	HWND native_view::get_window() const throw()
	{	return _window ? _window->hwnd() : 0;	}

	HWND native_view::get_window(HWND hparent_for)
	{
		if (_window && hparent_for == ::GetParent(_window->hwnd()))
			return _window->hwnd();

		HWND hwnd = materialize(hparent_for);

		if (_window)
			::DestroyWindow(_window->hwnd());
		::SendMessage(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(_font.get()), 0);
		_window = win32::window::attach(hwnd, bind(&native_view::on_message, this, _1, _2, _3, _4));
		_own = true;
		return hwnd;
	}

	void native_view::apply_styles(const stylesheet &stylesheet_, win32::font_manager &font_manager)
	{
		auto new_font = font_manager.get_font(stylesheet_.get_font(_text_style_name.c_str())->get_key());

		if (_window)
			::SendMessage(_window->hwnd(), WM_SETFONT, reinterpret_cast<WPARAM>(new_font.get()), 0);
		_font = new_font;
	}
}
