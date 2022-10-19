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

#include <wpl/win32/helpers.h>

#include <wpl/helpers.h>

using namespace std;

namespace wpl
{
	namespace win32
	{
		void helpers::client_to_screen(rect_i &value, HWND hwnd)
		{
			POINT pt[] = {	{	value.x1, value.y1	}, {	value.x2, value.y2	},	};

			::MapWindowPoints(hwnd, NULL, pt, 2);
			value.x1 = pt[0].x, value.y1 = pt[0].y, value.x2 = pt[1].x, value.y2 = pt[1].y;
		}

		void helpers::screen_to_client(point_i &value, HWND hwnd)
		{
			POINT pt = { value.x, value.y };

			::ScreenToClient(hwnd, &pt);
			value.x = pt.x, value.y = pt.y;
		}


		helpers::defer_window_pos::defer_window_pos(size_t count)
			: _hdwp(::BeginDeferWindowPos(static_cast<int>(count)))
		{	}

		helpers::defer_window_pos::~defer_window_pos()
		{	::EndDeferWindowPos(_hdwp);	}

		void helpers::defer_window_pos::update_location(HWND hwnd, const rect_i &location)
		{
			_hdwp = ::DeferWindowPos(_hdwp, hwnd, NULL, location.x1, location.y1,
				wpl::width(location), wpl::height(location), SWP_NOZORDER);
		}


		helpers::paint_sequence::paint_sequence(HWND hwnd)
			: _hwnd(hwnd)
		{	::BeginPaint(_hwnd, this);	}

		helpers::paint_sequence::~paint_sequence() throw()
		{	::EndPaint(_hwnd, this);	}

		agge::count_t helpers::paint_sequence::width() const throw()
		{	return rcPaint.right - rcPaint.left;	}

		agge::count_t helpers::paint_sequence::height() const throw()
		{	return rcPaint.bottom - rcPaint.top;	}


		helpers::window_handle::window_handle(HWND hwnd) throw()
			: _hwnd(hwnd)
		{	}

		helpers::window_handle::~window_handle() throw()
		{
			if (_hwnd)
				::DestroyWindow(_hwnd);
		}

		void helpers::window_handle::reset(HWND hwnd) throw()
		{
			if (_hwnd)
				::DestroyWindow(_hwnd);
			_hwnd = hwnd;
		}

		HWND helpers::window_handle::release() throw()
		{	return move(_hwnd);	}

		helpers::window_handle::operator HWND() const throw()
		{	return _hwnd;	}


		void helpers::window_text::get(string &value, HWND hwnd) const
		{
			_buffer.resize(::GetWindowTextLengthW(hwnd) + 1);
			_buffer.back() = L'\0';
			::GetWindowTextW(hwnd, _buffer.data(), static_cast<int>(_buffer.size()));
			value = _converter(_buffer.data());
		}

		void helpers::window_text::set(HWND hwnd, const string &value) const
		{	::SetWindowTextW(hwnd, _converter(value.c_str()));	}

		const wchar_t *helpers::window_text::convert(const string &value) const
		{	return _converter(value.c_str());	}
	}
}
