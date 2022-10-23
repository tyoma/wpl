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

#include "input.h"

namespace wpl
{
	class drag_helper : mouse_input
	{
	public:
		template <typename OnDragFn, typename OnCompleteFn, typename CaptureFn>
		void start(const OnDragFn &on_drag, const OnCompleteFn &on_complete, const CaptureFn &capture,
			mouse_input::mouse_buttons button_, int x, int y);
		void cancel();

	private:
		// mouse_input methods
		virtual void mouse_move(int depressed, int x, int y) override;
		virtual void mouse_up(mouse_buttons button_, int depressed, int x, int y) override;

		void reset();

	private:
		std::function<void (int dx, int dy)> _on_drag;
		std::function<void ()> _on_complete;
		std::shared_ptr<void> _capture_handle;
		mouse_buttons _button;
		int _x, _y;
	};



	template <typename OnDragFn, typename OnCompleteFn, typename CaptureFn>
	inline void drag_helper::start(const OnDragFn &on_drag, const OnCompleteFn &on_complete, const CaptureFn &capture,
		mouse_input::mouse_buttons button_, int x, int y)
	{
		_on_drag = on_drag;
		_on_complete = on_complete;
		capture(_capture_handle, *this);
		_button = button_;
		_x = x, _y = y;
	}
}
