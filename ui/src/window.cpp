//	Copyright (C) 2011 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../win32/window.h"

#include <tchar.h>
#include <stdexcept>

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace
		{
			const TCHAR c_wrapper_ptr_name[] = _T("IntegricityWrapperPtr");

			class disconnector : noncopyable, public destructible
			{
				shared_ptr<window_wrapper> _target;

			public:
				disconnector(shared_ptr<window_wrapper> target);
				~disconnector() throw();
			};

			class wndproc
			{
				HWND _hwnd;
				WNDPROC _address;

			public:
				wndproc(HWND hwnd, WNDPROC address);

				LRESULT operator ()(UINT message, WPARAM wparam, LPARAM lparam) const;
			};


			disconnector::disconnector(shared_ptr<window_wrapper> target)
				: _target(target)
			{	}

			disconnector::~disconnector() throw()
			{	_target->unadvise();	}


			wndproc::wndproc(HWND hwnd, WNDPROC address)
				: _hwnd(hwnd), _address(address)
			{	}

			LRESULT wndproc::operator ()(UINT message, WPARAM wparam, LPARAM lparam) const
			{	return ::CallWindowProc(_address, _hwnd, message, wparam, lparam);	}
		}

		window_wrapper::window_wrapper(HWND hwnd)
			: _window(hwnd), _original_wndproc(reinterpret_cast<WNDPROC>(::SetWindowLongPtr(hwnd, GWLP_WNDPROC,
				reinterpret_cast<LONG_PTR>(&window_wrapper::windowproc_proxy))))
		{
			::SetProp(hwnd, c_wrapper_ptr_name, reinterpret_cast<HANDLE>(this));
		}

		LRESULT CALLBACK window_wrapper::windowproc_proxy(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
		{
			shared_ptr<window_wrapper> w(window_wrapper::extract(hwnd));

			if (message == WM_NCDESTROY)
				::RemoveProp(hwnd, c_wrapper_ptr_name);

			LRESULT result = w->_user_handler ? w->_user_handler(message, wparam, lparam,
				wndproc(hwnd, w->_original_wndproc)) : w->_original_wndproc(hwnd, message, wparam, lparam);

			if (message == WM_NCDESTROY)
				w->_this.reset();
			return result;
		}

		shared_ptr<window_wrapper> window_wrapper::extract(HWND hwnd)
		{
			window_wrapper *attached = reinterpret_cast<window_wrapper *>(::GetProp(hwnd, c_wrapper_ptr_name));

			return attached ? attached->shared_from_this() : shared_ptr<window_wrapper>();
		}

		shared_ptr<window_wrapper> window_wrapper::attach(HWND hwnd)
		{
			if (::IsWindow(hwnd))
			{
				shared_ptr<window_wrapper> w(window_wrapper::extract(hwnd));

				if (w == 0)
				{
					w.reset(new window_wrapper(hwnd));
					w->_this = w;
				}
				return w;
			}
			else
				throw invalid_argument("");
		}

		bool window_wrapper::detach() throw()
		{
			if (&windowproc_proxy != reinterpret_cast<WNDPROC>(::GetWindowLongPtr(_window, GWLP_WNDPROC)))
				return false;
			::SetWindowLongPtr(_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(_original_wndproc));
			::RemoveProp(_window, c_wrapper_ptr_name);
			_this.reset();
			return true;
		}

		shared_ptr<destructible> window_wrapper::advise(const user_handler_t &user_handler)
		{
			_user_handler = user_handler;
			return shared_ptr<destructible>(new disconnector(shared_from_this()));
		}

		void window_wrapper::unadvise() throw()
		{	_user_handler = user_handler_t();	}

		HWND window_wrapper::hwnd() const
		{	return _window;	}
	}
}
