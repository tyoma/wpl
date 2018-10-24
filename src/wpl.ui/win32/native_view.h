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

#pragma once

#include <wpl/ui/visual.h>
#include <wpl/ui/win32/native_view.h>
#include <wpl/ui/win32/window.h>

#include <functional>

namespace wpl
{
	namespace ui
	{
		namespace win32
		{
			template <typename BaseT>
			class native_view : public BaseT, wpl::ui::native_view
			{
			public:
				native_view();
				~native_view();

				void init(HWND hwnd, bool own);

			protected:
				virtual void resize(unsigned cx, unsigned cy, visual::positioned_native_views &native_views);
				virtual HWND get_window() const throw();

			private:
				virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
					const window::original_handler_t &handler) = 0;

			private:
				std::shared_ptr<window> _window;
				bool _own;
			};



			template <typename BaseT>
			inline native_view<BaseT>::native_view()
				: _own(false)
			{	}

			template <typename BaseT>
			inline native_view<BaseT>::~native_view()
			{
				if (_own)
					if (HWND hwnd = get_window())
						_window.reset(), ::DestroyWindow(hwnd);
			}

			template <typename BaseT>
			inline void native_view<BaseT>::init(HWND hwnd, bool own)
			{
				using namespace std::placeholders;
				_window = window::attach(hwnd, std::bind(&native_view::on_message, this, _1, _2, _3, _4));
				_own = own;
			}

			template <typename BaseT>
			inline void native_view<BaseT>::resize(unsigned cx, unsigned cy, visual::positioned_native_views &native_views)
			{
				view_location l = { 0, 0, static_cast<int>(cx), static_cast<int>(cy) };
				native_views.push_back(visual::positioned_native_view(*this, l));
			}

			template <typename BaseT>
			inline HWND native_view<BaseT>::get_window() const throw()
			{	return _window->hwnd();	}
		}
	}
}
