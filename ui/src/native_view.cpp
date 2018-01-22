//	Copyright (c) 2011-2018 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../win32/native_view.h"

#include "../geometry.h"
#include <windows.h>

namespace wpl
{
	namespace ui
	{
		native_view::native_view(std::shared_ptr<wpl::ui::widget> widget, HWND view_hwnd, const native_root &r)
			: view(widget), _view_hwnd(view_hwnd)
		{
			::SetParent(_view_hwnd, r.handle);
			::SetWindowLong(_view_hwnd, GWL_STYLE, (::GetWindowLong(_view_hwnd, GWL_STYLE) & ~WS_CHILD) | (r.handle ? WS_CHILD : 0));
		}

		native_view::~native_view()
		{	}


		void native_view::move(int left, int top, int width, int height)
		{
			for (transform_chain::const_iterator i = _transforms.begin(); i != _transforms.end(); ++i)
				(*i)->unmap(left, top);
			::MoveWindow(_view_hwnd, left, top, width, height, TRUE);
		}

		//void native_view::set_parent(const transform_chain &tc, HWND parent)
		//{
		//	_transforms = tc;
		//	::SetParent(_view_hwnd, parent);
		//	::SetWindowLong(_view_hwnd, GWL_STYLE, (::GetWindowLong(_view_hwnd, GWL_STYLE) & ~WS_CHILD) | (parent ? WS_CHILD : 0));
		//}
	}
}
