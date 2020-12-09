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

#include "input.h"

namespace wpl
{
	class drag_helper
	{
	public:
		template <typename CaptureFn, typename OnDragFn>
		void start(const OnDragFn &on_drag, const CaptureFn &capture, mouse_input::mouse_buttons button_, int x, int y);
		bool mouse_move(int x, int y);
		bool mouse_up(mouse_input::mouse_buttons button_);
		void cancel();

	private:
		std::function<void (int dx, int dy)> _on_drag;
		std::shared_ptr<void> _capture_handle;
		mouse_input::mouse_buttons _button;
		int _x, _y;
	};




	template <typename CaptureFn, typename OnDragFn>
	inline void drag_helper::start(const OnDragFn &on_drag, const CaptureFn &capture,
		mouse_input::mouse_buttons button_, int x, int y)
	{
		_on_drag = on_drag;
		capture(_capture_handle);
		_button = button_;
		_x = x, _y = y;
	}

	inline bool drag_helper::mouse_move(int x, int y)
	{	return _on_drag ? _on_drag(x - _x, y - _y), true : false;	}

	inline bool drag_helper::mouse_up(mouse_input::mouse_buttons button_)
	{
		return _button == button_ && _on_drag
			? _capture_handle.reset(), _on_drag = std::function<void(int, int)>(), true : false;
	}

	inline void drag_helper::cancel()
	{
		if (_on_drag)
			_on_drag(0, 0), _capture_handle.reset(), _on_drag = std::function<void(int, int)>();
	}
}
