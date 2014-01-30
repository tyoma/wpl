//	Copyright (c) 2011-2014 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../widget.h"

typedef struct HWND__ *HWND;

namespace wpl
{
	namespace ui
	{
		class native_root : noncopyable
		{
		public:
			native_root(HWND handle);

			const HWND handle;
		};

		class native_view : public view
		{
			typedef std::vector< std::shared_ptr<const wpl::ui::transform> > transform_chain_;

			HWND _view_hwnd;
			transform_chain_ _transforms;

		public:
			typedef transform_chain_ transform_chain;

		public:
			native_view(std::shared_ptr<wpl::ui::widget> widget, HWND view_hwnd, const native_root &r);
			~native_view();

			virtual void move(int left, int top, int width, int height);
		};



		inline native_root::native_root(HWND handle_)
			: handle(handle_)
		{	}
	}
}
